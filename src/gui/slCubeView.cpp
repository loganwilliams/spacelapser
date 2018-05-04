//
//  slCubeView.cpp
//  spacelapser
//
//  Created by Logan Williams on 5/1/18.
//

#include "slCubeView.h"

slCubeView::slCubeView(slVideoCube * videoCube, int width, int height) {
    cube = videoCube;
    fbo.allocate(width, height);
    
    cam.setPosition(500, 500, 500);
    cam.lookAt(ofVec3f(0,0,0));
}

void slCubeView::initFirstFrame() {
    firstFrameImage = cube->getFrame(0);
}

void slCubeView::initLastFrame() {
    lastFrameImage = cube->getFrame(cube->frames-1);
}

void slCubeView::setParams(sliceParams newParams) {
    params = newParams;
}

void slCubeView::setT(float newT) {
    t = newT;
}

void slCubeView::setSize(int width, int height) {
    fbo.allocate(width, height);
}

ofFbo slCubeView::render(float t, ofTexture tex, bool drawSlice) {
    // start and clear the buffer we are drawing in to
    fbo.begin();
    ofClear(20,40,30,255);
    ofSetColor(255, 255, 255);
    
    cam.setLensOffset(ofVec2f(-0.15, 0));
    cam.begin();
    
    if (drawSlice) {
        // Draw plane
        ofMatrix3x3 transDir = getRotationMatrix(params.dirX, params.dirY, params.zSlider);
        ofVec3f travel_normal = matMul(ofVec3f(0,0,1), transDir);
        ofVec3f travel_direction = travel_normal * t;
        ofVec3f start_position = ofVec3f(travel_direction.x + params.outXOffset, travel_direction.y + params.outYOffset, travel_direction.z);
        
        slice.resizeToTexture(tex);
        slice.setHeight(params.outHeightSlider);
        slice.setWidth(params.outWidthSlider);
        tex.bind();
        ofSetColor(255, 255, 255, 200);
        slice.setOrientation(ofVec3f(params.xSlider, params.ySlider, params.zSlider));
        
        slice.setPosition(start_position);
        slice.draw();
        tex.unbind();
        
        // draw an arrow in the direction of t
        ofSetColor(127, 255, 127, 200);
        ofSetLineWidth(5);
        ofDrawArrow(start_position, start_position + travel_normal*100, 10);
    }
    
    // Draw cube
    ofSetColor(127, 255, 127);
    ofNoFill();
    ofSetLineWidth(2);
    ofDrawBox(cube->width, cube->frames, cube->height);
    
    ofSetColor(255,255,255,200);
    // Draw first frame of the movie on the bottom of the box
    firstFrame.resizeToTexture(firstFrameImage.getTexture());
    firstFrame.setPosition(0,-cube->frames/2, 0);
    firstFrame.setOrientation(ofVec3f(90, 0, 0));
    firstFrame.setHeight(cube->height);
    firstFrame.setWidth(cube->width);
    
    firstFrameImage.getTexture().bind();
    firstFrame.draw();
    firstFrameImage.getTexture().unbind();
    
    if (drawSlice) {
        // Draw last frame of the movie on the top of the box
        lastFrame.resizeToTexture(lastFrameImage.getTexture());
        lastFrame.setPosition(0, cube->frames/2, 0);
        lastFrame.setOrientation(ofVec3f(90, 0, 0));
        lastFrame.setHeight(cube->height);
        lastFrame.setWidth(cube->width);
        
        lastFrameImage.getTexture().bind();
        ofSetColor(255,255,255,200);
        lastFrame.draw();
        lastFrameImage.getTexture().unbind();
    } else {
        // Draw last frame of the movie on the top of the box
        lastFrame.resizeToTexture(tex);
        lastFrame.setPosition(0, -cube->frames/2 + t, 0);
        lastFrame.setOrientation(ofVec3f(90, 0, 0));
        lastFrame.setHeight(cube->height);
        lastFrame.setWidth(cube->width);
        tex.bind();
        ofSetColor(255,255,255,200);
        lastFrame.draw();
        tex.unbind();
    }
    
    // we're done
    cam.end();
    fbo.end();
    
    return fbo;
}
