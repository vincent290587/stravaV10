
#ifndef BOARD_CUSTOM_H
#define BOARD_CUSTOM_H

#ifndef SUPERSEDE_BOARD_ID
#define PROTO_V11
#endif

#if defined (PROTO_V11)
#include "custom_board_v3.h"
#elif defined (PROTO_V10)
#include "custom_board_v2.h"
#else
#include "custom_board_v1.h"
#endif

#endif // BOARD_CUSTOM_H
