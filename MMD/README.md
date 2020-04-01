# Monitor Mode Debugging in Keil µVision5 and Segger Embedded Studio

## Introduction
Monitor Mode Debugging enables you to halt and step through low priority code whilst letting high priority code execute as normal. With MMD you can debug your application while maintaining a BLE connection. 

Requirements:
* nRF5_SDK16.0
* nRF52DK (PCA10040)
* Keil µVision5 or Segger Embedded Studio V3.30
* nRF Connect (mobile or computer)


## What exactly is MMD?
Monitor Mode Debugging is essentially an interrupt service routine that contains an infinite loop. The MMD ISR has an execution priority equeal to or higher than your application, but lower than the code you want to run unimpeded. When the CPU receives a halt command from the debugger, it will execute the MMD ISR instead of halting, preempting the application code. While the CPU is busy running the MMD ISR (infinite loop), code with higher execution priority (SoftDevice, drivers, etc) can preempt the MMD ISR, but lower execution priority code (main application, libraries, etc) cannot.

When single stepping through code, a debugger will insert a breakpoint after each step. This means that MMD can also handle stepping through code of equal or lower priority without preempting higher priority calls. 


## Okay that sounds great, but i'm still not convinced that this is useful for me

I've made an example for Keil and SES based on the ble_app_blinky_s132 tutorial in SDK16.0.

See the [Keil README](pca10040/s132/arm5_no_packs/README.md) and the [SES README](pca10040/s132/ses/README.md) for instructions. 

