/*
 * MenuObjects.cpp
 *
 *  Created on: 7 déc. 2018
 *      Author: Vincent
 */

#include "Menuable.h"
#include "MenuObjects.h"
#include "Model.h"
#include "segger_wrapper.h"

MenuItem::MenuItem(MenuPage &parent, const char* name, f_menu_callback _p_func, MenuPage *_p_page) : p_parent(parent) {
	m_name = name;
	p_func = _p_func;
	p_page = _p_page;
}

eFuncMenuAction MenuItem::clickAction(uint8_t ind_sel) {

	if (!ind_sel) {
		// Return button was clicked
		p_parent.goToParent();
		return eFuncMenuActionEndMenu;
	}

	// ind _sel > 0
	if (p_func && eFuncMenuActionEndMenu == p_func(ind_sel)) {
		p_parent.closeMenuPopagate();
		return eFuncMenuActionEndMenu;
	} else {
		if (p_page) {
			// go to submenu
			p_parent.goToPage(p_page);
			return eFuncMenuActionEndMenu;
		}
	}

	return eFuncMenuActionEndMenu;
}

eFuncMenuAction MenuItem::validateAction(int var) {

	if (p_func) {
		p_func(var);
		p_parent.goToParent();

		LOG_INFO("Validation");

		return eFuncMenuActionEndMenu;
	}

	return eFuncMenuActionEndMenu;
}

void MenuItem::render(void) {

	vue.print("  ");
	vue.println(this->getName());

}

void MenuItem::render(bool isSelec) {

	vue.print("  ");
	vue.print(this->getName());
	if (isSelec) vue.println(" <=");
	else vue.println();

}

void MenuItem::addSubPage(MenuPage &page) {
	p_page = &page;
}

MenuPage::MenuPage(Menuable &menu, MenuPage *parent) : p_parent(parent), p_menu(menu) {

	// insert the "go back" element
	m_items.push_back(MenuItem(*this, "Retour"));
	ind_sel = 0;
}

void MenuPage::goToParent(void) {
	p_menu.goToParentPage();
	ind_sel = 0;
}

void MenuPage::goToPage(MenuPage *page) {
	p_menu.goToChildPage(page);
	ind_sel = 0;
}

void MenuPage::propagateEvent(eButtonsEvent event) {

	switch (event) {
	case eButtonsEventLeft:
	{
		ind_sel += m_items.size() - 1;
	}
	break;

	case eButtonsEventRight:
	{
		ind_sel += 1;
	}
	break;

	case eButtonsEventCenter:
	{
		if (eFuncMenuActionEndMenu == m_items[ind_sel].clickAction(ind_sel)) {
			ind_sel = 0;
		}
	}
	break;

	default:
		break;
	}

	ind_sel = ind_sel % m_items.size();
}

void MenuPage::render(void) {
	// render page
	vue.setCursor(0, 20);
	vue.setTextSize(3);

	ind_sel = ind_sel % m_items.size();

	uint8_t i = 0;
	for (auto& item : m_items) {
		item.render(i++ == ind_sel);
	}

	LOG_INFO("Menu displayed: %u items", i);
}

void MenuPage::closeMenuPopagate(void) {
	p_menu.closeMenu();
}

void MenuPage::addItem(MenuItem& item) {
	m_items.push_back(item);
}

uint16_t MenuPage::nbItems(void) {
	return m_items.size();
}


MenuPageSetting::MenuPageSetting(int value, Menuable &menu, MenuPage *parent) : MenuPage(menu, parent), m_value(value) {

}

void MenuPageSetting::propagateEvent(eButtonsEvent event) {

	ind_sel = 0;

	switch (event) {
	case eButtonsEventLeft:
	{
		m_value--;
	}
	break;

	case eButtonsEventRight:
	{
		m_value++;
	}
	break;

	case eButtonsEventCenter:
	{
		ASSERT(m_items.size() > 1);
		m_items[1].validateAction(m_value);
	}
	break;

	default:
		break;
	}

}

void MenuPageSetting::render(void) {
	// render page
	vue.setCursor(0, 20);
	vue.setTextSize(3);

	ASSERT(m_items.size() > 1);
	m_items[1].render();

	vue.println();
	vue.setTextSize(4);

	vue.print("  ");
	vue.println(String(m_value));
}
