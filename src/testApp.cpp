#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){    
    // Allocate memory for images
    colorImgHSV.allocate(camWidth,camHeight);  
    
    hueImg.allocate(camWidth,camHeight);        //Hue map
    satImg.allocate(camWidth,camHeight);        //saturation map
    briImg.allocate(camWidth,camHeight);        //brightness map, not gonna be used but necessary 
    
    hueImgBg.allocate(camWidth,camHeight);        //Hue map
    satImgBg.allocate(camWidth,camHeight);        //saturation map
    briImgBg.allocate(camWidth,camHeight);        //brightness map, not gonna be used but 
    
    hueImgPeaks.allocate(camWidth,camHeight);
    satImgPeaks.allocate(camWidth,camHeight);
    briImgPeaks.allocate(camWidth,camHeight);
    
    background.allocate(camWidth,camHeight);
    bLearnBackground = true;
    
    thresholdHue = 80;
    thresholdSat = 80;
    thresholdBri = 200;
    
    trackedImage.allocate(camWidth, camHeight);         //our postRange image basically
    
    satPixelsRaw = new unsigned char [camWidth * camHeight];
    huePixelsRaw = new unsigned char [camWidth * camHeight];
    briPixelsRaw = new unsigned char [camWidth * camHeight];
    
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
    
    KF.statePre.at<float>(0) = ballPos.x;
    KF.statePre.at<float>(1) = ballPos.y;
    KF.statePre.at<float>(2) = 0;
    KF.statePre.at<float>(3) = 0;
    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(1e-4));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(KF.errorCovPost, Scalar::all(.1));
    
    // Optical flow
    flow.setup(camWidth,camHeight);
}

//--------------------------------------------------------------
void testApp::update(){
    vidGrabber.grabFrame();
    
    colorImgHSV.setFromPixels(vidGrabber.getPixels(), camWidth, camHeight);    
    colorImgHSV.convertRgbToHsv();     
    
    if (bLearnBackground) {
        background = colorImgHSV;
        bLearnBackground = false;
    }
    
    colorImgHSV.convertToGrayscalePlanarImages(hueImg, satImg, briImg);    
    background.convertToGrayscalePlanarImages(hueImgBg, satImgBg, briImgBg);  
    
    hueImg.flagImageChanged(); 
    satImg.flagImageChanged();
    briImg.flagImageChanged();
    
    hueImgBg.flagImageChanged(); 
    satImgBg.flagImageChanged();
    briImgBg.flagImageChanged();
    
    hueImgPeaks.absDiff(hueImgBg, hueImg);
    hueImgPeaks.threshold(thresholdHue);
    
    satImgPeaks.absDiff(satImgBg, satImg);
    satImgPeaks.threshold(thresholdSat);

    briImgPeaks.absDiff(briImgBg, briImg);
    briImgPeaks.threshold(thresholdBri);

//    // Load HSV pixels into arrays
//    unsigned char* huePixels = hueImg.getPixels();                         
//    unsigned char* satPixels = satImg.getPixels();                         
//    unsigned char* briPixels = briImg.getPixels();
//    
//    // For iteration
//    int nPixels = camWidth * camHeight;                                     
//    
//    hueImgPeaks.setFromPixels(huePixelsRaw, camWidth, camHeight);
//    satImgPeaks.setFromPixels(satPixelsRaw, camWidth, camHeight);
//    briImgPeaks.setFromPixels(briPixelsRaw, camWidth, camHeight);
    
//    trackedImage.setROI(0, ROI_Y_OFFSET, camWidth, ROI_Y_HEIGHT);      // clamp
    
//    ballFinder.findContours(trackedImage, 50, 100, 5, false, false);                
//    
//    // Filter out random blobs and get ball position
//    for (int i = 0; i < ballFinder.blobs.size(); i++) {
//        if (ballFinder.blobs[i].boundingRect.width < 15 && ballFinder.blobs[i].boundingRect.height < 15) {
//            ballPos = ballFinder.blobs[i].centroid;
//        }
//    }
    
    /*************
     Kalman filter
    *************/
    
    // First predict, to update the internal statePre variable
    Mat prediction = KF.predict();
    ofPoint predictPt(prediction.at<float>(0),prediction.at<float>(1));
    
    // Get mouse point
    measurement(0) = ballPos.x;
    measurement(1) = ballPos.y;
        
    // The "correct" phase that is going to use the predicted value and our measurement
    Mat estimated = KF.correct(measurement);
    kalmanBallPos = ofVec2f(estimated.at<float>(0),estimated.at<float>(1));
    
    /*************
          OSC
     *************/
    
    // Send ballPosition to server
    ofxOscMessage m;
    m.setAddress( "/ball_position" );
    m.addFloatArg( ballPos.x );
    m.addFloatArg( ballPos.y );
    m.addIntArg( ofGetSystemTimeMicros() );
    sender.sendMessage( m );
    
    /**************
      Optical flow
     **************/
    
    flow.update(colorImgHSV);
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
    
    // Draw hue, sat, bri RAW images
	hueImg.draw(340, 0);
    satImg.draw(340, 300);
    briImg.draw(340, 600);
		
    // Draw hue, sat, bri PEAK images
	hueImgPeaks.draw(680, 0);
    satImgPeaks.draw(680, 300);
    briImgPeaks.draw(680, 600);
    
    // Draw ball position
    ofSetColor(255, 255, 255); 
    char label[255];
    sprintf(label, "x : %f\ny : %f", ballPos.x, ballPos.y);  
    ofDrawBitmapString(label, 20, 250); 
    
    ofRect(0, 300, camWidth, camHeight);
    glTranslated(0, 300, 0);
    ofSetColor(255, 0, 255);
    ofCircle(ballPos, 5);
    
    flow.draw();
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
	switch (key){
		case ' ':
			bLearnBackground = true;
			break;
	}
}