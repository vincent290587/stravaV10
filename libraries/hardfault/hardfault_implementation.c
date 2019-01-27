/*
 * hardfault_implementation.c
 *
 *  Created on: 23 déc. 2018
 *      Author: Vincent
 */

#include "sdk_common.h"
#if !NRF_MODULE_ENABLED(HARDFAULT_HANDLER)
#error "Please enable the hard fault handler"
#endif
#include "hardfault.h"
#include "nrf.h"
#include "compiler_abstraction.h"
#include "app_util_platform.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_soc.h"
#endif

#include "segger_wrapper.h"

extern void HardFault_process(HardFault_stack_t * p_stack);


void HardFault_c_handler(uint32_t * p_stack_address)
{

#if (__CORTEX_M == 0x04)

#ifndef CFSR_MMARVALID
  #define CFSR_MMARVALID (1 << (0 + 7))
#endif

#ifndef CFSR_BFARVALID
  #define CFSR_BFARVALID (1 << (8 + 7))
#endif

    HardFault_stack_t * p_stack = (HardFault_stack_t *)p_stack_address;
    static const char *cfsr_msgs[] = {
        [0]  = "The processor has attempted to execute an undefined instruction",
        [1]  = "The processor attempted a load or store at a location that does not permit the operation",
        [2]  = NULL,
        [3]  = "Unstack for an exception return has caused one or more access violations",
        [4]  = "Stacking for an exception entry has caused one or more access violations",
        [5]  = "A MemManage fault occurred during floating-point lazy state preservation",
        [6]  = NULL,
        [7]  = NULL,
        [8]  = "Instruction bus error",
        [9]  = "Data bus error (PC value stacked for the exception return points to the instruction that caused the fault)",
        [10] = "Data bus error (return address in the stack frame is not related to the instruction that caused the error)",
        [11] = "Unstack for an exception return has caused one or more BusFaults",
        [12] = "Stacking for an exception entry has caused one or more BusFaults",
        [13] = "A bus fault occurred during floating-point lazy state preservation",
        [14] = NULL,
        [15] = NULL,
        [16] = "The processor has attempted to execute an undefined instruction",
        [17] = "The processor has attempted to execute an instruction that makes illegal use of the EPSR",
        [18] = "The processor has attempted an illegal load of EXC_RETURN to the PC, as a result of an invalid context, or an invalid EXC_RETURN value",
        [19] = "The processor has attempted to access a coprocessor",
        [20] = NULL,
        [21] = NULL,
        [22] = NULL,
        [23] = NULL,
        [24] = "The processor has made an unaligned memory access",
        [25] = "The processor has executed an SDIV or UDIV instruction with a divisor of 0",
    };

    uint32_t cfsr = SCB->CFSR;

#if defined(DEBUG)
    if (p_stack != NULL)
    {
        // Print information about error.
        LOG_ERROR("HARD FAULT at 0x%08X", p_stack->pc);
        LOG_ERROR("  R0:  0x%08X  R1:  0x%08X  R2:  0x%08X  R3:  0x%08X",
                             p_stack->r0, p_stack->r1, p_stack->r2, p_stack->r3);
        LOG_ERROR("  R12: 0x%08X  LR:  0x%08X  PSR: 0x%08X",
                             p_stack->r12, p_stack->lr, p_stack->psr);
    }
    else
    {
        LOG_ERROR("Stack violation: stack pointer outside stack area.");
    }

    if (SCB->HFSR & SCB_HFSR_VECTTBL_Msk)
    {
        LOG_ERROR("Cause: BusFault on a vector table read during exception processing.");
    }

    for (uint32_t i = 0; i < sizeof(cfsr_msgs) / sizeof(cfsr_msgs[0]); i++)
    {
        if (((cfsr & (1 << i)) != 0) && (cfsr_msgs[i] != NULL))
        {
            LOG_ERROR("Cause: %s.", cfsr_msgs[i]);
        }
    }

    if (cfsr & CFSR_MMARVALID)
    {
        LOG_ERROR("MemManage Fault Address: 0x%08X", SCB->MMFAR);
    }

    if (cfsr & CFSR_BFARVALID)
    {
        LOG_ERROR("Bus Fault Address: 0x%08X", SCB->BFAR);
    }



    NRF_BREAKPOINT_COND;

 #endif // defined (DEBUG)

#endif // __CORTEX_M == 0x04

    HardFault_process((HardFault_stack_t *)p_stack_address);
}



