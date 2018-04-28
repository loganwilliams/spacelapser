//
//  slTimeline.hpp
//  spacelapser
//
//  Created by Logan Williams on 4/27/18.
//

#pragma once

#include <stdio.h>
#include "ofMain.h"
#include "slVideoCube.h"
#include "slSliceParams.h"

class slTimeline {
public:
    void draw(int x, int y, int width, int height, int t);
    void setTBounds(int min, int max);
    void setParams(sliceParams params);
    void init(slVideoCube * cube);
    
private:
    slVideoCube * videoCube;
    int tMax, tMin;
    sliceParams params;
};

