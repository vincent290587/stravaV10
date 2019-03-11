
#ifndef __ANT_C__
#define __ANT_C__

#include "mk64f_parser.h"
#include "g_structs.h"

typedef enum {
	eAntSensorsChannelHRM,
	eAntSensorsChannelBSC,
	eAntSensorsChannelFEC,
	eAntSensorsChannelBS
} eAntSensorsChannelNumber;

#define WILDCARD_TRANSMISSION_TYPE      0x00
#define ANTPLUS_NETWORK_NUMBER          0x00                                           /**< Network number. */

#define HRM_CHANNEL_NUMBER              0x00
#define HRM_DEVICE_NUMBER               0x0D22    /**< Device Number. */

#define BSC_CHANNEL_NUMBER              0x01
#define BSC_DEVICE_NUMBER               0xB02B    /**< Device Number. */
#define BSC_DEVICE_TYPE                 0x79

#define GLASSES_CHANNEL_NUMBER          0x02

#define FEC_CHANNEL_NUMBER              0x03
#define TACX_DEVICE_NUMBER              2766U

#define BS_CHANNEL_NUMBER               0x04



typedef enum {
	eAntPairingSensorTypeNone,
	eAntPairingSensorTypeHRM,
	eAntPairingSensorTypeBSC,
	eAntPairingSensorTypeFEC
} eAntPairingSensorType;


extern sHrmInfo hrm_info;
extern sBscInfo bsc_info;

#ifdef __cplusplus
extern "C" {
#endif

void ant_tasks(void);

void ant_stack_init(void);

int ant_setup_start(void);

void ant_search_start(eAntPairingSensorType search_type);

void ant_search_end(uint16_t dev_id);

void ant_timers_init(void);

#ifdef __cplusplus
}
#endif

#endif
