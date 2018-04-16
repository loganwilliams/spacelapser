#include "ofApp.h"
#include <cstdio>

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(255, 255, 255);
    ofSetFrameRate(120);
    
    fbo.allocate(600, 768, GL_RGB);
    
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
    
    gui.add(playToggle.setup("playing", false));
    gui.add(tSlider.setup("t parameter", 0, 0, 0.995));
    gui.add(xSlider.setup("x angle", 0, -45, 45));
    gui.add(ySlider.setup("y angle", 0, -45, 45));
    gui.add(outWidthSlider.set("output width", mWidth, 100, 1920));
    gui.add(outHeightSlider.set("output height", mFrames, 100, 1920));
    
    outHeightSlider.addListener(this, &ofApp::outSizeChanged);
    outWidthSlider.addListener(this, &ofApp::outSizeChanged);
    
    // setup the camera
    cam.setPosition(100, 100, 200);
    cam.lookAt(ofVec3f(0,0,0));
    
    // set up the 3D planes
    slice.set(outWidth, outHeight);
    lastFrame.set(mWidth, mHeight);
    firstFrame.set(mWidth, mHeight);
}

void ofApp::outSizeChanged(float & parameter) {
    outWidth = (int) outWidthSlider;
    outHeight = (int) outHeightSlider;
    
    displayed.allocate(outWidth, outHeight, OF_IMAGE_COLOR);
    frame = displayed.getPixels().getData();
    
    bytesPerRowOutput = mChannels * outWidth;
    bytesPerFrameOutput = bytesPerRowOutput * outHeight;
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
            
            if (f < mFrames-1) {
                // get the next frame to do it again
                movie.nextFrame();
            } else {
                // safe a reference to the last texture
                // TODO should probably copy this too, because who knows how stable this reference is
                // that said, it seems to work
                lastFrameTex = movie.getTexture();
                movie.close();
                state = "PLAYING";
            }
            
        }
        
    } else if (state == "PLAYING") {
        // if we're playing, advance the time ticker
        if (playToggle) {
            tSlider = tSlider + 0.005;
            if (tSlider > 1) {
                tSlider = 0;
            }
        }
        
        updateFrame();

    }
}

// get the pixel values of the coordinates on the slice plane
void ofApp::updateFrame() {
    ofMatrix3x3 trans = getRotationMatrix(xSlider, ySlider, 0);
    ofVec3f rotated_coords;
    // I'm not sure if there's an overhead to using ofVec2fs, so I'm using floats.
    float normal_y, normal_x;
    int denormal_x, denormal_y, denormal_z;
    
    for (int y = 0; y < outHeight; y++) {
        for (int x = 0; x < outWidth; x++) {
            
            normal_y = (float) y - outHeight/2;
            normal_x = (float) x - outWidth/2;
            
            // TODO allow movement along any axis, not just the z axis.
            // this will be where the magic happens
            
            rotated_coords = matMul(ofVec3f(normal_x, normal_y, 0), trans);
            
            denormal_y = rotated_coords.y + mFrames / 2;
            denormal_x = rotated_coords.x + mWidth / 2;
            denormal_z = rotated_coords.z + tSlider * mHeight;
            
            // now apply these values to every  channel
            for (int c = 0; c < mChannels; c++) {
                frame[bytesPerRowOutput * y + x * mChannels + c] = getPixel(denormal_y, denormal_z, denormal_x, c);
            }
        }
    }

    displayed.setFromPixels(frame, outWidth, outHeight, OF_IMAGE_COLOR);
    displayed.update();
}

// get the value of a pixel within the X-Y-F volume
// TODO make a float version that does interpolation
unsigned char ofApp::getPixel(int frame, int y, int x, int channel) {
    if (frame < 0 || frame >= mFrames || x < 0 || x >= mWidth || y < 0 || y >= mHeight) {
        return 0;
    } else {
        return cube[frame * bytesPerFrame + y * bytesPerRow + x * mChannels + channel];
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (state == "LOADING") {
        int f = movie.getCurrentFrame();
        movie.draw(0, 40, 640, 400);
        ofSetHexColor(0x888888);
        ofDrawBitmapString("loading frame " + ofToString(f) + "/" + ofToString(mFrames), 20, 20);
  
    } else if (state == "PLAYING") {
        // blue-ish gray background
        ofBackground(60,70, 80);
        
        // Draw preview output image
        ofSetColor(255, 255, 255);
        if (outWidth > 800) {
            int drawWidth = 800;
            int drawHeight = (drawWidth * outHeight) / outWidth;
            displayed.draw(600, drawHeight, drawWidth, -drawHeight);
        } else {
            displayed.draw(600, outHeight, outWidth, -outHeight);
        }
        
        // Draw 3D reference cube
        drawSliceCube();
        fbo.draw(0,0);
        
        // Draw GUI for adjjusting parameters
        ofSetColor(255, 255, 255);
        gui.draw();
    }
}

void ofApp::drawSliceCube() {
    // start and clear the buffer we are drawing in to
    fbo.begin();
    ofClear(60,70,80,0);
    cam.begin();
    
    // Draw plane
    slice.resizeToTexture(displayed.getTexture());
    slice.setHeight(outHeight);
    slice.setWidth(outWidth);
    displayed.getTexture().bind();
    ofSetColor(255, 255, 255, 200);
    slice.setOrientation(ofVec3f(xSlider, ySlider, 0));
    slice.setPosition(0, 0, tSlider*mHeight - mHeight/2);
    slice.draw();
    displayed.getTexture().unbind();
    
    // Draw cube
    ofSetColor(255);
    ofNoFill();
    ofSetLineWidth(1);
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
    lastFrame.resizeToTexture(lastFrameTex);
    lastFrame.setPosition(0, mFrames/2, 0);
    lastFrame.setOrientation(ofVec3f(90, 0, 0));
    lastFrame.setHeight(mHeight);
    lastFrame.setWidth(mWidth);
    
    lastFrameTex.bind();
    ofSetColor(255,255,255,127);
    lastFrame.draw();
    lastFrameTex.unbind();
    
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
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

//--------------------------------------------------------------
// linear algebra helper functions

// makes a 3x3 rotation matrix (XYZ application order)
ofMatrix3x3 ofApp::getRotationMatrix(float xAngle, float yAngle, float zAngle) {
    ofMatrix3x3 xMatrix = ofMatrix3x3(1, 0, 0, 0, cosd(xAngle), -sind(xAngle), 0, sind(xAngle), cosd(xAngle));
    ofMatrix3x3 yMatrix = ofMatrix3x3(cosd(yAngle), 0, sind(yAngle), 0, 1, 0, -sind(yAngle), 0, cosd(yAngle));
    ofMatrix3x3 zMatrix = ofMatrix3x3(cosd(zAngle), -sind(zAngle), 0, sind(zAngle), cosd(zAngle), 0, 0, 0, 1);
    return zMatrix * yMatrix * xMatrix;
}

// multiply a 1x3 vector and a 3x3 matrix, producing a 1x3 vector
ofVec3f ofApp::matMul(ofVec3f vec, ofMatrix3x3 mat) {
    return ofVec3f(mat.a * vec.x + mat.b * vec.y + mat.c * vec.z,
                   mat.d * vec.x + mat.e * vec.y + mat.f * vec.z,
                   mat.g * vec.x + mat.h * vec.y + mat.h * vec.z);
}

