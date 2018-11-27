#include <mp3_devices/application/LCDapp/runLCD.hpp>

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

    QueueHandle_t qid = getSharedObject(sharedLCDQueueID);

    //Initialization objects for LCD Screen over SPI1
    PWM back_light(PWM::pwm1, 1000);
    back_light.set(50);
    GPIO sce(P2_1);
    GPIO dc(P2_3);
    GPIO rst(P2_2);
    //Constructor takes objects as params
    NOKIA5110 lcd_device(&sce, &dc, &rst, &back_light);
    lcd_device.init_display();

    int yOffset = 0;
    int xOffset = 0;
    int horizontalIncrement = 8;
    char *message;

    while(1){
        if(xQueueReceive(qid, &message, 1000)){
            lcd_device.print_string(xOffset, yOffset, message, BLACK);
            yOffset = yOffset + horizontalIncrement;
            u0_dbg_printf("Position x:%i, y:%i\n", xOffset, yOffset);
        }
    }

    return true;
}
