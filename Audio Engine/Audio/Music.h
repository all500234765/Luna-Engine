#pragma once

#include "pc.h"
#include "AudioDeviceChild.h"
#include "Sound.h"

class Music: public Sound {
private:

public:
    Music(std::string fname);
    Music(std::wstring fname);

};
