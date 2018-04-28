//
//  slVideoCube.h
//  spacelapser
//
//  Created by Logan Williams on 4/27/18.
//

#pragma once

#include <stdio.h>
#include "ofMain.h"
#include "slLinearAlgebra.h"
#include "slSliceParams.h"

class slVideoCube {
public:
    bool init(int frames, int width, int height, int channels);
    void addFrame(int f, unsigned char * pixels);
    ofImage getFrame(float t, bool hq, int resX, int resY, sliceParams params);

//private:
    unsigned char getPixel(int frame, int y, int x, int channel);
    unsigned char getPixel(float frame, float y, float x, int channel);
    
    unsigned char * cube;
    long                bytesPerRow, bytesPerFrame;
    int             height, width, frames, channels;
};
