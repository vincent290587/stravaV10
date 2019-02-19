/*
 * hardfault.h
 *
 *  Created on: 15 févr. 2019
 *      Author: Vincent
 */

#ifndef TDD_DRIVERS_HARDFAULT_H_
#define TDD_DRIVERS_HARDFAULT_H_

/**
 * @brief Contents of the stack.
 *
 * This structure is used to re-create the stack layout after a HardFault exception was raised.
 */
typedef struct HardFault_stack
{
    uint32_t r0;  ///< R0 register.
    uint32_t r1;  ///< R1 register.
    uint32_t r2;  ///< R2 register.
    uint32_t r3;  ///< R3 register.
    uint32_t r12; ///< R12 register.
    uint32_t lr;  ///< Link register.
    uint32_t pc;  ///< Program counter.
    uint32_t psr; ///< Program status register.
} HardFault_stack_t;


#endif /* TDD_DRIVERS_HARDFAULT_H_ */
