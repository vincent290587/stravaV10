/*
 * VuePRC.h
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_VUEPRC_H_
#define SOURCE_VUE_VUEPRC_H_

#include <Adafruit_GFX.h>
#include <display/Zoom.h>
#include <display/SegmentManager.h>
#include <routes/Parcours.h>
#include <vue/VueCRS.h>
#include <vue/VueGPS.h>
#include <vue/button.h>

typedef enum {
	eVuePRCScreenInit,
	eVuePRCScreenGps,
	eVuePRCScreenDataFull,
} eVuePRCScreenModes;

class VuePRC: virtual public Adafruit_GFX, virtual public VueGPS, protected Zoom {
public:
	VuePRC();

	eVuePRCScreenModes tasksPRC();

	bool propagateEventsPRC(eButtonsEvent event);

	void parcoursSelect(int prc_ind);

	void invalidatePRC(void);

	virtual void cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite)=0;
	virtual void cadran(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite)=0;

protected:
	eVuePRCScreenModes m_prc_screen_mode;
	float m_distance_prc;

private:
	void afficheSegment(uint8_t ligne, Segment *p_seg);
	void afficheParcours(uint8_t ligne, ListePoints2D *p_liste);

	Parcours *m_s_parcours;
};

#endif /* SOURCE_VUE_VUEPRC_H_ */
