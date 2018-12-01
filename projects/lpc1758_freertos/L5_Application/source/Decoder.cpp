#include "ssp0.h"
#include "gpio.hpp"
#include "Decoder.hpp"
#include "utilities.h"
#include <stdio.h>

GPIO xDCSPin(P0_0); //Active low, deactivate and reactivate every 32 bytes
GPIO DREQPin(P2_6); //Input to SJONE, Data request pin used to check every 32 bytes if FIFO is full (low)
GPIO xCSPin(P0_1); //Active low, Data is synchronized with rising edge of xCS
GPIO xRSTPin(P1_20); //Reset pin for MP3 decoder, Active Low

Decoder::Decoder()
{
    //Do Nothing
}

void Decoder::decoderInit()
{
    //Initialize GPIO pins
    xDCSPin.setAsOutput(); 
    DREQPin.setAsInput(); 
    xCSPin.setAsOutput(); 
    xRSTPin.setAsOutput(); 

    //Initialize all pins to disabled
    xDCSPin.setHigh(); 
    xCSPin.setHigh(); 
    hardReset(); 

    //Initialize ssp0 
    ssp0_init(max_clock); 
    ssp0_set_max_clock(max_clock); 
    delay_ms(10); 

    //Set volume 
    setVolume(0x32, 0x32); 
    delay_ms(10);

    //Enable native SPI mode
    xCSPin.setLow(); 
    ssp0_exchange_byte(SCI_WRITE); 
    ssp0_exchange_byte(0x00); 
    ssp0_exchange_byte(0x08); 
    ssp0_exchange_byte(0x00);
    xCSPin.setHigh(); 

    //Set clock frequency
    xCSPin.setLow(); 
    ssp0_exchange_byte(SCI_WRITE); 
    ssp0_exchange_byte(0x03); 
    ssp0_exchange_byte(0xA0); 
    ssp0_exchange_byte(0x00);
    xCSPin.setHigh(); 
} 

void Decoder::setVolume(char left, char right)
{
    //Set volume
    xCSPin.setLow();
    ssp0_exchange_byte(SCI_WRITE); 
    ssp0_exchange_byte(0x0B); //SCI_VOL register
    ssp0_exchange_byte(left); //left
    ssp0_exchange_byte(right); //right
    xCSPin.setHigh(); 
} 

void Decoder::hardReset()
{
    xRSTPin.setLow(); 
    delay_ms(5);
    xRSTPin.setHigh(); 
    delay_ms(5);
} 

void Decoder::transferData(char *buffer, size_t length)
{
    for(int i = 0; i < length; i++)
    {
        xCSPin.setHigh(); 
        xDCSPin.setLow();

        ssp0_exchange_byte(buffer[i]); 

        if(((i + 1) % 32) == 0)
        {
            while(!DREQ());
        } 
    }
    xCSPin.setHigh(); 
    xDCSPin.setHigh(); 
}

bool Decoder::DREQ()
{
    return DREQPin.read(); 
}

void Decoder::stop()
{
    //Cancel Decoding 
    xDCSPin.setHigh(); 
    xCSPin.setLow(); 
    ssp0_exchange_byte(SCI_WRITE); 
    ssp0_exchange_byte(0x00); 
    ssp0_exchange_byte(0x08); 
    ssp0_exchange_byte(0x08); 
    xCSPin.setHigh(); 
} 

void Decoder::sineTest()
{
    //Set native SPI mode (0x08), Enable testing(0x20), Soft reset(0x04)
    xCSPin.setLow(); 
    ssp0_exchange_byte(SCI_WRITE); 
    ssp0_exchange_byte(0x00); 
    ssp0_exchange_byte(0x08); 
    ssp0_exchange_byte(0x20); 
    xCSPin.setHigh();

    printf("Waiting on DREQ\n");
    while(!DREQPin.read()); //Wait for DREQ to be 1, if 0 FIFO is full 

    //Start Sine Test
    printf("sending sine test begin\n");
    xDCSPin.setLow(); 
    ssp0_exchange_byte(0x53); 
    ssp0_exchange_byte(0xEF);
    ssp0_exchange_byte(0x6E);
    ssp0_exchange_byte(0x24); 
    ssp0_exchange_byte(0x00); 
    ssp0_exchange_byte(0x00); 
    ssp0_exchange_byte(0x00); 
    ssp0_exchange_byte(0x00); 
    xDCSPin.setHigh(); 

    delay_ms(5000); 

    //End Sine Test
    xDCSPin.setLow(); 
    ssp0_exchange_byte(0x45);
    ssp0_exchange_byte(0x78);
    ssp0_exchange_byte(0x69);
    ssp0_exchange_byte(0x74); 
    ssp0_exchange_byte(0x00); 
    ssp0_exchange_byte(0x00); 
    ssp0_exchange_byte(0x00);
    ssp0_exchange_byte(0x00); 
    xDCSPin.setHigh(); 

    delay_ms(500); 
} 

Decoder::~Decoder()
{
    //Do Nothing
}
