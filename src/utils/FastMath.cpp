#include "Syngine/utils/FastMath.hpp"
#include "glm/fwd.hpp"

float FastMath::glmLen2(glm::vec2 vec) {
    return vec[0]*vec[0] + vec[1]*vec[1];
}

float FastMath::glmLen2(glm::vec3 vec) {
    return vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
}

// Quake
float FastMath::inv_sqrt(float number) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y  = number;
    i  = *(long *) &y;                        // evil floating point bit level hacking
    i  = 0x5f3759df - (i >> 1);               // what the hell?
    y  = *(float *) &i;
    y  = y * (threehalfs - (x2 * y * y));     // 1st iteration

    return y;
}