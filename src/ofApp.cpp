#include "ofApp.h"
#include <cstdio>

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(255, 255, 255);
    ofSetFrameRate(120);
    
    state = "LOADING";
    
    // Load a movie to work with
    ofFileDialogResult result = ofSystemLoadDialog("Load file");
    string path = "";
    
    if(result.bSuccess) {
        path = result.getPath();
    }
    
    movie.load(path);
    movie.firstFrame();
    mFrames = movie.getTotalNumFrames();
    mHeight = (int)movie.getHeight();
    mWidth = (int)movie.getWidth();
    maxDim = mHeight > mWidth ? mHeight : mWidth;
    maxDim = mFrames > maxDim ? mFrames : maxDim;
    mChannels = 3;
    cout << "Video loaded: " << mFrames << " frames @ " << mHeight << " x " << mWidth << ", " << mChannels << " channels.\n";
    
    // Then allocate enough memory to store all of the video in a 3d array, the "cube"
    bytesPerRow = mChannels * mWidth;
    bytesPerFrame = bytesPerRow * mHeight;
    cout << bytesPerFrame << " bytes per frame.\n";
    long bytesTotal = bytesPerFrame * mFrames;
    cout << "Allocating memory" << bytesTotal << "\n";
    cube = new unsigned char[bytesTotal];
    
    // Prepare some buffers for storing the preview image and orientation images for the cube
    outWidth = mWidth;
    outHeight = mFrames;
    displayed.allocate(outWidth, outHeight, OF_IMAGE_COLOR);
    frame = displayed.getPixels().getData();
    bytesPerRowOutput = mChannels * outWidth;
    bytesPerFrameOutput = bytesPerRowOutput * outHeight;
    
    firstFrameImage.allocate(mWidth, mHeight, OF_IMAGE_COLOR);

    // setup the GUI
    gui.setup();
    
    transportGroup.setName("transport");
    transportGroup.add(tSlider.set("t parameter", 0, -maxDim/2, maxDim/2-1));
    gui.add(transportGroup);
    gui.add(playToggle.setup("playing", false));

    renderGroup.setName("render");
    renderGroup.add(xSlider.set("x angle", 0, -90, 90));
    renderGroup.add(ySlider.set("y angle", 0, -90, 90));
    renderGroup.add(outWidthSlider.set("output width", mWidth, 100, maxDim));
    renderGroup.add(outHeightSlider.set("output height", mFrames, 100, maxDim));
    renderGroup.add(dirX.set("t direction x", 0, -90, 90));
    renderGroup.add(dirY.set("t direction y", 0, -90,  90));
    gui.add(renderGroup);
    gui.add(hq.setup("hq preview", false));
    
    outputGroup.setName("output");
    outputGroup.add(tMin.set("t minimum (for save)",  -maxDim/2, -maxDim/2, maxDim/2-1));
    outputGroup.add(tMax.set("t maximum (for savee)",  maxDim/2-1, -maxDim/2, maxDim/2-1));
    gui.add(outputGroup);
    gui.add(saveButton.setup("save video"));
    
    // TODO add some preset options
    
    outHeightSlider.addListener(this, &ofApp::outSizeChanged);
    outWidthSlider.addListener(this, &ofApp::outSizeChanged);
    
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
    outWidth = (int) outWidthSlider;
    outHeight = (int) outHeightSlider;
    
    bytesPerRowOutput = mChannels * outWidth;
    bytesPerFrameOutput = bytesPerRowOutput * outHeight;
    
    windowResized(ofGetWidth(), ofGetHeight());
}

void ofApp::drawTimeline() {
    float aspect = (float) outWidth / outHeight;
    float tlAspect = (float) timelineWidth / timelineHeight;
    
    int n = (int) ceil(tlAspect / aspect);
    int w = aspect * timelineHeight;
    
    for (int i = 0; i < n; i++) {
        float t = (int) tMin + (tMax - tMin) * ((float) i / (n-1));
        getFrame(t, w, timelineHeight).draw(i * w + guiWidth, previewHeight + timelineHeight, w, -timelineHeight);
    }
    
    cout << "\n";
}

//--------------------------------------------------------------
void ofApp::update(){
    if (state == "LOADING") {
        movie.update();
        
        int f = movie.getCurrentFrame();
        
        if (movie.isFrameNew()) {
            
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
            
            // copy the frame values into a big 3d array at the appropriate spot
            for (int y = 0; y < mHeight; y++) {
                for (int x = 0; x < mWidth; x++) {
                    for (int c = 0; c < mChannels; c++) {
                        cube[bytesPerFrame * f + bytesPerRow * y + x * mChannels + c] = pixels[bytesPerRow * y + x * mChannels + c];
                    }
                }
            }
            
            lastFrameTex = movie.getTexture();
            
            if (f < mFrames-1) {
                // get the next frame to do it again
                movie.nextFrame();
            } else {
                // safe a reference to the last texture
                // TODO should probably copy this too, because who knows how stable this reference is
                // that said, it seems to work
                movie.close();
                state = "PLAYING";
            }
            
        }
        
    } else if (state == "PLAYING") {
        // if we're playing, advance the time ticker
        if (playToggle) {
            tSlider = tSlider + 1;
            
            if (tSlider >= maxDim/2) {
                tSlider = -maxDim/2;
            }
        }
        
        updateFrame();

    }
}

ofImage ofApp::getFrame(float t, int resX, int resY) {
    ofImage tmpFrame;
    tmpFrame.allocate(resX, resY, OF_IMAGE_COLOR);
    unsigned char * tmpPixels = tmpFrame.getPixels().getData();
    ofMatrix3x3 transDir = getRotationMatrix(dirX, dirY, 0);
    
    ofMatrix3x3 trans = getRotationMatrix(xSlider, ySlider, 0);
    ofVec3f rotated_coords, travel_direction;
    // I'm not sure if there's an overhead to using ofVec2fs, so I'm using floats.
    float normal_y, normal_x;
    float denormal_x, denormal_y, denormal_z;
    
    for (int y = 0; y < resY; y++) {
        for (int x = 0; x < resX; x++) {
            
            normal_y = ((((float) y) - ((float) resY)/2) / (((float) resY) / 2)) * (((float) outHeight) / 2);
            normal_x = ((((float) x) - ((float) resX)/2) / (((float) resX) / 2)) * (((float) outWidth) / 2);

            travel_direction = matMul(ofVec3f(0, 0, t), transDir);
            
            rotated_coords = matMul(ofVec3f(normal_x, normal_y, 0), trans);
            
            denormal_y = rotated_coords.y + mFrames / 2 + travel_direction.y;
            denormal_x = rotated_coords.x + mWidth / 2  + travel_direction.x;
            denormal_z = rotated_coords.z + mHeight / 2 + travel_direction.z;
            
            // now apply these values to every  channel
            if (hq) {
                for (int c = 0; c < mChannels; c++) {
                    tmpPixels[(mChannels * resX) * y + x * mChannels + c] = getPixel(denormal_y, denormal_z, denormal_x, c);
                }
            } else {
                for (int c = 0; c < mChannels; c++) {
                    tmpPixels[(mChannels * resX) * y + x * mChannels + c] = getPixel((int) denormal_y, (int) denormal_z, (int) denormal_x, c);
                }
            }
        }
    }
    
    tmpFrame.update();
    return tmpFrame;
}

// get the pixel values of the coordinates on the slice plane
void ofApp::updateFrame() {
    displayed = getFrame(tSlider, drawWidth, drawHeight);
    displayed.update();
}

// get the value of a pixel within the X-Y-F volume
unsigned char ofApp::getPixel(int frame, int y, int x, int channel) {
    if (frame < 0 || frame >= mFrames || x < 0 || x >= mWidth || y < 0 || y >= mHeight) {
        return 0;
    } else {
        return cube[frame * bytesPerFrame + y * bytesPerRow + x * mChannels + channel];
    }
}

// get the value of a pixel within the X-Y-F volume, using trilinear interpolation
unsigned char ofApp::getPixel(float frame, float y, float x, int channel) {
    // these thresholds are different so that we have a smooth fade to black at the sides.
    if (frame < -1 || frame > mFrames + 1 || x < -1 || x > mWidth + 1 || y < -1 || y > mHeight + 1) {
        return 0;
    } else {
        // tri-linear interpolation

        int fl = (int) floor(frame);
        int fh = (int) ceil(frame);
        int xl = (int) floor(x);
        int xh = (int) ceil(x);
        int yl = (int) floor(y);
        int yh = (int) ceil(y);
        
        unsigned char c000 = getPixel(fl, yl, xl, channel);
        unsigned char c001 = getPixel(fh, yl, xl, channel);
        unsigned char c010 = getPixel(fl, yh, xl, channel);
        unsigned char c100 = getPixel(fl, yl, xh, channel);
        unsigned char c011 = getPixel(fh, yh, xl, channel);
        unsigned char c101 = getPixel(fh, yl, xh, channel);
        unsigned char c110 = getPixel(fl, yh, xh, channel);
        unsigned char c111 = getPixel(fh, yh, xh, channel);
        
        float xd = x - xl;
        float yd = y - yl;
        float fd = frame - fl;
        
        float c00 = c000 * (1 - xd) + c100 * xd;
        float c01 = c001 * (1 - xd) + c101 * xd;
        float c10 = c010 * (1 - xd) + c110 * xd;
        float c11 = c011 * (1 - xd) + c111 * xd;
        
        float c0 = c00 * (1 - yd) + c10 * yd;
        float c1 = c01 * (1 - yd) + c11 * yd;
        
        return ((unsigned char) c0 * (1 - fd) + c1 * fd);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (state == "LOADING") {
        ofBackground(60,70, 80);
        int f = movie.getCurrentFrame();
        ofSetColor(255,255,255);
        movie.draw(guiWidth + drawX, drawY, drawWidth, drawHeight);
        ofSetColor(255, 119, 35, 200);
        ofDrawBitmapString("loading frame " + ofToString(f) + "/" + ofToString(mFrames), 20, 20);
        
        ofSetColor(255, 255, 255);
        drawSliceCube();
        fbo.draw(0,0);
  
    } else if (state == "PLAYING") {
        // blue-ish gray background
        ofBackground(60,70, 80);
        
        // Draw preview output image
        ofSetColor(255, 255, 255);
        displayed.draw(guiWidth + drawX, previewHeight - drawY, drawWidth, -drawHeight);

        // Draw 3D reference cube
        drawSliceCube();
        fbo.draw(0,0);
        
        // Draw GUI for adjjusting parameters
        ofSetColor(255, 255, 255);
        gui.draw();
        
        drawTimeline();
    }
}

void ofApp::drawSliceCube() {
    // start and clear the buffer we are drawing in to
    fbo.begin();
    ofClear(40,45,50,0);
    cam.begin();
    
    if (state == "PLAYING") {
        // Draw plane
        ofMatrix3x3 transDir = getRotationMatrix(dirX, dirY, 0);
        ofVec3f travel_normal = matMul(ofVec3f(0,0,1), transDir);
        ofVec3f travel_direction = travel_normal * tSlider;
        ofVec3f start_position = ofVec3f(travel_direction.x, travel_direction.y, travel_direction.z);
        
        slice.resizeToTexture(displayed.getTexture());
        slice.setHeight(outHeight);
        slice.setWidth(outWidth);
        displayed.getTexture().bind();
        ofSetColor(255, 255, 255, 200);
        slice.setOrientation(ofVec3f(xSlider, ySlider, 0));
        
        slice.setPosition(start_position);
        slice.draw();
        displayed.getTexture().unbind();
        
        // draw an arrow in the direction of t
        ofSetColor(255, 119, 35, 200);
        ofSetLineWidth(5);
        ofDrawArrow(start_position, start_position + travel_normal*100, 10);
    }
    
    // Draw cube
    ofSetColor(255);
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
        ofSetColor(255,255,255,127);
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
    if (x > guiWidth && y > previewHeight) {
        tSlider = tMin + (tMax - tMin) * ((float) ((float) x - guiWidth) / timelineWidth);
    }
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
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
    
    guiWidth = (int) (5.0/13.0 * (float) w);
    guiHeight = h;
    
    previewWidth = (int) (8.0/13.0 * (float) w);
    previewHeight = (int) (6.0/8.0 * (float) h);
    float previewAspect = (float) previewWidth / previewHeight;
    float frameAspect = (float) outWidth / outHeight;
    if (previewAspect < frameAspect) {
        // letterbox top and bottom
        drawHeight = previewWidth / frameAspect;
        drawWidth = previewWidth;
        
        drawX = 0;
        drawY = (previewHeight - drawHeight) / 2;
    } else {
        drawWidth = previewHeight * frameAspect;
        drawHeight = previewHeight;
        
        drawX = (previewWidth - drawWidth) / 2;
        drawY = 0;
    }
    
    timelineWidth = previewWidth;
    timelineHeight = (int) (2.0/8.0 * (float) h);
    
    fbo = ofFbo();
    fbo.allocate(guiWidth, guiHeight, GL_RGB);
    
    displayed = ofImage();
    displayed.allocate(drawWidth, drawHeight, OF_IMAGE_COLOR);
    frame = displayed.getPixels().getData();
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
    } else {
        return;
    }
    
    vidRecorder.setup(path, outWidth, outHeight, 30);
    
    vidRecorder.start();
    ofImage tmpIm;
    
    for (tSlider = tMin; tSlider < tMax; tSlider = tSlider + 1) {
        cout << "frame " << tSlider << "\n";
        tmpIm = getFrame(tSlider, outWidth, outHeight);
        tmpIm.mirror(true, false);
        vidRecorder.addFrame(tmpIm.getPixels());
        
    }
    
    vidRecorder.close();
}

//--------------------------------------------------------------
// linear algebra helper functions

// makes a 3x3 rotation matrix (XYZ application order)
ofMatrix3x3 ofApp::getRotationMatrix(float xAngle, float yAngle, float zAngle) {
    ofMatrix3x3 xMatrix = ofMatrix3x3(1, 0              , 0,
                                      0, cosd(xAngle)   , -sind(xAngle),
                                      0, sind(xAngle)   , cosd(xAngle));
    
    ofMatrix3x3 yMatrix = ofMatrix3x3(cosd(yAngle), 0, sind(yAngle), 0, 1, 0, -sind(yAngle), 0, cosd(yAngle));
    ofMatrix3x3 zMatrix = ofMatrix3x3(cosd(zAngle), -sind(zAngle), 0, sind(zAngle), cosd(zAngle), 0, 0, 0, 1);
    return zMatrix * yMatrix * xMatrix;
}

// multiply a 1x3 vector and a 3x3 matrix, producing a 1x3 vector
ofVec3f ofApp::matMul(ofVec3f vec, ofMatrix3x3 mat) {
    return ofVec3f(mat.a * vec.x + mat.b * vec.y + mat.c * vec.z,
                   mat.d * vec.x + mat.e * vec.y + mat.f * vec.z,
                   mat.g * vec.x + mat.h * vec.y + mat.i * vec.z);
}

