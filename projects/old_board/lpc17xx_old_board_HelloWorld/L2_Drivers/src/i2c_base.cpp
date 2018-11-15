/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */
#include <string.h>         // memcpy
#include "i2c_base.hpp"
#include "isr_priorities.h"



void I2C_Base::handleInterrupt()
{
    long higherPriorityTaskWaiting = 0;
    mI2CStateMachineStatusType status = i2cStateMachine();

    /* For READ, do not release I2C Lock, instead give the i2cReadCompleteSignal
     * so that the read() routine can read the data first, and then release the lock.
     * For WRITE, release the i2c Lock so next task can access I2C
     */
    if(status == readComplete) {
        xSemaphoreGiveFromISR(mReadCompSig, &higherPriorityTaskWaiting);
    }
    else if(status == writeComplete) {
        xSemaphoreGiveFromISR(mI2CMutex, &higherPriorityTaskWaiting);
    }
    else {
        // I2C is busy ... do nothing
        return ;
    }

    portEND_SWITCHING_ISR(higherPriorityTaskWaiting);
}

char I2C_Base::readReg(char deviceAddress, char registerAddress)
{
    if(mDisableOperation) {
        return 0;
    }

    char byte = 0;
    readRegisters(deviceAddress, registerAddress, &byte, 1);
    return byte;
}

bool I2C_Base::readRegisters(char deviceAddress, char firstReg, char* pData, unsigned int bytesToRead)
{
    if(mDisableOperation) {
        return 0;
    }

    if(bytesToRead > sizeof(mI2CIOFrame.rwBuffer)) {
        return false;
    }

    // If scheduler not running, perform polling transaction
    if(taskSCHEDULER_RUNNING != xTaskGetSchedulerState())
    {
        NVIC_DisableIRQ(mIRQ);
        {
            i2cKickOffTransfer(i2cRead, deviceAddress, firstReg, pData, bytesToRead);
            do {
                // Wait until SI flag is set, then call i2cStateMachine()
                while(! (mpI2CRegs->I2CONSET & (1 << 3)) )
                {
                    ;
                }
            }while (readComplete != i2cStateMachine());
        }
        memcpy(pData, &mI2CIOFrame.rwBuffer[0], bytesToRead);
        NVIC_EnableIRQ(mIRQ);

        return (0 == mI2CIOFrame.error);
    }

    // Wait to get I2C Access, and take clear read complete signal if it's available
    xSemaphoreTake(mI2CMutex, portMAX_DELAY);
    xSemaphoreTake(mReadCompSig, 0);

    // Kick off the transfer and wait for read to finish
    i2cKickOffTransfer(i2cRead, deviceAddress, firstReg, pData, bytesToRead);
    xSemaphoreTake(mReadCompSig, OS_MS(I2C_READ_TIMEOUT_MS));

    // Transfer is successful if error is nonzero
    const bool success = (0 == mI2CIOFrame.error);
    if(success) {
        memcpy(pData, &mI2CIOFrame.rwBuffer[0], bytesToRead);
    }

    // After read is done, give the I2C access back.
    xSemaphoreGive(mI2CMutex);

    return success;
}

char I2C_Base::writeReg(char deviceAddress, char registerAddress, char value)
{
    if(mDisableOperation) {
        return 0;
    }

    // If scheduler not running , perform polling transaction
    if(taskSCHEDULER_RUNNING != xTaskGetSchedulerState())
    {
        NVIC_DisableIRQ(mIRQ);
        {
            i2cKickOffTransfer(i2cWrite, deviceAddress, registerAddress, &value, 1);
            do {
                // Wait until SI flag is set, then call i2cStateMachine()
                while(! (mpI2CRegs->I2CONSET & (1 << 3)) )
                {
                    ;
                }
            }while (writeComplete != i2cStateMachine());
        }
        NVIC_EnableIRQ(mIRQ);
        return true;
    }

    // Wait for I2C Access, and just return, I2C Lock will be given by ISR()
    xSemaphoreTake(mI2CMutex, portMAX_DELAY);
    i2cKickOffTransfer(i2cWrite, deviceAddress, registerAddress, &value, 1);
    return 0;
}

char I2C_Base::isDevicePresent(char deviceAddress)
{
    if(mDisableOperation) {
        return 0;
    }

    char notUsed = 0;
    unsigned int lenZeroToTestDeviceReady = 0;

    // The I2C State machine will not continue after 1st state when length is set to 0
    return readRegisters(deviceAddress, 0, &notUsed, lenZeroToTestDeviceReady);
}

I2C_Base::I2C_Base(LPC_I2C_TypeDef* pI2CBaseAddr) :
        mpI2CRegs(pI2CBaseAddr),
        mDisableOperation(false)
{
    mI2CMutex = xSemaphoreCreateMutex();
    vSemaphoreCreateBinary(mReadCompSig);

    /// Binary semaphore needs to be taken after creating it
    xSemaphoreTake(mReadCompSig, 0);

    if((unsigned int)mpI2CRegs == LPC_I2C0_BASE)
    {
        mIRQ = I2C0_IRQn;
    }
    else if((unsigned int)mpI2CRegs == LPC_I2C1_BASE)
    {
        mIRQ = I2C1_IRQn;
    }
    else if((unsigned int)mpI2CRegs == LPC_I2C2_BASE)
    {
        mIRQ = I2C2_IRQn;
    }
    else {
        mIRQ = (IRQn_Type)99; // Using invalid IRQ on purpose
    }
}

bool I2C_Base::init(unsigned int pclk, unsigned int busRateInKhz)
{
    // Power on I2C
    switch(mIRQ) {
        case I2C0_IRQn: LPC_SC->PCONP |= (1 <<  7);  break;
        case I2C1_IRQn: LPC_SC->PCONP |= (1 << 19);  break;
        case I2C2_IRQn: LPC_SC->PCONP |= (1 << 26);  break;
        default: return false;
    }

    mpI2CRegs->I2CONCLR = 0x6C;           // Clear ALL I2C Flags

    // Set the I2C Dividers to attain the frequency
    const unsigned int i2cFrequency = busRateInKhz > 1000 ? 100*1000 : busRateInKhz * 1000;
    mpI2CRegs->I2SCLH = (pclk/(i2cFrequency + 1)) / 2;
    mpI2CRegs->I2SCLL = (pclk / i2cFrequency) / 2;

    // Set I2C slave address and enable I2C
    mpI2CRegs->I2ADR0 = 0;
    mpI2CRegs->I2ADR1 = 0;
    mpI2CRegs->I2ADR2 = 0;
    mpI2CRegs->I2ADR3 = 0;

    // Finally, enable I2C and enable Interrupts for this I2C
    mpI2CRegs->I2CONSET = 0x40;
    NVIC_EnableIRQ(mIRQ);

    return true;
}



/// Private ///

void I2C_Base::i2cKickOffTransfer(mI2cModeType mode, char devAddr, char regStart, char* pBytes, int len)
{
    if(len <= I2C_FRAME_MAX_DATABYTES)
    {
        mI2CIOFrame.error           = 0;
        mI2CIOFrame.mode            = mode;
        mI2CIOFrame.slaveAddress    = devAddr;
        mI2CIOFrame.firstRegister   = regStart;
        mI2CIOFrame.bytesToTransfer = len;
        mI2CIOFrame.bytePointer     = 0;

        // Copy data to write into I2C Buffer.
        if(i2cWrite == mode) {
            memcpy(mI2CIOFrame.rwBuffer, pBytes, len);
        }

        // Send START, I2C State Machine will finish the rest.
        mpI2CRegs->I2CONSET = 0x20;
    }
}

/*
 * I2CONSET
 * 0x04 AA
 * 0x08 SI
 * 0x10 STOP
 * 0x20 START
 * 0x40 ENABLE
 *
 * I2CONCLR
 * 0x04 AA
 * 0x08 SI
 * 0x20 START
 * 0x40 ENABLE
 */
I2C_Base::mI2CStateMachineStatusType I2C_Base::i2cStateMachine()
{
    enum I2CStatus{ busError=0, start=0x08, repeatStart=0x10, arbitrationLost=0x38,
            // Master Transmitter States:
            slaveAddressAcked=0x18, slaveAddressNacked=0x20, dataAckedBySlave=0x28, dataNackedBySlave=0x30,
            // Master Receiver States:
            readAckedBySlave=0x40, readModeNackedBySlave=0x48, dataAvailableAckSent=0x50, dataAvailableNackSent=0x58
    };

    mI2CStateMachineStatusType state = busy;

    /*
     ***********************************************************************************************************
     * Write-mode state transition :
     * start --> slaveAddressAcked --> dataAckedBySlave --> ... (dataAckedBySlave) --> (stop)
     *
     * Read-mode state transition :
     * start --> slaveAddressAcked --> dataAcked --> repeatStart --> readAckedBySlave
     *  For 2+ bytes:  dataAvailableAckSent --> ... (dataAvailableAckSent) --> dataAvailableNackSent --> (stop)
     *  For 1  byte :  dataAvailableNackSent --> (stop)
     ***********************************************************************************************************
     */

    #define clearSIFlag()       mpI2CRegs->I2CONCLR = (1<<3)
    #define setSTARTFlag()      mpI2CRegs->I2CONSET = (1<<5)
    #define clearSTARTFlag()    mpI2CRegs->I2CONCLR = (1<<5)

    // busInUse is only set to 0 for write operation since read operation should set to 0 itself
    #define setStop()           clearSTARTFlag();                       \
                                mpI2CRegs->I2CONSET = (1<<4);           \
                                clearSIFlag();                          \
                                while((mpI2CRegs->I2CONSET&(1<<4)));    \
                                if(i2cRead == mI2CIOFrame.mode)         \
                                    state = readComplete;               \
                                else                                    \
                                    state = writeComplete;

    switch (mpI2CRegs->I2STAT)
    {
        case start:
            mpI2CRegs->I2DAT = mI2CIOFrame.slaveAddress;
            clearSIFlag();
            break;
        case repeatStart:
            mpI2CRegs->I2DAT = mI2CIOFrame.slaveAddress | 0x01;
            clearSIFlag();
            break;

        case slaveAddressAcked:
            clearSTARTFlag();
            if(0 == mI2CIOFrame.bytesToTransfer) {
                setStop();
            }
            else {
                mpI2CRegs->I2DAT = mI2CIOFrame.firstRegister;
                clearSIFlag();
            }
            break;
        case slaveAddressNacked:
            mI2CIOFrame.error = mpI2CRegs->I2STAT;
            setStop();
            break;

        case dataAckedBySlave:
            if (i2cRead == mI2CIOFrame.mode) {
                setSTARTFlag();         // Send Repeat-start for read-mode
                clearSIFlag();
            }
            else {
                if(mI2CIOFrame.bytePointer >= mI2CIOFrame.bytesToTransfer) {
                    setStop();
                }
                else {
                    mpI2CRegs->I2DAT = mI2CIOFrame.rwBuffer[mI2CIOFrame.bytePointer++];
                    clearSIFlag();
                }
            }
            break;
        case dataNackedBySlave:
            mI2CIOFrame.error = mpI2CRegs->I2STAT;
            setStop();
            break;


        case readAckedBySlave:
            clearSTARTFlag();
            if(mI2CIOFrame.bytesToTransfer > 1)
                mpI2CRegs->I2CONSET = 0x04;  // Send ACK to receive a byte and transition to dataAvailableAckSent
            else
                mpI2CRegs->I2CONCLR = 0x04;  // NACK next byte to go to dataAvailableNackSent for 1-byte read.
            clearSIFlag();
            break;
        case readModeNackedBySlave:
            mI2CIOFrame.error = mpI2CRegs->I2STAT;
            setStop();
            break;
        case dataAvailableAckSent:
            mI2CIOFrame.rwBuffer[mI2CIOFrame.bytePointer++] = mpI2CRegs->I2DAT;
            if(mI2CIOFrame.bytePointer >= (mI2CIOFrame.bytesToTransfer-1)) {    // Only 1 more byte remaining
                mpI2CRegs->I2CONCLR = 0x04; // NACK next byte --> Next state: dataAvailableNackSent
            }
            else {
                mpI2CRegs->I2CONSET = 0x04; // ACK next byte --> Next state: dataAvailableAckSent(back to this state)
            }
            clearSIFlag();
            break;
        case dataAvailableNackSent: // Read last-byte from Slave
            mI2CIOFrame.rwBuffer[mI2CIOFrame.bytePointer++] = mpI2CRegs->I2DAT;
            setStop();
            break;


        case busError:
            mI2CIOFrame.error = mpI2CRegs->I2STAT;
            setStop();
            break;
        case arbitrationLost:
            // We should not issue stop() in this condition, but we still need to end our
            // i2c transaction.
            state = mI2CIOFrame.mode == i2cRead ? readComplete : writeComplete;
            mI2CIOFrame.error = mpI2CRegs->I2STAT;
            break;
        default:
            mI2CIOFrame.error = mpI2CRegs->I2STAT;
            setStop();
            break;
    }

    return state;
}



