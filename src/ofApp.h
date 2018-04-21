#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxVideoRecorder.h"

#define sind(x) (sin(fmod((x),360) * M_PI / 180))
#define cosd(x) (cos(fmod((x),360) * M_PI / 180))


class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    unsigned char getPixel(int frame, int y, int x, int channel);
    unsigned char getPixel(float frame, float y, float x, int channel);
    
    ofImage getFrame(float t, int resX, int resY);
    void updateFrame();
    ofMatrix3x3 getRotationMatrix(float xAngle, float yAngle, float zAngle);
    ofVec3f matMul(ofVec3f vec, ofMatrix3x3 mat);
    void outSizeChanged(float & parameter);
    void drawSliceCube();

    ofxVideoRecorder    vidRecorder;
    void recordVideo();
    void drawTimeline();

    
private:
    string              state;

    ofVideoPlayer       movie;
    int                 mFrames, mHeight, mWidth, mChannels, maxDim;
    unsigned char *     cube;
    
    ofImage             displayed;
    unsigned char *     frame;
    long                bytesPerRow, bytesPerFrame;
    
    int                 guiWidth, guiHeight;
    int                 previewWidth, previewHeight;
    int                 timelineWidth, timelineHeight;
    int                 drawHeight, drawWidth, drawX, drawY;

    // gui
    ofxPanel            gui;
    ofParameterGroup    transportGroup;
    ofxToggle           playToggle;
    ofParameter<float>  tSlider;

    ofParameterGroup    renderGroup;
    ofParameter<float>  xSlider, ySlider;
    ofParameter<float>  outHeightSlider, outWidthSlider;
    ofParameter<float>  dirX, dirY;
    ofxToggle           hq;

    ofParameterGroup    outputGroup;
    ofParameter<float>  tMin, tMax;
    ofxButton           saveButton;
    
    
    
    // 3D cube slicer
    ofCamera            cam;
    ofFbo               fbo;
    ofPlanePrimitive    slice;
    ofPlanePrimitive    firstFrame;
    ofImage             firstFrameImage;
    ofPlanePrimitive    lastFrame;
    ofTexture           lastFrameTex;

    int                 outHeight;
    int                 outWidth;
    long                bytesPerRowOutput, bytesPerFrameOutput;

};
