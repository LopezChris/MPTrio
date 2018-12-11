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
    SemaphoreHandle_t spi_bus_lock = NULL;


    uart0_puts("Acquiring queue LCD TASK...");
    while ((qid = scheduler_task::getSharedObject("lcd_str_queue")) == NULL) {
        vTaskDelay(10000);

        uart0_puts("Still acquiring queue LCD TASK...");
    }
    uart0_puts("Queue acquired");


    while ((spi_bus_lock = scheduler_task::getSharedObject("spi_bus_lock")) == NULL) {
        vTaskDelay(100);

        uart0_puts("Still acquiring SPI lock MP3 TASK...\n");
    }
    uart0_puts("runLCD acquired SPI lock\n");

    //Initialization objects for LCD Screen over SPI1
    PWM back_light(PWM::pwm1, 1000);
    back_light.set(50);
    GPIO sce(P2_1);
    GPIO dc(P2_3);
    GPIO rst(P2_2);
    //Constructor takes objects as params
    NOKIA5110 lcd_device(&sce, &dc, &rst, &back_light);

    if(xSemaphoreTake(spi_bus_lock, 1000) == pdTRUE){
        uart0_puts("runLCD task took spi_bus_lock Sem\n");
        lcd_device.init_display();
    }
    xSemaphoreGive(spi_bus_lock);


    int yOffset = 0;
    int xOffset = 0;
    int verticalIncrement = 8;
    char *message;

    while(1){

        if(xQueueReceive(qid, &message, 1000)){

            // Christian, something in here is crashing!!!
            // TODO: look into why this is crashing
            if(xSemaphoreTake(spi_bus_lock, 1000) == pdFALSE){
                u0_dbg_printf("Failed to take spi_bus runLCD line 70");
            }
            u0_dbg_printf("Aquired SPI Bus lock LCD task\n");
            u0_dbg_printf("Position x:%i, y:%i\n", xOffset, yOffset);
            lcd_device.print_string(xOffset, yOffset, message, BLACK);
            xSemaphoreGive(spi_bus_lock);

            yOffset = yOffset + verticalIncrement;




            free(message);
        }

    }

    return true;
}
