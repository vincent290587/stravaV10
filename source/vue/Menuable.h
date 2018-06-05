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
#include <Adafruit_GFX.h>
#include <button.h>
#include "WString.h"

#define MENU_MAX_ITEMS_NB     10

typedef void (*f_menu_callback)(int);

typedef enum {
	eMenuableModeAffi,
	eMenuableModeMenu,
	eMenuableModeSubMenu,
} eMenuableMode;

typedef struct {
	String          name;
	f_menu_callback p_func;
} sMenuItem;

typedef struct {
	uint16_t  nb_elem;
	sMenuItem item[MENU_MAX_ITEMS_NB];
} sMenuPage;

typedef struct {
	uint16_t  cur_page;
	sMenuPage menu_page[MENU_MAX_ITEMS_NB];
} sMenus;

void menu_init_page(sMenuPage *page);

void menu_add_item(sMenuPage *page, const char *name, f_menu_callback callback);

class Menuable: virtual public Adafruit_GFX {
public:
	Menuable();
	virtual ~Menuable();

	void initMenu(void);

	void refreshMenu(void);

	void propagateEvent(eButtonsEvent event);

	void tasksMenu(void);

	virtual void refresh(void)=0;

	bool m_is_menu_selected;
	sMenus m_menus;
protected:
	uint8_t m_ind_selec;

};

#endif /* SOURCE_VUE_MENUABLE_H_ */
