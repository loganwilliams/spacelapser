//
//  sliceParams.h
//  spacelapser
//
//  Created by Logan Williams on 4/27/18.
//

#pragma once

struct sliceParams {
    ofParameter<float>  xSlider, ySlider, zSlider, xSkew, ySkew;
    ofParameter<float>  outHeightSlider, outWidthSlider, outXOffset, outYOffset;
    ofParameter<float>  dirX, dirY;
};
