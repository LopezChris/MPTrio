/*
 * LabGPIOInterrupts.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: nroth
 */

#include <core_cm3.h>
#include <lpc_isr.h>
#include <LPC17xx.h>
#include "source/LabGPIOInterrupts.hpp"
#include "printf_lib.h"

LabGpioInterrupts gpio_interrupt;

void interrupt_callback(void) {

    // TODO: Disable interrupts
    gpio_interrupt.HandleInterrupt();
    // TODO: Enable interrupts
}

// Let's not make these macros
static inline void mask_set(volatile uint32_t *base, uint8_t offset) {
    *base |= 1 << offset;
}
static inline void mask_clr(volatile uint32_t *base, uint8_t offset) {
    *base &= ~(1 << offset);
}

static void dispatch_interrupts(uint32_t status, IsrPointer *vector) {
    for (uint8_t i = 0; i < 32; ++i) {
        if (status & 1) {
            vector[i]();
        }
        status >>= 1;
    }
}

void LabGpioInterrupts::HandleInterrupt() {
    dispatch_interrupts(LPC_GPIOINT->IO0IntStatR, pin_isr_map_r[0]);
    dispatch_interrupts(LPC_GPIOINT->IO0IntStatF, pin_isr_map_f[0]);
    dispatch_interrupts(LPC_GPIOINT->IO2IntStatR, pin_isr_map_r[1]);
    dispatch_interrupts(LPC_GPIOINT->IO2IntStatF, pin_isr_map_f[1]);
    // Just clear everything and hope to god a nested handler wasn't triggered
    // TODO: Figure out how to disable interrupts
    LPC_GPIOINT->IO0IntClr = 0xFFFFFFFF;
    LPC_GPIOINT->IO2IntClr = 0xFFFFFFFF;
}

bool LabGpioInterrupts::AttachInterruptHandler(uint8_t port, uint32_t pin, IsrPointer pin_isr, InterruptCondition condition) {

    if (pin > 31 || (port != 0 && port != 2)) {
        return false;
    }

    if (port == 0) {
        switch(condition) {
            case kRisingEdge:
                u0_dbg_printf("attached pin at port 0");
                mask_set(&LPC_GPIOINT->IO0IntEnR, pin);
                pin_isr_map_r[0][pin] = pin_isr;
                break;
            case kFallingEdge:
                mask_set(&LPC_GPIOINT->IO0IntEnF, pin);
                pin_isr_map_f[0][pin] = pin_isr;
                break;
            case kBothEdges:
                mask_set(&LPC_GPIOINT->IO0IntEnR, pin);
                mask_set(&LPC_GPIOINT->IO0IntEnF, pin);
                pin_isr_map_r[0][pin] = pin_isr;
                pin_isr_map_f[0][pin] = pin_isr;
                break;
        }
    } else if (port == 2) {
        switch(condition) {
            case kRisingEdge:
                mask_set(&LPC_GPIOINT->IO2IntEnR, pin);
                pin_isr_map_r[1][pin] = pin_isr;
                break;
            case kFallingEdge:
                mask_set(&LPC_GPIOINT->IO2IntEnF, pin);
                pin_isr_map_f[1][pin] = pin_isr;
                break;
            case kBothEdges:
                mask_set(&LPC_GPIOINT->IO2IntEnR, pin);
                mask_set(&LPC_GPIOINT->IO2IntEnF, pin);
                pin_isr_map_r[1][pin] = pin_isr;
                pin_isr_map_f[1][pin] = pin_isr;
                break;
        }
    }
    return true;
}


void LabGpioInterrupts::Initialize() {

    NVIC_EnableIRQ(EINT3_IRQn);
    isr_register(EINT3_IRQn, interrupt_callback);
}
