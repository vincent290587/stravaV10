
#include "IntelliScreen.h"

#define NB_ELEM_MENU   menu[m_act_menu].nb_elem


IntelliScreen* IntelliScreen::pIntelliScreen = nullptr;


static void callbackMENU(int entier) {
	// empty callback
}

static void callbackSUBMENU(int entier) {
	// empty callback
}

IntelliScreen::IntelliScreen() {

	_mode_calcul = 0;
	_mode_affi = 0;

	_is_menu_active = 0;
	m_act_menu = 0;

	pIntelliScreen = this;

	memset(&menu, 0, sizeof(sIntelliMenu));

	// init all the menu items
	sIntelliMenuItem item;

	item.name = "Menu";
	item.p_func = callbackMENU;
	this->addMenuItem(0, &item);

}


uint8_t IntelliScreen::getModeAffi() {

	if (_is_menu_active) {
		return I_MODE_MENU;
	} else {
		return _mode_affi;
	}

}

void IntelliScreen::menuDescend () {
	if (_is_menu_active) {
		_selectionMenu++;
		_selectionMenu = _selectionMenu % NB_ELEM_MENU;
	}
}

void IntelliScreen::menuMonte () {
	if (_is_menu_active) {
		_selectionMenu+=NB_ELEM_MENU - 1;
		_selectionMenu = _selectionMenu % NB_ELEM_MENU;
	}
}


void IntelliScreen::menuClic () {

	if (_is_menu_active) {

		_selectionMenu = _selectionMenu % NB_ELEM_MENU;

		if (_selectionMenu == I_MODE_MENU && m_act_menu == 0) {
			// back to affi
			_is_menu_active = 0;
		} else if (_selectionMenu == I_MODE_MENU) {
			// go back to main menu
			m_act_menu = 0;
		} else {

			// if item was clicked: call the function
			if (_selectionMenu > I_MODE_MENU) {
				// an item other than menu was clicked
				// -> call its handler
				(*menu[m_act_menu].item[_selectionMenu].p_func)(_selectionMenu);
			}

		}

	} else {
		// activate menu
		_is_menu_active = 1;
		m_act_menu = 0;
		// activate first element
		_selectionMenu = 1;
	}

}

void IntelliScreen::activateSubMenu(int indm) {

	m_act_menu = indm;

	// activate first element
	_selectionMenu = 1;

}

int IntelliScreen::getActiveSubMenu(void) {
	return m_act_menu;
}

void IntelliScreen::deactivateMenu() {
	m_act_menu = 0;
	_is_menu_active = 0;
}

void IntelliScreen::addMenuItem(uint8_t menu_ind, sIntelliMenuItem *item) {

	// check submenus
	if (menu_ind && menu[menu_ind].nb_elem == 0) {

		// this submenu does not have the return button yet
		menu[menu_ind].item[0].name = "Retour";
		menu[menu_ind].item[0].p_func = callbackSUBMENU;
		menu[menu_ind].nb_elem = 1;

	}

	menu[menu_ind].item[menu[menu_ind].nb_elem].name = item->name;
	menu[menu_ind].item[menu[menu_ind].nb_elem].p_func = item->p_func;
	menu[menu_ind].nb_elem++;
}
