//
//  slLinearAlgebra.hpp
//  spacelapser
//
//  Some helper linear algebra functions for making a 3D rotation matrix and applying it to a vector.
//
//  Created by Logan Williams on 4/27/18.
//

#pragma once

#include <stdio.h>
#include "ofMain.h"

#define sind(x) (sin(fmod((x),360) * M_PI / 180))
#define cosd(x) (cos(fmod((x),360) * M_PI / 180))

ofMatrix3x3 getRotationMatrix(float xAngle, float yAngle, float zAngle);
ofVec3f matMul(ofVec3f vec, ofMatrix3x3 mat);
