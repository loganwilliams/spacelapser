//
//  slTimeline.h
//  spacelapser
//
//  This class renders a timeline view of the current movie that would be exported.
//  It is constructed by passing a reference to an slVideoCube object, but also needs
//  the current slice parameters and the start and end of the movie to draw succesfully.
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
    slTimeline(slVideoCube * cube);
    void draw(int x, int y, int width, int height, int t);
    void setTBounds(int min, int max);
    void setParams(sliceParams params);
    
private:
    slVideoCube * videoCube;
    int tMax, tMin;
    sliceParams params;
};

