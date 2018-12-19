#include "tasks.hpp"
#include "examples/examples.hpp"
#include "mp3_devices/application/LCDapp/runLCD.hpp"
#include "mp3_devices/application/LCDapp/testLCD.hpp"
#include "source/LabGPIOInterrupts.hpp"
#include <L4_IO/gpio.hpp>

mp3PlayerTask *mp3_player = nullptr;



int main(void){

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));


    #if 1

        GPIO extSW(P1_22);
        extSW.setAsInput();
        gpio_interrupt.Initialize();


        mp3_player = new mp3PlayerTask(PRIORITY_CRITICAL);
        scheduler_add_task(mp3_player);

        // Waits for string to be posted to shared queue
        scheduler_add_task(new receiveMessage());



    #endif

    scheduler_start(); ///< This shouldn't return
    return -1;
}
