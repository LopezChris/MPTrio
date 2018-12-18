#include "tasks.hpp"
#include "uart0_min.h"
#include "queue.h"
//#include "FreeRTOS.h"
#include "gpio.hpp"
#include <source/cmd_handlers/LabGPIOInterrupts.hpp>
#include <source/cmd_handlers/button.hpp>

/*
 * P1_20,
 * P1_22, prev
 * P1_23, play
 * P1_28, next
 */

QueueHandle_t task_queue = scheduler_task::getSharedObject("mp3_cmd_queue");

buttons::buttons()
{
    //do nothing
}

void buttons::create_q()
{
    if (task_queue == NULL) {
         task_queue = xQueueCreate(16, sizeof(mp3Command));
         scheduler_task::addSharedObject("mp3_cmd_queue", task_queue);
         uart0_puts("Initialized queue");
     }
}

void buttons::send_mp3_cmd(mp3Command toSend) {

        xQueueSendFromISR(task_queue, &toSend, NULL);
}

bool buttons::mp3PlayerNextHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    send_mp3_cmd(mp3Command::SKIP);
    return true;
}

bool buttons::mp3PlayerPrevHandler(str& cmdParams, CharDev& output, void* pDataParam) {


    send_mp3_cmd(mp3Command::PREV);
    return true;
}

bool buttons::mp3PlayerPauseHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    send_mp3_cmd(mp3Command::PAUSE);

    return true;
}

bool buttons::mp3PlayerResumeHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    send_mp3_cmd(mp3Command::PLAY);

    return true;
}

void buttons::prev_isr()
{

    send_mp3_cmd(mp3Command::PREV);
}

void buttons::pause_isr()
{

    send_mp3_cmd(mp3Command::PAUSE);

}

void buttons::next_isr()
{
    GPIO Next(P1_28);
    Next.setAsInput();
}
void buttons::init()
{
   // LabGpioInterrupts gpio_i;

    //QueueHandle_t task_queue = scheduler_task::getSharedObject("mp3_cmd_queue");

   // GPIO Prev(P1_22);
   /// GPIO Pause(P1_23);
   // GPIO Next(P1_28);

   // Prev.setAsInput();
   // Pause.setAsInput();
   // Next.setAsOutput();

    GPIO Pause(P1_23);
    Pause.setAsInput();

    gpio_interrupt.Initialize();
    gpio_interrupt.AttachInterruptHandler(1,23,&pause_isr,kFallingEdge);

   // gpio_interrupt.AttachInterruptHandler(1,22,&prev_isr,kFallingEdge);

}

buttons::~buttons()
{
 //deconstructor
}

/*
bool mp3PlayerHandler(str& cmdParams, CharDev& output, void* pDataParam) {

    const char *filename = cmdParams.c_str();

    mp3PlayerTask mp3(3);
    mp3.initCodec();
    create_q();
    init();

    gpio_interrupt.AttachInterruptHandler(1,23,&pause_isr,kRisingEdge);
    gpio_interrupt.AttachInterruptHandler(1,22,&prev_isr,kFallingEdge);
    //while (true) {
    //mp3.sineTest();
    //}
    mp3.playFile(std::string(filename));
    return true;
}
*/
