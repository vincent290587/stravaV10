
#ifndef __ANT_C__
#define __ANT_C__

#define APP_TIMER_PRESCALER         0                                   /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE     10                                   /**< Size of timer operation queues. */



#define WILDCARD_TRANSMISSION_TYPE      0x00
#define ANTPLUS_NETWORK_NUMBER          0x00                                           /**< Network number. */

#define HRM_CHANNEL_NUMBER              0x00
#define HRM_DEVICE_NUMBER               0x0D22    /**< Device Number. */

#define BSC_CHANNEL_NUMBER              0x01
#define BSC_DEVICE_NUMBER               0xB02B    /**< Device Number. */
#define BSC_DEVICE_TYPE                 0x79

#define GLASSES_CHANNEL_NUMBER          0x02                                            /**< Default ANT Channel. */



#define ANTPLUS_NETWORK_NUMBER          0x00                                           /**< Network number. */

#define FEC_CHANNEL_NUMBER              0x03

#define TACX_DEVICE_NUMBER              2766U


void ant_stack_init(void);

int ant_setup_start(void);

void ant_timers_init(void);

#endif
