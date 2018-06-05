/*
 * sdk_libuarte.h
 *
 *  Created on: 5 juin 2018
 *      Author: Vincent
 */

#ifndef PCA10056_S332_CONFIG_SDK_LIBUARTE_H_
#define PCA10056_S332_CONFIG_SDK_LIBUARTE_H_


// <h> nrf_libuarte - libUARTE library

//==========================================================
// <o> NRF_LIBUARTE_CONFIG_TIMER_USED  - Timer instance

// <0=> 0
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4

#ifndef NRF_LIBUARTE_CONFIG_TIMER_USED
#define NRF_LIBUARTE_CONFIG_TIMER_USED 3
#endif

// <o> NRF_LIBUARTE_ASYNC_CONFIG_TIMER_USED  - Timer instance

// <0=> 0
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4

#ifndef NRF_LIBUARTE_ASYNC_CONFIG_TIMER_USED
#define NRF_LIBUARTE_ASYNC_CONFIG_TIMER_USED 4
#endif

// <q> NRF_LIBUARTE_CONFIG_UARTE_USED  - UARTE instance


#ifndef NRF_LIBUARTE_CONFIG_UARTE_USED
#define NRF_LIBUARTE_CONFIG_UARTE_USED 0
#endif

// </h>
//==========================================================

// </h>
//==========================================================


#endif /* PCA10056_S332_CONFIG_SDK_LIBUARTE_H_ */
