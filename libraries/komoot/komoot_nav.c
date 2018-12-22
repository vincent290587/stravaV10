/*
 * komoot_nav.c
 *
 *  Created on: 22 déc. 2018
 *      Author: Vincent
 */


#include "komoot_nav.h"
#include "komoot_icons.h"


const uint8_t* komoot_nav_get_icon(uint8_t direction) {

	const uint8_t *res = 0;

	switch (direction) {
	case 1:
		res = ic_nav_arrow_keep_going;
		break;
	case 2:
		res = ic_nav_arrow_start;
		break;
	case 3:
		res = ic_nav_arrow_finish;
		break;
	case 4:
		res = ic_nav_arrow_keep_left;
		break;
	case 5:
		res = ic_nav_arrow_turn_left;
		break;
	case 6:
		res = ic_nav_arrow_turn_hard_left;
		break;
	case 7:
		res = ic_nav_arrow_turn_hard_right;
		break;
	case 8:
		res = ic_nav_arrow_turn_right;
		break;
	case 9:
		res = ic_nav_arrow_keep_right;
		break;
	case 10:
		res = ic_nav_arrow_fork_right;
		break;
	case 11:
		res = ic_nav_arrow_fork_left;
		break;
	case 12:
		res = ic_nav_arrow_uturn;
		break;
	case 13:
		break;
	case 14:
		break;
	case 15:
		res = ic_nav_roundabout_exit_ccw;
		break;
	case 16:
		res = ic_nav_roundabout_exit_cw;
		break;
	case 17:
		res = ic_nav_roundabout_ccw1_1;
		break;
	case 18:
		res = ic_nav_roundabout_ccw1_2;
		break;
	case 19:
		res = ic_nav_roundabout_ccw1_3;
		break;
	case 20:
		res = ic_nav_roundabout_ccw2_2;
		break;
	case 21:
		res = ic_nav_roundabout_ccw2_3;
		break;
	case 22:
		res = ic_nav_roundabout_ccw3_3;
		break;
	case 23:
		res = ic_nav_roundabout_cw1_1;
		break;
	case 24:
		res = ic_nav_roundabout_cw1_2;
		break;
	case 25:
		res = ic_nav_roundabout_cw1_3;
		break;
	case 26:
		res = ic_nav_roundabout_cw2_2;
		break;
	case 27:
		res = ic_nav_roundabout_cw2_3;
		break;
	case 28:
		res = ic_nav_roundabout_cw3_3;
		break;
	case 29:
		res = ic_nav_roundabout_fallback;
		break;
	case 30:
		res = ic_nav_outof_route;
		break;
	default:
		break;
	}

	return res;
}
