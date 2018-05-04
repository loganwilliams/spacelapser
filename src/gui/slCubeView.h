//
//  slCubeView.hpp
//  spacelapser
//
//  Created by Logan Williams on 5/1/18.
//

#pragma once

#include <stdio.h>
#include "slVideoCube.h"
#include "slSliceParams.h"

class slCubeView {
public:
    slCubeView(slVideoCube * cube, int width, int height);
    void setSize(int width, int height);
    void setParams(sliceParams params);
    void setT(float t);
    ofFbo render(float t, ofTexture tex, bool drawSlice);
    void initFirstFrame();
    void initLastFrame();
private:
    slVideoCube *   cube;
    sliceParams     params;
    float           t;
    ofEasyCam           cam;
    ofFbo               fbo;
    ofPlanePrimitive    slice;
    ofPlanePrimitive    firstFrame, lastFrame;
    ofImage             firstFrameImage, lastFrameImage;
};
