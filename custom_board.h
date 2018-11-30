/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef BOARD_CUSTOM_H
#define BOARD_CUSTOM_H

#define PROTO_V10

#if defined (PROTO_V10)
#include "custom_board_v2.h"
#else
#include "custom_board_v1.h"
#endif

#endif // BOARD_CUSTOM_H
