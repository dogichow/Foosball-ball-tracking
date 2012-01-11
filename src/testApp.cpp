#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){    
    
    // Allocate memory for images
    
    colorImgHSV.allocate(camWidth,camHeight);   //our HSB image that will house the color image and deal out the Hue, Saturation and brightness
    hueImg.allocate(camWidth,camHeight);        //Hue map
    satImg.allocate(camWidth,camHeight);        //saturation map
    briImg.allocate(camWidth,camHeight);        //brightness map, not gonna be used but necessary 
    trackedImage.allocate(camWidth, camHeight);         //our postRange image basically
    colorTrackedPixelsRed = new unsigned char [camWidth * camHeight];     //rangeImage
    trackedTextureRed.allocate(camWidth, camHeight, GL_LUMINANCE);        //final output
    vidGrabber.setVerbose(true);                    //just some text for debugging
    vidGrabber.initGrabber(camWidth,camHeight);     //start the show!
    
    // Open communication protocol
    sender.setup( HOST, PORT );
}

//--------------------------------------------------------------
void testApp::update(){
    vidGrabber.grabFrame();                                                 //get a frame from the camera
    
    colorImgHSV.setFromPixels(vidGrabber.getPixels(), camWidth, camHeight);    //remember that colorImg? put the camera image into it
    colorImgHSV.convertRgbToHsv();                                          //now we convert the colorImg inside colorImgHSV into HSV
    colorImgHSV.convertToGrayscalePlanarImages(hueImg, satImg, briImg);     //distribute the hue, saturation and brightness to hueImg, satImg, and briImg
    
    hueImg.flagImageChanged(); 
    satImg.flagImageChanged();
    briImg.flagImageChanged();
    
    unsigned char * huePixels = hueImg.getPixels();                         //huePixels is now a raw array of pixels
    unsigned char * satPixels = satImg.getPixels();                         //satPixels is now a raw array of pixels  just like huePixels
    unsigned char * briPixels = briImg.getPixels();
    int nPixels = camWidth * camHeight;                                     //get the number of pixels in the images since these raw images are continuous, so no breaks
    
    // Color tracking filter
    for (int i = 0; i < nPixels; i++){                                           
        if ((huePixels[i] >= one.hue - 100 && huePixels[i] <= one.hue + 100) &&    
            (satPixels[i] >= one.sat - 12 && satPixels[i] <= one.sat + 12)) {    
            colorTrackedPixelsRed[i] = 255;                                 
        } 
        else {
            colorTrackedPixelsRed[i] = 0;                                        
        }
    }
    
    trackedImage.setFromPixels(colorTrackedPixelsRed, camWidth, camHeight);              // set reds from the colorTrackedPixelsRed array so it's all clean and openCv operable
    trackedImage.setROI(0, ROI_Y_OFFSET, camWidth, ROI_Y_HEIGHT);                        // clamp
    
    ballFinder.findContours(trackedImage, 50, 100, 5, false, false);                  //lets find one (1) blob in the grayscale openCv image reds
    
    // Filter out random blobs and get ball position
    for (int i = 0; i < ballFinder.blobs.size(); i++) {
        if (ballFinder.blobs[i].boundingRect.width < 15 && ballFinder.blobs[i].boundingRect.height < 15) {
            ballPos = ballFinder.blobs[i].centroid;
        }
    }
    
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
    glPopMatrix();
    
    ofColor c;
    c.setHsb(one.hue, one.sat, one.bri);
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
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
    unsigned char * huePixels = hueImg.getPixels();  //teh hue
    unsigned char * satPixels = satImg.getPixels();  //teh saturation
    unsigned char * briPixels = briImg.getPixels();  //teh brightness*/ //unnecessary really, hue and sat should be enough
    
    x = MIN(x,hueImg.width-1);     //find the smallest value out of those two so we don't crash if we click outside of the camera image
    y = MIN(y,hueImg.height-1);
    
    if(button == 0) {
        one.hue = huePixels[x+(y*hueImg.width)];  //set the hue
        one.sat = satPixels[x+(y*satImg.width)];  //set the sat
        one.bri = briPixels[x+(y*briImg.width)];
    }
}