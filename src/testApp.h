#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"

#include "constants.h"

using namespace cv;

class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void mousePressed(int x, int y, int button);
    void keyPressed(int key);
    
    ofVideoGrabber 	vidGrabber;         //our video grabber
    
    ofxCvColorImage		    colorImgHSV;    //the image doing the wrapping
    
    ofxCvGrayscaleImage		hueImg;     //Hue map
    ofxCvGrayscaleImage		satImg;     //Saturation map
    ofxCvGrayscaleImage     briImg;     //Brightness map
    
    ofxCvColorImage 	background;
    
    ofxCvGrayscaleImage		hueImgBg;     
    ofxCvGrayscaleImage		satImgBg;    
    ofxCvGrayscaleImage     briImgBg;   

    ofxCvGrayscaleImage		hueImgPeaks;     //Hue map
    ofxCvGrayscaleImage		satImgPeaks;     //Saturation map
    ofxCvGrayscaleImage     briImgPeaks;     //Brightness map
    
    bool bLearnBackground;
    int thresholdHue;
    int thresholdSat;
    int thresholdBri;
    
    ofxCvGrayscaleImage     trackedImage;               //Grayscale image we are gonna run the contour finder over to find our color
    
    unsigned char *         satPixelsRaw; 
    unsigned char *         huePixelsRaw; 
    unsigned char *         briPixelsRaw; 
    
    
    ofxCvContourFinder      ballFinder;                  //contour finder, very handy 
    ofVec2f ballPos;
    ofVec2f kalmanBallPos;
    
    // OSC sender
    ofxOscSender sender;
    
    KalmanFilter KF;
    Mat_<float> state;
    Mat processNoise;
    Mat_<float> measurement;
};

#endif