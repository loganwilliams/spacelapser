//
//  slVideoCube.cpp
//  spacelapser
//
//  Created by Logan Williams on 4/27/18.
//

#include "slVideoCube.h"

bool slVideoCube::init(int mFrames, int mWidth, int mHeight, int mChannels) {
    height = mHeight;
    width = mWidth;
    frames = mFrames;
    channels = mChannels;
    
    // Then allocate enough memory to store all of the video in a 3d array, the "cube"
    bytesPerRow = channels * width;
    bytesPerFrame = bytesPerRow * height;
    cout << bytesPerFrame << " bytes per frame.\n";
    long bytesTotal = bytesPerFrame * frames;
    
    if (bytesTotal/(1024*1024*1024) > 16) {
        cout << "Video too large, would require " << ((float) (bytesTotal/1024*1024*1024)) << "GB.\n";
        return false;
    }
    
    cout << "Allocating memory: " << (bytesTotal/1024*1024*1024) << "GB\n";
    if (cube) {
        delete cube;
    }
    cube = new unsigned char[bytesTotal];
    return true;
}

void slVideoCube::addFrame(int f, unsigned char * pixels) {
    // copy the frame values into a big 3d array at the appropriate spot
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                cube[bytesPerFrame * f + bytesPerRow * y + x * channels + c] = pixels[bytesPerRow * y + x * channels + c];
            }
        }
    }
}

ofImage slVideoCube::getFrame(float t, bool hq, int resX, int resY, sliceParams params) {
    ofImage tmpFrame;
    tmpFrame.allocate(resX, resY, OF_IMAGE_COLOR);

    unsigned char * tmpPixels = tmpFrame.getPixels().getData();
    ofMatrix3x3 transDir = getRotationMatrix(params.dirX, params.dirY, 0);
    
    ofMatrix3x3 trans = getRotationMatrix(params.xSlider, params.ySlider, params.zSlider);
    ofVec3f rotated_coords, travel_direction;
    // I'm not sure if there's an overhead to using ofVec2fs, so I'm using floats.
    float normal_y, normal_x;
    float denormal_x, denormal_y, denormal_z;
    
    for (int y = 0; y < resY; y++) {
        for (int x = 0; x < resX; x++) {
            normal_y = ((((float) y) - ((float) resY)/2) / (((float) resY) / 2)) * (((float) params.outHeightSlider) / 2);
            normal_x = ((((float) x) - ((float) resX)/2) / (((float) resX) / 2)) * (((float) params.outWidthSlider) / 2);

            travel_direction = matMul(ofVec3f(0, 0, t), transDir);

            rotated_coords = matMul(ofVec3f(normal_x, normal_y, 0), trans);

            denormal_y = rotated_coords.y + frames / 2 + travel_direction.y + params.outYOffset;
            denormal_x = rotated_coords.x + width / 2  + travel_direction.x + params.outXOffset;
            denormal_z = rotated_coords.z + height / 2 + travel_direction.z;

            // now apply these values to every  channel
            if (hq) {
                for (int c = 0; c < channels; c++) {
                    tmpPixels[(channels * resX) * y + x * channels + c] = getPixel(denormal_y, denormal_z - denormal_y * params.ySkew, denormal_x - denormal_y * params.xSkew, c);
                }
            } else {
                for (int c = 0; c < channels; c++) {
                    tmpPixels[(channels * resX) * y + x * channels + c] = getPixel((int) (denormal_y), (int) (denormal_z - denormal_y * params.ySkew), (int) (denormal_x - denormal_y * params.xSkew), c);
                }
            }
        }
    }
    
    tmpFrame.update();
    return tmpFrame;
}


// get the value of a pixel within the X-Y-F volume
unsigned char slVideoCube::getPixel(int frame, int y, int x, int channel) {
    if (frame < 0 || frame >= frames || x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    } else {
        return cube[frame * bytesPerFrame + y * bytesPerRow + x * channels + channel];
    }
}

// get the value of a pixel within the X-Y-F volume, using trilinear interpolation
unsigned char slVideoCube::getPixel(float frame, float y, float x, int channel) {
    // these thresholds are different so that we have a smooth fade to black at the sides.
    if (frame < -1 || frame > frames + 1 || x < -1 || x > width + 1 || y < -1 || y > height + 1) {
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
