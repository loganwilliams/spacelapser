#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(255, 255, 255);
    ofSetFrameRate(120);
    
    state = "NOVIDEO";
    
    gui.setup();
    gui.add(loadButton.setup("Load video"));
    loadButton.addListener(this, &ofApp::loadVideo);
    
    tv.load("images/tv.png");
    tvTransparent.load("images/tv_transparent.png");
    
    drawGui = true;
    loadF = 0;
    
    videoCube = new slVideoCube;
    timeline = new slTimeline;
    timeline->init(videoCube);
}

void ofApp::loadVideo() {
    state = "LOADING";
    
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
    
    mFrames = movie.getTotalNumFrames();
    mHeight = (int)movie.getHeight();
    mWidth = (int)movie.getWidth();
    maxDim = mHeight > mWidth ? mHeight : mWidth;
    maxDim = mFrames > maxDim ? mFrames : maxDim;
    mChannels = 3;
    cout << "Video opened: " << mFrames << " frames @ " << mHeight << " x " << mWidth << ", " << mChannels << " channels.\n";
    
    videoCube->init(mFrames, mWidth, mHeight, mChannels);
    
    // Prepare some buffers for storing the preview image and orientation images for the cube
    outWidth = mWidth;
    outHeight = mFrames;
    displayed.allocate(outWidth, outHeight, OF_IMAGE_COLOR);
    bytesPerRowOutput = mChannels * outWidth;
    bytesPerFrameOutput = bytesPerRowOutput * outHeight;
    
    firstFrameImage.allocate(mWidth, mHeight, OF_IMAGE_COLOR);
    
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
    renderGroup.add(params.dirX.set("movement x ang", 0, -90, 90));
    renderGroup.add(params.dirY.set("movement y ang", 0, -90,  90));
    renderGroup.add(params.xSkew.set("x skew", 0, -1, 1));
    renderGroup.add(params.ySkew.set("y skew", 0, -1, 1));
    renderGroup.add(params.outWidthSlider.set("slice width", mWidth, 100, maxDim));
    renderGroup.add(params.outHeightSlider.set("slice height", mFrames, 100, maxDim));
    renderGroup.add(params.outXOffset.set("x offset", 0, -maxDim, maxDim));
    renderGroup.add(params.outYOffset.set("y offset", 0, -maxDim, maxDim));
    gui.add(renderGroup);
    gui.add(hq.setup("hq preview", false));
    
    outputGroup.setName("output");
    outputGroup.add(tMin.set("start coord",  -maxDim/2, -maxDim/2, maxDim/2-1));
    outputGroup.add(tMax.set("end coord",  maxDim/2-1, -maxDim/2, maxDim/2-1));
    gui.add(outputGroup);
    gui.add(saveButton.setup("save video"));
    
    params.outHeightSlider.addListener(this, &ofApp::outSizeChanged);
    params.outWidthSlider.addListener(this, &ofApp::outSizeChanged);
    
    saveButton.addListener(this, &ofApp::recordVideo);
    
    // setup the camera
    cam.setPosition(500, 500, 500);
    cam.lookAt(ofVec3f(0,0,0));
    
    // set up the 3D planes
    slice.set(outWidth, outHeight);
    lastFrame.set(mWidth, mHeight);
    firstFrame.set(mWidth, mHeight);
    
    windowResized(ofGetWidth(), ofGetHeight());
}

void ofApp::outSizeChanged(float & parameter) {
    outWidth = (int) params.outWidthSlider;
    outHeight = (int) params.outHeightSlider;
    
    bytesPerRowOutput = mChannels * outWidth;
    bytesPerFrameOutput = bytesPerRowOutput * outHeight;
    
    windowResized(ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::update(){
    if (state == "LOADING") {
        movie.update();
        
        int f = movie.getCurrentFrame();
        
        if (movie.isFrameNew()) {
            if (loadF > f) {
                loadF = f;
            }
            
            cout << "new frame " << f << "\n";
            ofPixels &pixels = movie.getPixels();
            
            if (f == 0) {
                // copy the first frame into a new image buffer and store it for later
                // TODO there should be a cleaner way of making a real copy of these
                ofPixels &copyRef = firstFrameImage.getPixels();
                
                for (int y = 0; y < mHeight; y++) {
                    for (int x = 0; x < mWidth; x++) {
                        for (int c = 0; c < mChannels; c++) {
                            copyRef[bytesPerRow * y + x * mChannels + c] = pixels[bytesPerRow * y + x * mChannels + c];
                            
                        }
                    }
                }
                
                firstFrameImage.setFromPixels(copyRef);
                firstFrameImage.update();
            }
            
            videoCube->addFrame(f, pixels.getData());
            
            // TODO figure out why we are sometimes skipping frames when loading (i.e., frame 491 in river_bank.mp4)
            // for now, this is a hack that just copies the next frame into the right position.
            // could be improved a bit with linear interpolation, but better to figure out the root cause
            if (loadF != f) {
                videoCube->addFrame(loadF, pixels.getData());
            }
            
            lastFrameTex = movie.getTexture();
            
            if (f < mFrames-1) {
                // get the next frame to do it again
                movie.nextFrame();
                loadF++;
            } else {
                // safe a reference to the last texture
                // TODO should probably copy this too, because who knows how stable this reference is
                // that said, it seems to work
                movie.close();
                state = "PLAYING";
                windowResized(ofGetWidth(), ofGetHeight());
            }
            
        }
        
    } else if (state == "PLAYING") {
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
        
        displayed = videoCube->getFrame(tSlider, hq, previewWidth, previewHeight, params);
        displayed.update();
    } else if (state == "SAVING") {
        tSlider = tSlider + 1;
        if (tSlider > tMax) {
            vidRecorder.close();
            state = "PLAYING";
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (state == "LOADING") {
        ofBackground(20,40, 30);
        int f = movie.getCurrentFrame();
        ofSetColor(127, 255, 127);
        ofDrawBitmapString("loading frame " + ofToString(f) + "/" + ofToString(mFrames), 20, 20);
        
        ofSetColor(255, 255, 255);
        drawSliceCube();
        fbo.draw(0,0);
  
    } else if (state == "PLAYING") {
        ofBackground(0,0,0);
        ofSetColor(0,0,0);
        ofDrawRectangle(guiWidth, 0, previewWidth, previewHeight);
        
        // Draw preview output image
        ofSetColor(255, 255, 255);
        tv.draw(guiWidth, 0, previewWidth, previewHeight);
        displayed.draw(guiWidth + previewWidth*.095, .128*previewHeight + .62 * previewHeight, .8*previewWidth, -.62 * previewHeight);

        // Draw 3D reference cube
        drawSliceCube();
        fbo.draw(0,0);
        ofSetColor(255,255,255);
        tvTransparent.draw(0,0,guiWidth,guiHeight);

        // Draw timeline
        timeline->setTBounds(tMin, tMax);
        timeline->setParams(params);
        timeline->draw(0, guiHeight, timelineWidth, timelineHeight, tSlider);
        
        // Draw GUI for adjusting parameters
        if (drawGui) {
            ofSetColor(255, 255, 255);
            gui.draw();
        }
    } else if (state == "SAVING") {
        ofBackground(0,0,0);
        // Draw preview output image
        ofSetColor(255, 255, 255);
        displayed = videoCube->getFrame(tSlider, hq, outWidth, outHeight, params);
        displayed.mirror(true, false);
        displayed.update();
        tv.draw(guiWidth, 0, previewWidth, previewHeight);
        displayed.draw(guiWidth + previewWidth*.095, .128*previewHeight + .62 * previewHeight, .8*previewWidth, -.62 * previewHeight);

        // Draw 3D reference cube
        drawSliceCube();
        fbo.draw(0,0);
        
        timeline->setTBounds(tMin, tMax);
        timeline->setParams(params);
        timeline->draw(0, guiHeight, timelineWidth, timelineHeight, tSlider);
    
        vidRecorder.addFrame(displayed.getPixels());
    } else if (state == "NOVIDEO") {
        ofBackground(20,40, 30);
        ofSetColor(255, 255, 255);
        gui.draw();
    }
}

void ofApp::drawSliceCube() {
    // start and clear the buffer we are drawing in to
    fbo.begin();
    ofClear(20,40,30,0);
    
    cam.setLensOffset(ofVec2f(-0.15, 0));
    cam.begin();
    
    if (state == "PLAYING" || state == "SAVING") {
        // Draw plane
        ofMatrix3x3 transDir = getRotationMatrix(params.dirX, params.dirY, params.zSlider);
        ofVec3f travel_normal = matMul(ofVec3f(0,0,1), transDir);
        ofVec3f travel_direction = travel_normal * tSlider;
        ofVec3f start_position = ofVec3f(travel_direction.x + params.outXOffset, travel_direction.y + params.outYOffset, travel_direction.z);
        
        slice.resizeToTexture(displayed.getTexture());
        slice.setHeight(outHeight);
        slice.setWidth(outWidth);
        displayed.getTexture().bind();
        ofSetColor(255, 255, 255, 200);
        slice.setOrientation(ofVec3f(params.xSlider, params.ySlider, params.zSlider));
        
        slice.setPosition(start_position);
        slice.draw();
        displayed.getTexture().unbind();
        
        // draw an arrow in the direction of t
        ofSetColor(127, 255, 127, 200);
        ofSetLineWidth(5);
        ofDrawArrow(start_position, start_position + travel_normal*100, 10);
    }
    
    // Draw cube
    ofSetColor(127, 255, 127);
    ofNoFill();
    ofSetLineWidth(2);
    ofDrawBox(mWidth, mFrames, mHeight);
    
    // Draw first frame of the movie on the bottom of the box
    firstFrame.resizeToTexture(firstFrameImage.getTexture());
    firstFrame.setPosition(0,-mFrames/2, 0);
    firstFrame.setOrientation(ofVec3f(90, 0, 0));
    firstFrame.setHeight(mHeight);
    firstFrame.setWidth(mWidth);
    
    firstFrameImage.getTexture().bind();
    ofSetColor(255,255,255,127);
    if (state == "LOADING") {
        ofSetColor(255,255,255,200);
    }
    firstFrame.draw();
    firstFrameImage.unbind();
    
    // Draw last frame of the movie on the top of the box
    if (state == "LOADING") {
        lastFrame.resizeToTexture(movie.getTexture());
        lastFrame.setPosition(0, -mFrames/2 + movie.getCurrentFrame(), 0);
        lastFrame.setOrientation(ofVec3f(90, 0, 0));
        lastFrame.setHeight(mHeight);
        lastFrame.setWidth(mWidth);

        movie.getTexture().bind();
        ofSetColor(255,255,255,200);
        lastFrame.draw();
        movie.getTexture().unbind();
    } else {
        lastFrame.resizeToTexture(lastFrameTex);
        lastFrame.setPosition(0, mFrames/2, 0);
        lastFrame.setOrientation(ofVec3f(90, 0, 0));
        lastFrame.setHeight(mHeight);
        lastFrame.setWidth(mWidth);

        lastFrameTex.bind();
        ofSetColor(255,255,255,127);
        lastFrame.draw();
        lastFrameTex.unbind();
    }
    
    // we're done
    cam.end();
    fbo.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
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
    
    if (state == "LOADING") {
        fbo = ofFbo();
        fbo.allocate(w, h);
    } else {
        fbo = ofFbo();
        fbo.allocate(guiWidth - previewWidth*.095, guiHeight - previewHeight*.128, GL_RGB);
    }
    
    displayed = ofImage();
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
    vidRecorder.setVideoCodec("mpeg4");
    vidRecorder.setVideoBitrate("40000k");
    
    ofFileDialogResult result = ofSystemSaveDialog("output.mp4", "Select save location");
    
    string path = "";
    
    if (result.bSuccess) {
        path = result.getPath();
        state = "SAVING";
    } else {
        return;
    }
    
    vidRecorder.setup(path, outWidth, outHeight, 30);
    vidRecorder.start();
    tSlider = tMin - 1;
}


