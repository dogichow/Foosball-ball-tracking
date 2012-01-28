#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){    
    // Allocate memory for images
    colorImg.allocate(camWidth,camHeight);
    colorImgHSV.allocate(camWidth,camHeight);  
    
    hueImg.allocate(camWidth,camHeight);        //Hue map
    satImg.allocate(camWidth,camHeight);        //saturation map
    briImg.allocate(camWidth,camHeight);        //brightness map, not gonna be used but 
    
    redImg.allocate(camWidth,camHeight);        //Hue map
    greenImg.allocate(camWidth,camHeight);        //saturation map
    blueImg.allocate(camWidth,camHeight);        //brightness map, not gonna be used but 
    
    trackedImage.allocate(camWidth, camHeight);         //our postRange image basically
    colorTrackedPixels = new unsigned char [camWidth * camHeight];     //rangeImage
    trackedTextureRed.allocate(camWidth, camHeight, GL_LUMINANCE);        //final output
    vidGrabber.setVerbose(true);                    //just some text for debugging
    vidGrabber.initGrabber(camWidth,camHeight);     //start the show!
    
    // Open communication protocol
    sender.setup( HOST, PORT );
    
    // Ball position init
    ballPos = ofVec2f(0,0);
    kalmanBallPos = ofVec2f(0,0);
    
    // Kalman Filter setup
    KF = KalmanFilter(4, 2);
    state = Mat_<float>(4, 1);
    processNoise =  Mat(4, 1, CV_32F);
    measurement = Mat_<float>(2, 1);
    
    measurement.setTo(Scalar(0));
    
    cameraThreshold = 29;
    
    KF.statePre.at<float>(0) = ballPos.x;
    KF.statePre.at<float>(1) = ballPos.y;
    KF.statePre.at<float>(2) = 0;
    KF.statePre.at<float>(3) = 0;
    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(1e-4));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(KF.errorCovPost, Scalar::all(.1));
}

//--------------------------------------------------------------
void testApp::update(){
    vidGrabber.grabFrame();                                                 //get a frame from the camera
    colorImg.setFromPixels(vidGrabber.getPixels(), camWidth, camHeight);    //remember that colorImg? put the camera image into it
//    colorImgHSV.convertRgbToHsv();                                          //now we convert the colorImg inside colorImgHSV into HSV
    
    colorImg.convertToGrayscalePlanarImages(redImg, greenImg, blueImg);     
//    colorImgHSV.convertToGrayscalePlanarImages(hueImg, satImg, briImg);     
    
//    hueImg.flagImageChanged(); 
//    satImg.flagImageChanged();
//    briImg.flagImageChanged();
    
    redImg.flagImageChanged(); 
    greenImg.flagImageChanged();
    blueImg.flagImageChanged();

//    unsigned char * huePixels = hueImg.getPixels();                         
//    unsigned char * satPixels = satImg.getPixels();                       
//    unsigned char * briPixels = briImg.getPixels();
    
    unsigned char * redPixels = redImg.getPixels();                         
    unsigned char * greenPixels = greenImg.getPixels();                         
    unsigned char * bluePixels = blueImg.getPixels();
    
    int nPixels = camWidth * camHeight;        
    
    // Color tracking filter
    for (int i = 0; i < nPixels; i++){                                           
        ofVec3f colorVec = ofVec3f(redPixels[i], greenPixels[i], bluePixels[i]);
        
        float distance = colorVec.distance(trackedColor);
        
        if (distance < cameraThreshold)
            colorTrackedPixels[i] = 255;
        else
            colorTrackedPixels[i] = 0;
    }
    
    trackedImage.setFromPixels(colorTrackedPixels, camWidth, camHeight);            
    trackedImage.setROI(0, ROI_Y_OFFSET, camWidth, ROI_Y_HEIGHT);            
    
    ballFinder.findContours(trackedImage, 10, 100, 5, false, false);                  //lets find one (1) blob in the grayscale openCv image reds
    
    // Filter out random blobs and get ball position
    for (int i = 0; i < ballFinder.blobs.size(); i++) {
        if (ballFinder.blobs[i].boundingRect.width < 15 && ballFinder.blobs[i].boundingRect.height < 15) {
            ballPos = ballFinder.blobs[i].centroid;
        }
    }
    
    // Kalman filter calculations
    // First predict, to update the internal statePre variable
    Mat prediction = KF.predict();
    ofPoint predictPt(prediction.at<float>(0),prediction.at<float>(1));
    
    // Get mouse point
    measurement(0) = ballPos.x;
    measurement(1) = ballPos.y;
        
    // The "correct" phase that is going to use the predicted value and our measurement
    Mat estimated = KF.correct(measurement);
    kalmanBallPos = ofVec2f(estimated.at<float>(0),estimated.at<float>(1));
    
    // Send ballPosition to server
    ofxOscMessage m;
    m.setAddress( "/ball_position" );
    m.addFloatArg( ballPos.x );
    m.addFloatArg( ballPos.y );
    m.addIntArg( ofGetSystemTimeMicros() );
    sender.sendMessage( m );
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(100,100,100);
    ofSetColor(255, 255, 255);
    
    // Draw raw video
    vidGrabber.draw(0,0);
    
    // Draw contour tracker
    glPushMatrix();
    glTranslatef(0,ROI_Y_OFFSET, 0);
    ballFinder.draw();
     
    // Draw ball position circle
    ofSetColor(255, 255, 0);
    ofCircle(ballPos, 5);
    
    ofSetColor(255, 0, 0);
    ofCircle(kalmanBallPos, 3);
    glPopMatrix();
    
    ofColor c;
    c.set(trackedColor.x, trackedColor.y, trackedColor.z);
    ofSetColor(c);
    
    // Draw racked color reference
    ofRect(700, 50, 0, 100, 100);
    
    // Draw HSV image
	colorImgHSV.draw(340, 0);
		
    // Draw ball position
    ofSetColor(255, 255, 255); 
    char label[255];
    sprintf(label, "x : %f\ny : %f", ballPos.x, ballPos.y);  
    ofDrawBitmapString(label, 20, 250); 
    
    ofRect(0, 300, camWidth, camHeight);
    glTranslated(0, 300, 0);
    ofSetColor(255, 0, 255);
    ofCircle(ballPos, 5);
    
    trackedImage.draw(680, 0);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
    unsigned char * redPixels = redImg.getPixels(); 
    unsigned char * greenPixels = greenImg.getPixels();  
    unsigned char * bluePixels = blueImg.getPixels();  
    
    x = MIN(x,redImg.width-1);     
    y = MIN(y,redImg.height-1);
    
    if(button == 0) {
        trackedColor.x = redPixels[x+(y*hueImg.width)];
        trackedColor.y = greenPixels[x+(y*satImg.width)];  
        trackedColor.z = bluePixels[x+(y*briImg.width)];
    }
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
    if (key == 'a')
        cameraThreshold++;
    else if (key == 's')
        cameraThreshold--;
    
    cout << cameraThreshold << endl;
}