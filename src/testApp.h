#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"

#include "constants.h"

class color
{
public:
    float hue, sat, bri;    
};

class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void mousePressed(int x, int y, int button);
    
    ofVideoGrabber 	vidGrabber;         //our video grabber
    
    ofxCvColorImage		    colorImgHSV;    //the image doing the wrapping
    
    ofxCvGrayscaleImage		hueImg;     //Hue map
    ofxCvGrayscaleImage		satImg;     //Saturation map
    ofxCvGrayscaleImage     briImg;     //Brightness map
    
    ofxCvGrayscaleImage     trackedImage;               //Grayscale image we are gonna run the contour finder over to find our color
    
    color                   one;                //color that we're gonna track
    
    unsigned char *         colorTrackedPixelsRed;      //just some raw images which we are gonna put pixels into
    ofTexture               trackedTextureRed;          //color texture that we are gonna draw to
    
    ofxCvContourFinder      ballFinder;                  //contour finder, very handy 
    ofVec2f ballPos;
    
    // OSC sender
    ofxOscSender sender;
};

#endif