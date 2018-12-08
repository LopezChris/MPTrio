#include <mp3_devices/application/LCDapp/runLCD.hpp>
#include <string.h>
#include <stdio.h>

receiveMessage::receiveMessage() :
    scheduler_task("LCDMessagePrint", 2048, PRIORITY_LOW)
{

}

/**
 * Simple function to initialize 
 * the LCD object and wait to hear from queue.
 * 
 * TODO: Write separate function for 'run' task. this will do for now until we are able to read SD
 */


bool receiveMessage::run(void *pvParameters){

    QueueHandle_t qid = NULL;
    SemaphoreHandle_t bus_lock = NULL;


    uart0_puts("Acquiring queue...");
    while ((qid = scheduler_task::getSharedObject("lcd_str_queue")) == NULL) {
        vTaskDelay(100);

        uart0_puts("Still acquiring queue...");
    }
    uart0_puts("Queue acquired");


    uart0_puts("Aquiring Semaphore...");
    while ((bus_lock = scheduler_task::getSharedObject("spi_bus_lock")) == NULL) {
        vTaskDelay(100);

        uart0_puts("Still acquiring semaphore...");
    }
    uart0_puts("Semaphore aquired\n");

    //Initialization objects for LCD Screen over SPI1
    PWM back_light(PWM::pwm1, 1000);
    back_light.set(50);
    GPIO sce(P2_1);
    GPIO dc(P2_3);
    GPIO rst(P2_2);
    NOKIA5110 lcd_device(&sce, &dc, &rst, &back_light);

    if(xSemaphoreTake(bus_lock, 1000) == pdTRUE){
        //Constructor takes objects as params

        lcd_device.init_display();
        xSemaphoreGive(bus_lock);
        uart0_puts("Screen initialized\n");
    }


    int yOffset = 0;
    int xOffset = 0;
    int horizontalIncrement = 8;
    char *message;
    while(1){
        if(xSemaphoreTake(bus_lock, 1000) == pdTRUE){
            if(xQueueReceive(qid, &message, 1000)){

                uart0_puts(message);
                lcd_device.print_string(xOffset, yOffset, message, BLACK);
                yOffset = yOffset + horizontalIncrement;
                u0_dbg_printf("Position x:%i, y:%i\n", xOffset, yOffset);
            }
            xSemaphoreGive(bus_lock);
        }
    }

    return true;
}
