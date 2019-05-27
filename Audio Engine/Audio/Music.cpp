#include "Music.h"

Music::Music(std::string fname) {
    OpenFile(std::wstring(fname.begin(), fname.end()).data());
}

Music::Music(std::wstring fname) {
    OpenFile(fname.data());
}
