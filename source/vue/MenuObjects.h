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

typedef int (*f_setting_callback)(int);

typedef struct {
	f_setting_callback pre_hook;
	f_setting_callback post_hook;
} sSettingsCallbacks;

#ifndef SOURCE_VUE_MENUABLE_H_
#endif

class MenuItem {
public:
	MenuItem(MenuPage &parent, const char *name, f_menu_callback _p_func = nullptr, MenuPage *p_page = nullptr);

	virtual eFuncMenuAction clickAction(uint8_t ind_sel);
	virtual eFuncMenuAction validateAction(int var);

	virtual void render(void);
	virtual void render(bool isSelec);

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

	virtual void render(void);
	virtual void propagateEvent(eButtonsEvent event);

	void goToPage(MenuPage *page);

	MenuPage* getParent() const {
		return p_parent;
	}

protected:
	MenuPage *p_parent;
	Menuable &p_menu;
	uint8_t ind_sel;
};

class MenuPageItems : public MenuPage {
public:
	MenuPageItems(Menuable &menu, MenuPage *parent = nullptr);

	void addItem(MenuItem &item);
	uint16_t nbItems(void);

	virtual void render(void);

	virtual void propagateEvent(eButtonsEvent event);


private:
	std::vector<MenuItem> m_items;
};


class MenuPageSetting : public MenuPage {
public:
	MenuPageSetting(Menuable &menu, MenuPage *parent = nullptr);

	void setCallbacks(sSettingsCallbacks &calls);
	void setName(const char *name) {m_name = name;}

	virtual void render(void);

	virtual void propagateEvent(eButtonsEvent event);


private:
	int m_value;
	String m_name;
	sSettingsCallbacks m_callbacks;
};



#endif /* SOURCE_VUE_MENUOBJECTS_H_ */
