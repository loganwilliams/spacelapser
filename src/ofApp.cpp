#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(255, 255, 255);
    ofSetFrameRate(120);
    
    state = State::NoVideo;
    
    gui.setup();
    gui.add(loadButton.setup("Load video"));
    loadButton.addListener(this, &ofApp::loadVideo);
    
    tv.load("images/tv_transparent.png");
    
    drawGui = true;
    loadF = 0;
    
    videoCube = new slVideoCube;
    timeline = new slTimeline(videoCube);
    cubeView = new slCubeView(videoCube, ofGetWidth(), ofGetHeight());
}

void ofApp::loadVideo() {
    state = State::Loading;
    
    // Load a movie to work with
    ofFileDialogResult result = ofSystemLoadDialog("Load file");
    string path = "";
    
    if(result.bSuccess) {
        path = result.getPath();
    }
    
    movie.load(path);
    movie.firstFrame();
    movie.setPaused(true);
    movie.play();
    movie.stop();
    
    int mFrames = movie.getTotalNumFrames();
    int mHeight = (int)movie.getHeight();
    int mWidth = (int)movie.getWidth();
    int maxDim = mHeight > mWidth ? mHeight : mWidth;
    maxDim = mFrames > maxDim ? mFrames : maxDim;
    int mChannels = 3;
    cout << "Video opened: " << mFrames << " frames @ " << mHeight << " x " << mWidth << ", " << mChannels << " channels.\n";
    
    videoCube->init(mFrames, mWidth, mHeight, mChannels);
    
    // Prepare some buffers for storing the preview image and orientation images for the cube
    displayed.allocate(mWidth, mFrames, OF_IMAGE_COLOR);
    
    // setup the GUI
    gui.setup();
    
    gui.add(loadButton.setup("Load video"));
    
    transportGroup.setName("transport");
    transportGroup.add(tSlider.set("t parameter", 0, -maxDim/2, maxDim/2-1));
    gui.add(transportGroup);
    gui.add(playToggle.setup("playing", false));
    
    renderGroup.setName("render");
    renderGroup.add(params.xSlider.set("slice x ang", 0, -90, 90));
    renderGroup.add(params.ySlider.set("slice y ang", 0, -90, 90));
    renderGroup.add(params.zSlider.set("slice z ang", 0, -180, 180));
    renderGroup.add(params.outWidthSlider.set("slice width", mWidth, 100, maxDim));
    renderGroup.add(params.outHeightSlider.set("slice height", mFrames, 100, maxDim));
    gui.add(renderGroup);
    gui.add(hq.setup("hq preview", false));
    
    advancedGroup.setName("advanced");
    advancedGroup.add(params.dirX.set("movement x ang", 0, -90, 90));
    advancedGroup.add(params.dirY.set("movement y ang", 0, -90,  90));
    advancedGroup.add(params.xSkew.set("x skew", 0, -1, 1));
    advancedGroup.add(params.ySkew.set("y skew", 0, -1, 1));
    advancedGroup.add(params.outXOffset.set("x offset", 0, -maxDim, maxDim));
    advancedGroup.add(params.outYOffset.set("y offset", 0, -maxDim, maxDim));
    gui.add(advancedGroup);
    
    outputGroup.setName("output");
    outputGroup.add(tMin.set("start coord",  -maxDim/2, -maxDim/2, maxDim/2-1));
    outputGroup.add(tMax.set("end coord",  maxDim/2-1, -maxDim/2, maxDim/2-1));
    gui.add(outputGroup);
    gui.add(saveButton.setup("save video"));
    
    saveButton.addListener(this, &ofApp::recordVideo);
    params.xSlider.addListener(this, &ofApp::matchMovement);
    params.ySlider.addListener(this, &ofApp::matchMovement);
    
    windowResized(ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::update(){
    if (state == State::Loading) {
        movie.update();
        
        int f = movie.getCurrentFrame();
        
        if (movie.isFrameNew()) {
            if (loadF > f) {
                loadF = f;
            }
            
            cout << "new frame " << f << "\n";

            videoCube->addFrame(f, movie.getPixels().getData());
            
            // TODO figure out why we are sometimes skipping frames when loading (i.e., frame 491 in river_bank.mp4)
            // for now, this is a hack that just copies the next frame into the right position.
            // could be improved a bit with linear interpolation, but better to figure out the root cause
            if (loadF != f) {
                videoCube->addFrame(loadF, movie.getPixels().getData());
            }
            
            if (f == 0) {
                cubeView->initFirstFrame();
            }
            
            if (f < videoCube->frames-1) {
                // get the next frame to do it again
                movie.nextFrame();
                loadF++;
            } else {
                cubeView->initLastFrame();
                movie.close();
                state = State::Playing;
                windowResized(ofGetWidth(), ofGetHeight());
            }
            
        }
        
    } else if (state == State::Playing) {
        // if we're playing, advance the time ticker
        if (playToggle) {
            tSlider = tSlider + 1;
            
            if (tSlider >= tMax) {
                tSlider = tMin;
            }
            
            if (tSlider < tMin) {
                tSlider = tMin;
            }
        }
        
        cubeView->setParams(params);
        displayed = videoCube->getFrame(tSlider.get(), hq, previewWidth, previewHeight, params);
        displayed.update();
    } else if (state == State::Saving) {
        tSlider = tSlider + 1;
        if (tSlider > tMax) {
            vidRecorder.close();
            state = State::Playing;
            hq = false;
            drawGui = true;
        }
        
        displayed = videoCube->getFrame(tSlider.get(), hq, params.outWidthSlider, params.outHeightSlider, params);
        displayed.mirror(true, false);
        displayed.update();
        vidRecorder.addFrame(displayed.getPixels());
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0,0,0);
    ofSetColor(255, 255, 255);

    if (state == State::NoVideo) {
        ofBackground(20,40, 30);
        gui.draw();
        
    } else if (state == State::Loading) {
        ofBackground(20,40, 30);
        int f = movie.getCurrentFrame();
        ofSetColor(127, 255, 127);
        ofDrawBitmapString("loading frame " + ofToString(f) + "/" + ofToString(videoCube->frames), 20, 20);
        
        ofSetColor(255, 255, 255);
        
        cubeView->render(f, movie.getTexture(), false).draw(0,0);
  
    } else {
        // Draw preview output image
        tv.draw(guiWidth, 0, previewWidth, previewHeight);
        displayed.draw(guiWidth + previewWidth*.095, .128*previewHeight + .62 * previewHeight, .8*previewWidth, -.62 * previewHeight);

        // Draw 3D reference cube
        ofSetColor(255,255,255);
        cubeView->render(tSlider, displayed.getTexture(), true).draw(0,0);
        tv.draw(0,0,guiWidth,guiHeight);

        // Draw timeline
        timeline->setTBounds(tMin, tMax);
        timeline->setParams(params);
        timeline->draw(0, guiHeight, timelineWidth, timelineHeight, tSlider);
        
        // Draw GUI for adjusting parameters
        if (drawGui) {
            gui.draw();
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ' && state != State::Saving) {
        drawGui = !drawGui;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
    // if we clicked inside the timeline, scrub the current time
    if (y > previewHeight) {
        tSlider = tMin + (tMax - tMin) * ((float) ((float) x) / ofGetWidth());
    }
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if (y > previewHeight) {
        tSlider = tMin + (tMax - tMin) * ((float) ((float) x) / ofGetWidth());
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
    guiWidth = (int) (0.5 * (float) w);
    guiHeight = (int) (7.0/8.0 * (float) h);
    
    previewWidth = (int) (0.5 * (float) w);
    previewHeight = (int) (7.0/8.0 * (float) h);
    
    timelineWidth = w;
    timelineHeight = (int) (1.0/8.0 * (float) h);
    
    if (state == State::Loading) {
        cubeView->setSize(w, h);
    } else {
        cubeView->setSize(guiWidth - previewWidth*.095, guiHeight - previewHeight*.128);
    }
    
    displayed.allocate(previewWidth, previewHeight, OF_IMAGE_COLOR);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

void ofApp::recordVideo() {
    cout << "recording video \n";
    hq = true;
    drawGui = false;
    vidRecorder.setVideoCodec("mpeg4");
    vidRecorder.setVideoBitrate("40000k");
    
    ofFileDialogResult result = ofSystemSaveDialog("output.mp4", "Select save location");
    
    string path = "";
    
    if (result.bSuccess) {
        path = result.getPath();
        state = State::Saving;
    } else {
        return;
    }
    
    vidRecorder.setup(path, params.outWidthSlider, params.outHeightSlider, 30);
    vidRecorder.start();
    tSlider = tMin - 1;
}

void ofApp::matchMovement(float & parameter) {
    params.dirX = params.xSlider;
    params.dirY = params.ySlider;
}
