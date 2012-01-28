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

using namespace cv;

class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void mousePressed(int x, int y, int button);
    void keyPressed  (int key);
    
    ofVideoGrabber 	vidGrabber;         //our video grabber
    
    ofxCvColorImage		    colorImg;    
    ofxCvColorImage		    colorImgHSV;    
    
    ofxCvGrayscaleImage		hueImg;     // Hue map
    ofxCvGrayscaleImage		satImg;     // Saturation map
    ofxCvGrayscaleImage     briImg;     // Brightness map
    
    ofxCvGrayscaleImage		redImg;     // Red map
    ofxCvGrayscaleImage		greenImg;     // Gren map
    ofxCvGrayscaleImage     blueImg;     // Blue map
    
    ofxCvGrayscaleImage     trackedImage;                   
    ofVec3f                 trackedColor;                
    
    unsigned char *         colorTrackedPixels;      
    ofTexture               trackedTextureRed;         
    
    ofxCvContourFinder      ballFinder;                  
    ofVec2f ballPos;
    ofVec2f kalmanBallPos;
    
    // OSC sender
    ofxOscSender sender;
    
    int cameraThreshold;
    
    KalmanFilter KF;
    Mat_<float> state;
    Mat processNoise;
    Mat_<float> measurement;
};

#endif