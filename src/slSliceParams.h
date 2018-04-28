//
//  sliceParams.h
//  spacelapser
//
//  Created by Logan Williams on 4/27/18.
//

#pragma once

struct sliceParams {
    ofParameter<float>  xSlider, ySlider, zSlider, xSkew, ySkew;
    ofParameter<int>    outHeightSlider, outWidthSlider;
    ofParameter<float>  outXOffset, outYOffset;
    ofParameter<float>  dirX, dirY;
};
