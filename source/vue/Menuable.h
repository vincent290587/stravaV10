/*
 * Menuable.h
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_MENUABLE_H_
#define SOURCE_VUE_MENUABLE_H_

#include <stdint.h>
#include <stdbool.h>
#include <button.h>
#include <list>
#include "WString.h"

typedef enum {
	eMenuableModeAffi,
	eMenuableModeMenu,
	eMenuableModeSubMenu,
} eMenuableMode;

class MenuPage;

class Menuable {
public:
	Menuable();
	virtual ~Menuable();

	void initMenu(void);

	void refreshMenu(void);

	void propagateEvent(eButtonsEvent event);

	void tasksMenu(void);

	void goToParentPage(void);

	void goToChildPage(MenuPage *page);

	void closeMenu();

	virtual void refresh(void)=0;

	bool m_is_menu_selected;

	std::list<MenuPage> m_menus;
protected:

	MenuPage *p_cur_page;
};

#endif /* SOURCE_VUE_MENUABLE_H_ */
