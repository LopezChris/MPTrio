#include "ssp0.h"
#include "gpio.hpp"
#include "Decoder.hpp"
#include "utilities.h"
#include <stdio.h>
#include "uart0_min.h"
#include "ff.h"

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

    endFillByte = getEndFillByte();

    //endSong();
    sineTest();
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

#define SCI_WRAMADDR 7
#define SCI_WRAM 6
/*
void Decoder::getExtras(struct parametric *in) {
    size_t to_read = sizeof(struct parametric);
    xCSPin.setLow(); // TODO: Semaphore
    size_t start = 0x1e02;
    uint16_t *read_buf = (uint16_t *)in;
    for (size_t pos = start, i=0; pos - start < sizeof(struct parametric); pos+=2, ++i) {
        sciWrite(SCI_WRAMADDR, pos);
        read_buf[i] = sciRead(SCI_WRAM);
    }
    xCSPin.setHigh(); // TODO: Semaphore
}*/

uint8_t Decoder::getEndFillByte() {
    sciWrite(SCI_WRAMADDR, 0x1e02 + 8);
    uint8_t endFillByte = (uint8_t)sciRead(SCI_WRAM);
    return endFillByte;
}

void Decoder::initSong() {
    // TODO: Do any init work needed
}

void Decoder::stopSong(FIL *to_stream) {
    uint16_t mode = sciRead(SCI_MODE);
    mode |= SM_CANCEL;
    sciWrite(SCI_MODE, mode);

    while ((sciRead(SCI_MODE) & SM_CANCEL) != 0) {
        // Stream remaining bytes from file, 32 at a time, as requested by docs
        char mp3_buffer[32];
        UINT bytes_read = 0;
        bool has_read = false;
        FRESULT read_rslt;
        if ((FR_OK == (read_rslt = f_read(to_stream, mp3_buffer, 32, &bytes_read))) &&
                bytes_read == 32) {
            has_read = true;
            // Woo! Everything was read into mp3_buffer
            transferData(mp3_buffer, 32);
        }

        if (read_rslt != FR_OK) {
            // TODO: Do we need to do some
            // Uh-oh, something went bad with the read
            // UI TODO: Display error message (maybe use the code from in read_rslt "READ ERR: <CODE>")
            uart0_puts("Error reading from file: error code returned from f_read");

            // In the case of a read failure, we must break as there is no way to get the rest
            break;
        } else {
            // bytes read != file size -- looks like the end!
            transferData(mp3_buffer, bytes_read);

            // We can't send what we don't have -- we must break now that we're at the end
            break;
        }
        // TODO: If we've done this 64 times and we're
        // still here, we should send a software reset
        // (docs say extremely rare, so let's ignore)
    }

    endFillByte = getEndFillByte();
    for (int i = 0; i < 2052; ++i) {
        ssp0_exchange_byte(endFillByte);
    }
}

void Decoder::endSong() {

    endFillByte = getEndFillByte();
    for (int i = 0; i < 2052; ++i) {
        ssp0_exchange_byte(endFillByte);
    }

    uint16_t mode = sciRead(SCI_MODE);
    mode |= SM_CANCEL;
    sciWrite(SCI_MODE, mode);

    uart0_puts("Foo");

    while ((sciRead(SCI_MODE) & SM_CANCEL) != 0) {
        for (int i = 0; i < 32; ++i) {
            ssp0_exchange_byte(endFillByte);
        }
        // TODO: If we've done this 64 times and we're
        // still here, we should send a software reset
        // (docs say extremely rare, so let's ignore)
    }
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
    xCSPin.setHigh();
    xDCSPin.setLow();
    for(int i = 0; i < length; i++)
    {
        ssp0_exchange_byte(buffer[i]); 

        if(((i + 1) % 32) == 0)
        {
            while(!DREQ());

            xDCSPin.setHigh();
            delay_ms(1);
            xDCSPin.setLow();
        } 
    }
    //xCSPin.setHigh();
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
