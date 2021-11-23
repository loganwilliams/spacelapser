//
//  slLinearAlgebra.cpp
//  spacelapser
//
//  Created by Logan Williams on 4/27/18.
//

#include "slLinearAlgebra.h"

//--------------------------------------------------------------
// linear algebra helper functions

// makes a 3x3 rotation matrix (XYZ application order)
ofMatrix3x3 getRotationMatrix(float xAngle, float yAngle, float zAngle) {
    ofMatrix3x3 xMatrix = ofMatrix3x3(1, 0              , 0,
                                      0, cosd(xAngle)   , -sind(xAngle),
                                      0, sind(xAngle)   , cosd(xAngle));
    
    ofMatrix3x3 yMatrix = ofMatrix3x3(cosd(yAngle), 0, sind(yAngle), 0, 1, 0, -sind(yAngle), 0, cosd(yAngle));
    ofMatrix3x3 zMatrix = ofMatrix3x3(cosd(zAngle), -sind(zAngle), 0, sind(zAngle), cosd(zAngle), 0, 0, 0, 1);
    return zMatrix * yMatrix * xMatrix;
}

// multiply a 1x3 vector and a 3x3 matrix, producing a 1x3 vector
ofVec3f matMul(ofVec3f vec, ofMatrix3x3 mat) {
    return ofVec3f(mat.a * vec.x + mat.b * vec.y + mat.c * vec.z,
                   mat.d * vec.x + mat.e * vec.y + mat.f * vec.z,
                   mat.g * vec.x + mat.h * vec.y + mat.i * vec.z);
}
