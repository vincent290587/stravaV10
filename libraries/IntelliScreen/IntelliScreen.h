
#ifndef _INTELLI_H_
#define _INTELLI_H_

#include <stdint.h>
#include "WString.h"

#define NB_MENU_ITEMS     10
#define NB_MENU           5
#define I_MODE_MENU       0

/**
 * Enums
 */
enum ACTIONS {
	NO_ACTION = 0,
	BUTTON_DOWN,
	BUTTON_UP,
	BUTTON_PRESS,
	ACTIONS_NB
};


typedef void (*intelli_event)(int);

typedef struct {
	String name;
	intelli_event p_func;  // Declare typedef
} sIntelliMenuItem;

typedef struct {
	uint16_t nb_elem;
	sIntelliMenuItem item[NB_MENU_ITEMS];  // Declare typedef
} sIntelliMenu;


class IntelliScreen {
public:
	IntelliScreen();
	uint8_t getModeCalcul() {return _mode_calcul;}
	uint8_t getModeAffi();
	uint8_t getModeAffiPrec() {return _mode_affi_prec;}
	void setModeCalcul(uint8_t mode) {_mode_calcul = mode;}
	void setModeAffi(uint8_t mode) {_mode_affi = mode;}
	void setModeAffiPrec(uint8_t mode) {_mode_affi_prec = mode;}
	void setStoredMode(uint8_t mode) {_stored_mode = mode;}
	uint8_t getStoredMode() {return _stored_mode;}
	uint8_t getSelectionMenu() {return _selectionMenu;}
	uint16_t getNbElemMenu(uint8_t menu_ind=0) {return menu[menu_ind].nb_elem;}
	String getMenuItem(uint8_t menu_ind, uint16_t indice) {return menu[menu_ind].item[indice].name;}
	int getActiveSubMenu(void);
	void activateSubMenu(int indm);
	void deactivateMenu();

	void addMenuItem(uint8_t menu_ind, sIntelliMenuItem *item);

	void menuDescend ();
	void menuMonte ();
	void menuClic ();

	uint8_t _is_menu_active;

	static IntelliScreen* pIntelliScreen;
protected:


private:
	uint8_t _mode_affi, _mode_affi_prec, _mode_calcul;
	uint8_t _stored_mode;
	uint8_t _selectionMenu, m_act_menu;

	sIntelliMenu menu[NB_MENU];
};


#endif
