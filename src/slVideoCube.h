//
//  slVideoCube.h
//  spacelapser
//
//  Loads an entire video into memory and allows arbitrary planar slices out of that
//  three-dimensional array to be taken, with or without interpolation.
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
    ofImage getFrame(float t, bool interpolate, int resX, int resY, sliceParams params);
    ofImage getFrame(int frame);
    int             height, width, frames, channels;


private:
    unsigned char getPixel(int frame, int y, int x, int channel);
    unsigned char getPixelUnsafe(int frame, int y, int x, int channel);
    unsigned char getPixel(float frame, float y, float x, int channel);
    
    unsigned char * cube;
    long            bytesPerRow, bytesPerFrame;
    bool            initialized = false;
};
