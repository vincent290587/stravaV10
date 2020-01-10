/*
 * sdk_config.h
 *
 *  Created on: 9 janv. 2020
 *      Author: vgol
 */

#ifndef TDD_NORDIC_SDK_CONFIG_H_
#define TDD_NORDIC_SDK_CONFIG_H_

#include <stdint.h>

/**
 * @brief API Result.
 *
 * @details Indicates success or failure of an API procedure. In case of failure, a comprehensive
 *          error code indicating cause or reason for failure is provided.
 *
 *          Though called an API result, it could used in Asynchronous notifications callback along
 *          with asynchronous callback as event result. This mechanism is employed when an event
 *          marks the end of procedure initiated using API. API result, in this case, will only be
 *          an indicative of whether the procedure has been requested successfully.
 */
typedef uint32_t ret_code_t;


#endif /* TDD_NORDIC_SDK_CONFIG_H_ */
