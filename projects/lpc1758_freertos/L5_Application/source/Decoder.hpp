
#include <stdlib.h>

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

//SCI Mode Registers
//MODE_SM_RESET 0x0004 //Soft Reset
//MODE_SM_SDINEW 0x0800 //Native SPI modes enabled
//MODE_SM_TESTS 0x0020 //SDI tests enabled
//MODE_SM_CANCEL 0x0008 //Cancel decoding -> Cleared after set

class Decoder
{
    private: 
        uint8_t max_clock = 4; 

    public:

        /* *
         * Empty Constructor
         */
        Decoder(); 

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
