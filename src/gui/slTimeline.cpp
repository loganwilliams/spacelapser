//
//  slTimeline.cpp
//  spacelapser
//
//  Created by Logan Williams on 4/27/18.
//

#include "slTimeline.h"

slTimeline::slTimeline(slVideoCube * cube) {
    videoCube = cube;
}

void slTimeline::setTBounds(int min, int max) {
    tMin = min;
    tMax = max;
}

void slTimeline::setParams(sliceParams params) {
    slTimeline::params = params;
}

void slTimeline::draw(int x, int y, int width, int height, int t) {
    ofSetColor(0, 0, 0);
    ofDrawRectangle(x, y, width, height);
    ofSetColor(255, 255, 255);

    float tickInterval = ((float) width / (tMax - tMin)) * 100;

    for (float i = 0; i < width; i+= tickInterval) {
        ofSetColor(64, 127, 64);
        ofDrawLine(i, y, i, y + 12);
        ofDrawLine(i, y + height - 12, i, y + height);
    }

    int timelineDrawHeight = height - 30;
    float aspect = (float) params.outWidthSlider / params.outHeightSlider;
    float tlAspect = (float) width / timelineDrawHeight;

    int n = (int) ceil(tlAspect / aspect);
    int w = aspect * timelineDrawHeight;

    ofSetColor(255);
    for (int i = 0; i < n; i++) {
        float t = (int) tMin + (tMax - tMin) * ((float) i / (n));
        videoCube->getFrame(t, false, w, timelineDrawHeight, params).draw(i * w + x, y + height - 15, w, -(height - 30));
    }

    float scrubberPos = ((float) (t - tMin) / (tMax - tMin)) * width;

    if (scrubberPos > 0 && scrubberPos < width) {
        ofSetColor(127, 255, 127, 200);
        ofDrawRectangle(scrubberPos-1, y, 3, height);
    }
    
    // reset color
    ofSetColor(255, 255, 255);
}
