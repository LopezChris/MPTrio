#include "tasks.hpp"

bool mp3PlayerHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    const char *filename = cmdParams.c_str();

    mp3PlayerTask mp3(3);
    mp3.initCodec();
    while (true) {
    mp3.sineTest();
    }
    //mp3.playFile(std::string(filename));
    return true;
}
