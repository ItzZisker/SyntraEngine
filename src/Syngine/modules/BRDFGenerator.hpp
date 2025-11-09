#pragma once

#include "Syngine/modules/Texture.hpp"

using namespace syng;

namespace syng
{
    void SG_generateBRDFLUT(TextureImage& img, int samples = 128, int size = 1024);
};
