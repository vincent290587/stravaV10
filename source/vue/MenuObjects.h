/*
 * MenuObjects.h
 *
 *  Created on: 7 déc. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_MENUOBJECTS_H_
#define SOURCE_VUE_MENUOBJECTS_H_

#include <stdint.h>
#include <stdbool.h>
#include <button.h>
#include <Adafruit_GFX.h>
#include <vector>
#include "WString.h"

class MenuPage;
class Menuable;

typedef enum {
	eFuncMenuActionNone,
	eFuncMenuActionEndMenu
} eFuncMenuAction;

typedef eFuncMenuAction (*f_menu_callback)(int);

#ifndef SOURCE_VUE_MENUABLE_H_
#endif

class MenuItem {
public:
	MenuItem(MenuPage &parent, const char *name, f_menu_callback _p_func = nullptr, MenuPage *p_page = nullptr);

	eFuncMenuAction clickAction(uint8_t ind_sel);

	const char* getName(void) {
		return m_name.c_str();
	}

	void addSubPage(MenuPage &page);

private:
	String m_name;
	f_menu_callback p_func;
	MenuPage &p_parent;
	MenuPage *p_page;
};

class MenuPage {
public:
	MenuPage(Menuable &menu, MenuPage *parent = nullptr);

	void goToParent(void);
	void closeMenuPopagate(void);
	void render(void);

	void propagateEvent(eButtonsEvent event);

	void goToPage(MenuPage *page);

	void addItem(MenuItem &item);
	uint16_t nbItems(void);

	MenuPage* getParent() const {
		return p_parent;
	}

private:
	MenuPage *p_parent;
	Menuable &p_menu;
	std::vector<MenuItem> m_items;
	uint8_t ind_sel;
};




#endif /* SOURCE_VUE_MENUOBJECTS_H_ */
