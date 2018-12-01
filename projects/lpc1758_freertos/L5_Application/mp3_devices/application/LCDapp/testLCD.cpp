#include <mp3_devices/application/LCDapp/testLCD.hpp>

postMessage::postMessage():
    scheduler_task("PassingMessage", 2048, PRIORITY_LOW){

    QueueHandle_t LCDQueue = xQueueCreate(1, sizeof(char *));

    /* Save the queue handle by using addSharedObject() */
    addSharedObject(sharedLCDQueueID, LCDQueue);
    #if 1
    u0_dbg_printf("Added Shared Object\n");
    #endif
}

bool postMessage::run(void *pvParameters){

    // This would the SD Card reader App passing a c-string to the shared queue
    // that is all the SD card needs to know.
    //char *message = "SharedOBJTest";
    const char *SDCardDataTest[3] = { "Hello.mp3", "World.mp3", "Test.mp3" };
    int count = 0;

    while(1){
        while(count < 4){
            if(xQueueSend(getSharedObject(sharedLCDQueueID), &SDCardDataTest[count], 1000) == pdTRUE){
                u0_dbg_printf("Message sent: %s\n", SDCardDataTest[count]);
                count++;
                vTaskDelay(5000);
            }
        }
    }

}
