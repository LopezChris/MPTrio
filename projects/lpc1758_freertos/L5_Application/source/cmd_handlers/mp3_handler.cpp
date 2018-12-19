#include "tasks.hpp"
#include "uart0_min.h"


void mp3PlayerNextHandlerISR();

bool mp3PlayerHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    const char *filename = cmdParams.c_str();

    mp3PlayerTask mp3(3);
    mp3.initCodec();

    uart0_puts("mp3Handler function");

    mp3.playFile(std::string(filename));
    return true;
}

//void send_mp3_cmd(mp3Command toSend) {
//    QueueHandle_t task_queue = scheduler_task::getSharedObject("mp3_cmd_queue");
//    if (task_queue == NULL) {
//        task_queue = xQueueCreate(16, sizeof(mp3Command));
//        scheduler_task::addSharedObject("mp3_cmd_queue", task_queue);
//        uart0_puts("Initialized queue");
//    }
//
//    xQueueSendFromISR(task_queue, &toSend, 0);
//}
//
//void mp3PlayerNextHandlerISR(){
//    u0_dbg_printf("TEST\n");
//    send_mp3_cmd(mp3Command::SKIP);
//}
//
//bool mp3PlayerNextHandler(str& cmdParams, CharDev& output, void* pDataParam) {
//
//    send_mp3_cmd(mp3Command::SKIP);
//    return true;
//}
//
//bool mp3PlayerPrevHandler(str& cmdParams, CharDev& output, void* pDataParam) {
//
//
//    send_mp3_cmd(mp3Command::PREV);
//    return true;
//}
//
//bool mp3PlayerPauseHandler(str& cmdParams, CharDev& output, void* pDataParam) {
//
//    send_mp3_cmd(mp3Command::PAUSE);
//
//    return true;
//}
//
//bool mp3PlayerResumeHandler(str& cmdParams, CharDev& output, void* pDataParam) {
//
//    send_mp3_cmd(mp3Command::PLAY);
//
//    return true;
//}
