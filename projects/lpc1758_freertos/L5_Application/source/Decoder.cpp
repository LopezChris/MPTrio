#include "ssp0.h"
#include "gpio.hpp"
#include "Decoder.hpp"
#include "utilities.h"
#include <stdio.h>
#include <ff.h>
#include <string.h>

#define min(a,b) (((a)<(b))?(a):(b))

GPIO xDCSPin(P0_0); //Active low, deactivate and reactivate every 32 bytes
GPIO DREQPin(P2_6); //Input to SJONE, Data request pin used to check every 32 bytes if FIFO is full (low)
GPIO xCSPin(P0_1); //Active low, Data is synchronized with rising edge of xCS
GPIO xRSTPin(P1_20); //Reset pin for MP3 decoder, Active Low

Decoder::Decoder()
{
    //Do Nothing
    playerState = psPlayback;
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
uint16_t Decoder::ReadMem(uint16_t addr)
{
    sciWrite(0x07, addr); //SCI_WRAMADDR
    return sciRead(0x06); //SCI_WRAM
}

void Decoder::sciWrite(uint8_t reg_addr, uint16_t value)
{
  while (!DREQ());
  xCSPin.setLow();
  //Write
  ssp0_exchange_byte(0x02);     //write opcode
  ssp0_exchange_byte(reg_addr);  //write address
  ssp0_exchange_byte(value >> 8);  //shift right 8 to send next 8
  ssp0_exchange_byte(value & 0xff);  //wr

  xCSPin.setHigh();
}

uint16_t Decoder::sciRead(uint8_t reg_addr)
{
  uint16_t data;
  while (!DREQ());
  xCSPin.setLow();                     //Update CS
  //Read
  ssp0_exchange_byte(0x03);         //read opcode
  ssp0_exchange_byte(reg_addr);
  delay_ms(10);
  data = ssp0_exchange_byte(0x00);
  data <<= 8;
  data |= ssp0_exchange_byte(0x00);

  xCSPin.setHigh();                    //Update CS
  return data;
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

void Decoder::playAndDecode(FILE *readfp)
{
    static uint8_t playBUF[512];
    uint32_t bytes_in_buffer; //bytes in buffer
    int End_fillbyte = 0; //what byte value to send after file end
    int End_fillbytes = 2050;
    int playmode_pause = 1;
    int playMode = ReadMem(0x1e09);

    playerState = psPlayback;

    sciWrite(0x04, 0); //reset decode time

    while ((bytes_in_buffer = fread(playBUF, 1, 512, readfp)) > 0 && playerState != psStopped)
    {
        uint8_t *buffP = playBUF;

        while (bytes_in_buffer && playerState != psStopped)
        {
            if (!(playMode & playmode_pause))
            {
                int t = min(32, bytes_in_buffer); //32 max transfer size

                //audio going to decoder
                transferData(buffP, t);
                buffP += t;
                bytes_in_buffer -= t;
            }

            //user request cancel, set SM_CANCEL
            if(playerState == psUserRequestedCancel)
            {
                unsigned short old_mode;
                playerState = psUserCancelSent;
                old_mode =sciRead(0x00); //sci mode register
                sciWrite(0x00, old_mode | 0x03); // 3 SM_CANCEL
            }

            //sm_cancel set, stop playback
            if(playerState == psUserCancelSent)
            {
                unsigned short mode = sciRead(0x00);
                if(!(mode & 0x03))
                {
                    playerState = psStopped;
                }
            }

            //if file is broken or cancel playback write end_fillbytes
            memset(playBUF, End_fillbyte, sizeof(playBUF));
            for(int i = 0; i < End_fillbytes; i+=32)
            {
                transferData(playBUF, 32);
            }

            //when file actually ends
            if(playerState == psPlayback)
            {
                unsigned short old_mode = sciRead(0x00);
                (0x00, old_mode | 0x03);
                printf("Setting SM_Cancel...");

                while(sciRead(0x00) & 0x03)
                    transferData(playBUF, 2);
            }
        }


    }





}

void Decoder::transferData(uint8_t *buffer, uint32_t length)
{

    while (!DREQ());
    xDCSPin.setLow();
    for (int i = 0; i < length; i++)
    {
      while (!DREQ());
      ssp0_exchange_byte(data[i]);
    }
    if (length < 32)
      for (int i = 0; i < (32 - length); i++)
        ssp0_exchange_byte(0x00);
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
