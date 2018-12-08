
#include <stdlib.h>
#include "ff.h"

#ifndef DECODER_H
#define DECODER_H

// SCI Instructions
#define SCI_WRITE 0x02
#define SCI_READ 0x03

//SCI Registers
#define SCI_MODE 0x00
#define SCI_CLOCKF 0x03 //Clock Frequency
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06 //RAM Rd/Wr
#define SCI_WRAMADDR 0x07 //Base address for WRAM
#define SCI_VOL 0x0B //Maximum volume = 0x0000, Silence is 0xFEFE

#define SM_CANCEL (1<<3)

//SCI Mode Registers
//MODE_SM_RESET 0x0004 //Soft Reset
//MODE_SM_SDINEW 0x0800 //Native SPI modes enabled
//MODE_SM_TESTS 0x0020 //SDI tests enabled
//MODE_SM_CANCEL 0x0008 //Cancel decoding -> Cleared after set

#define PARAMETRIC_VERSION 0x0003
struct parametric {
    /* configs are not cleared between files */
    uint16_t version; /*1e02 - structure version */
    uint16_t config1; /*1e03 ---- ---- ppss RRRR PS mode, SBR mode, Reverb */
    uint16_t playSpeed; /*1e04 0,1 = normal speed, 2 = twice, 3 = three times etc. */
    uint16_t byteRate; /*1e05 average byterate */
    uint16_t endFillByte; /*1e06 byte value to send after file sent */
    uint16_t reserved[16]; /*1e07..15 file byte offsets */
    uint32_t jumpPoints[8]; /*1e16..25 file byte offsets */
    uint16_t latestJump; /*1e26 index to lastly updated jumpPoint */
    uint32_t positionMsec; /*1e27-28 play position, if known (WMA, Ogg Vorbis) */
    int16_t resync; /*1e29 > 0 for automatic m4a, ADIF, WMA resyncs */
    union {
        struct {
            uint32_t curPacketSize;
            uint32_t packetSize;
        } wma;
        struct {
            uint16_t sceFoundMask; /*1e2a SCE's found since last clear */
            uint16_t cpeFoundMask; /*1e2b CPE's found since last clear */
            uint16_t lfeFoundMask; /*1e2c LFE's found since last clear */
            uint16_t playSelect; /*1e2d 0 = first any, initialized at aac init */
            int16_t dynCompress; /*1e2e -8192=1.0, initialized at aac init */
            int16_t dynBoost; /*1e2f 8192=1.0, initialized at aac init */
            uint16_t sbrAndPsStatus; /*0x1e30 1=SBR, 2=upsample, 4=PS, 8=PS active */
        } aac;
        struct {
            uint32_t bytesLeft;
        } midi;
        struct {
            int16_t gain; /* 0x1e2a proposed gain offset in 0.5dB steps, default = -12 */
        } vorbis;
    } i;
};


class Decoder
{
    private: 
        uint8_t max_clock = 4; 
        uint8_t endFillByte = 0;

    public:

        /* *
         * Empty Constructor
         */
        Decoder(); 

        uint16_t ReadMem(uint16_t addr);
        void sciWrite(uint8_t reg_addr, uint16_t value);
        uint16_t sciRead(uint8_t reg_addr);
        void initSong();
        void stopSong(FIL *to_stream);
        void endSong();

        // Note: Implementation incompletely swaps 32-bit fields
        //void getExtras(struct parametric *in);
        uint8_t getEndFillByte();

        /* *
         * Initialize the decoder
         * xDCS set to output, xCS set to output, xRST set to output, DREQ set to input
         * Disable all pins -> xDCS, xCS, and xRST set high
         * Initialize ssp0 to max clock
         * Set Volume 
         * Enable native SPI mode 
         * Set clock frequency
         */
        void decoderInit(); 

        /* *
         * Control the volume level
         * Max volume = 0x0000, Silence 0xFEFE
         * @param {char} left - MSB for volume register
         * @param {char} right - LSB for volume register
         */
        void setVolume(char left, char right); 

        /* *
         * Hardware reset -> Set xRST low then high
         */
        void hardReset(); 

        /* *
         * Stream MP3 file
         * Send 512 bytes
         * Check DREQ every 32 bytes
         * @param {char} buffer[] - array with 512 MP3 bytes to decode
         */
        void transferData(char *buffer, size_t length);

        /* *
         * Check DREQ pin
         * @return {bool} - true = pin high, false = pin low
         */
        bool DREQ();  

        /* *
         * Stop playing and decoding music
         * Set cancel bit -> cancel bit automatically cleared
         */
        void stop(); 

        /* *
         *  Validate proper SDI transfer
         */
        void sineTest(); 

        /* *
         * Empty Destructor
         */
        ~Decoder(); 
};

#endif
