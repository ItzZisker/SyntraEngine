#pragma once

namespace syng
{
class SetupObject {
protected:
    bool created = false;

    void onCreate() {
        this->created = true;
    }
public:
    bool isCreated() {
        return this->created;
    }
};

class Screenbuffer : public SetupObject {
protected:
    unsigned int FBO = 0;
    unsigned int width = 0, height = 0;
    bool outputToParent = false;

    void onCreate(unsigned int width, unsigned int height, bool outputToParent, unsigned int FBO = 0);
public:
    Screenbuffer(unsigned int FBO = 0);

    Screenbuffer(unsigned int FBO, unsigned int width, unsigned int height, bool outputToParent = false);

    unsigned int getFBO();

    unsigned int getWidth();

    unsigned int getHeight();

    bool isOutputToParent();
};
}