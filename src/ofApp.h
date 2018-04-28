#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxVideoRecorder.h"
#include "slTimeline.h"
#include "slVideoCube.h"
#include "slLinearAlgebra.h"
#include "slSliceParams.h"

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

    void outSizeChanged(float & parameter);
    void drawSliceCube();

    ofxVideoRecorder    vidRecorder;
    void recordVideo();
    void loadVideo();

    
private:
    string              state;

    ofVideoPlayer       movie;
    int                 mFrames, mHeight, mWidth, mChannels, maxDim;
    unsigned char *     cube;
    slVideoCube *       videoCube;
    
    ofImage             displayed;
    long                bytesPerRow, bytesPerFrame;
    
    // TODO: these should all be replaced by a GUI size struct of some sort
    int                 guiWidth, guiHeight;
    int                 previewWidth, previewHeight;
    int                 timelineWidth, timelineHeight;

    // Parameter GUI
    ofxPanel            gui;
    ofParameterGroup    transportGroup;
    ofxToggle           playToggle;
    ofParameter<float>  tSlider;

    ofParameterGroup    renderGroup;
    sliceParams         params;
    ofxToggle           hq;

    ofParameterGroup    outputGroup;
    ofParameter<int>    tMin, tMax;
    ofxButton           saveButton;
    ofxButton           loadButton;
    
    // Timeline
    slTimeline *        timeline;
    
    // TODO the cube slicer should be its own class
    // 3D cube slicer
    ofEasyCam           cam;
    ofFbo               fbo;
    ofPlanePrimitive    slice;
    ofPlanePrimitive    firstFrame;
    ofImage             firstFrameImage;
    ofPlanePrimitive    lastFrame;
    ofTexture           lastFrameTex;

    int                 outHeight;
    int                 outWidth;
    long                bytesPerRowOutput, bytesPerFrameOutput;
    
    // TODO these should go in their own class too
    ofImage             tv, tvTransparent;
    
    bool                drawGui;
    int                 loadF;

};
