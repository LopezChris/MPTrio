#include "tasks.hpp"
#include "uart0_min.h"
#include "queue.h"
//#include "FreeRTOS.h"
#include "gpio.hpp"
#include "LabGPIOInterrupts.hpp"

QueueHandle_t task_queue = scheduler_task::getSharedObject("mp3_cmd_queue");
void create_q()
{
    if (task_queue == NULL) {
         task_queue = xQueueCreate(16, sizeof(mp3Command));
         scheduler_task::addSharedObject("mp3_cmd_queue", task_queue);
         uart0_puts("Initialized queue");
     }
}

void send_mp3_cmd(mp3Command toSend) {

        xQueueSendFromISR(task_queue, &toSend, NULL);
}

bool mp3PlayerNextHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    send_mp3_cmd(mp3Command::SKIP);
    return true;
}

bool mp3PlayerPrevHandler(str& cmdParams, CharDev& output, void* pDataParam) {


    send_mp3_cmd(mp3Command::PREV);
    return true;
}

bool mp3PlayerPauseHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    send_mp3_cmd(mp3Command::PAUSE);

    return true;
}

bool mp3PlayerResumeHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    send_mp3_cmd(mp3Command::PLAY);

    return true;
}

void prev()
{
    GPIO Prev(P1_22);
    Prev.setAsInput();

    send_mp3_cmd(mp3Command::PREV);
}

void pause()
{
    GPIO Pause(P1_23);


    Pause.setAsInput();


    send_mp3_cmd(mp3Command::PAUSE);



}

void next()
{
    GPIO Next(P1_28);
    Next.setAsInput();
}
void init()
{
   // LabGpioInterrupts gpio_interrupt;

    //QueueHandle_t task_queue = scheduler_task::getSharedObject("mp3_cmd_queue");

   // GPIO Prev(P1_22);
   /// GPIO Pause(P1_23);
   // GPIO Next(P1_28);

   // Prev.setAsInput();
   // Pause.setAsInput();
   // Next.setAsOutput();

    gpio_interrupt.Initialize();
    gpio_interrupt.AttachInterruptHandler(1,23,&pause,kFallingEdge);
    gpio_interrupt.AttachInterruptHandler(1,22,&prev,kFallingEdge);

}

bool mp3PlayerHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    const char *filename = cmdParams.c_str();

    mp3PlayerTask mp3(3);
    mp3.initCodec();
    create_q();
    init();
    //while (true) {
    //mp3.sineTest();
    //}
    mp3.playFile(std::string(filename));
    return true;
}

