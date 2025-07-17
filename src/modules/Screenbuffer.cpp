#include <modules/Screenbuffer.hpp>

using namespace syng;

Screenbuffer::Screenbuffer(unsigned int FBO) : FBO(FBO) {}

Screenbuffer::Screenbuffer(unsigned int FBO, unsigned int width, unsigned int height, bool outputToParent)
                            : FBO(FBO), width(width), height(height), outputToParent(outputToParent) {}

void Screenbuffer::onCreate(unsigned int width, unsigned int height, bool outputToParent, unsigned int FBO) {
    this->width = width;
    this->height = height;
    this->outputToParent = outputToParent;
    this->FBO = FBO;
}

unsigned int Screenbuffer::getFBO() {
    return this->FBO;
}

unsigned int Screenbuffer::getWidth() {
    return this->width;
}

unsigned int Screenbuffer::getHeight() {
    return this->height;
}

bool Screenbuffer::isOutputToParent() {
    return this->outputToParent;
}