#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxVideoRecorder.h"
#include "slTimeline.h"
#include "slVideoCube.h"
#include "slLinearAlgebra.h"
#include "slSliceParams.h"
#include "slCubeView.h"

enum class State{NoVideo, Loading, Playing, Saving};

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void recordVideo();
    void loadVideo();
    void matchMovement(float& parameter);
    
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
    
private:
    State               state;
    slVideoCube *       videoCube;

    // Loading and saving
    ofVideoPlayer       movie;
    int                 loadF;
    ofxVideoRecorder    vidRecorder;
    
    long                bytesPerRow, bytesPerFrame;
    
    // TODO: these should all be replaced by a GUI size struct of some sort
    int                 guiWidth, guiHeight;
    int                 previewWidth, previewHeight;
    int                 timelineWidth, timelineHeight;

    // Parameter GUI
    bool                drawGui;
    ofxPanel            gui;
    ofParameterGroup    transportGroup;
    ofxToggle           playToggle;
    ofParameter<float>  tSlider;

    ofParameterGroup    renderGroup;
    ofParameterGroup    advancedGroup;
    sliceParams         params;
    ofxToggle           hq;

    ofParameterGroup    outputGroup;
    ofParameter<int>    tMin, tMax;
    ofxButton           saveButton;
    ofxButton           loadButton;
    
    // Other GUI elements
    slTimeline *        timeline;
    slCubeView *        cubeView;
    
    // TODO this should be in the display preview class
    ofImage             displayed;
    ofImage             tv;

};
