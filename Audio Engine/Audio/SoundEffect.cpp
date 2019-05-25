#include "SoundEffect.h"

SoundEffect::SoundEffect(std::string fname) {
    OpenFile(std::wstring(fname.begin(), fname.end()).data());
}

SoundEffect::SoundEffect(std::wstring fname) {
    OpenFile(fname.data());
}
