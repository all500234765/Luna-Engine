#pragma once

#include "pc.h"
#include "Sound.h"

class SoundEffect: public Sound {
private:

public:
    SoundEffect(std::string fname);
    SoundEffect(std::wstring fname);

};
