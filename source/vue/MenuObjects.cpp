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

void MenuPage::closeMenuPopagate(void) {
	p_menu.closeMenu();
}

void MenuPage::propagateEvent(eButtonsEvent event) {

	ind_sel = 0;

	switch (event) {
	case eButtonsEventLeft:
	{
	}
	break;

	case eButtonsEventRight:
	{
	}
	break;

	case eButtonsEventCenter:
	{
		this->goToParent();
	}
	break;

	default:
		break;
	}

}

void MenuPage::render(void) {

}

MenuPageItems::MenuPageItems(Menuable &menu, MenuPage *parent) : MenuPage(menu, parent) {

	// insert the "go back" element
	m_items.push_back(MenuItem(*this, "Retour"));
}

void MenuPageItems::addItem(MenuItem& item) {
	m_items.push_back(item);
}

void MenuPageItems::removeAllItems(void) {
	m_items.clear();
}

uint16_t MenuPageItems::nbItems(void) {
	return m_items.size();
}

void MenuPageItems::propagateEvent(eButtonsEvent event) {

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

void MenuPageItems::render(void) {
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


MenuPageSetting::MenuPageSetting(Menuable &menu, MenuPage *parent) : MenuPage(menu, parent), m_value(0), m_name("Value:") {

	m_callbacks.pre_hook = nullptr;
	m_callbacks.post_hook = nullptr;
}

void MenuPageSetting::setCallbacks(sSettingsCallbacks &calls) {

	m_callbacks.pre_hook = calls.pre_hook;
	m_callbacks.post_hook = calls.post_hook;

	m_value = m_callbacks.pre_hook(0);
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
		if (m_callbacks.post_hook) m_callbacks.post_hook(m_value);
		this->goToParent();
	}
	break;

	default:
		break;
	}

}

void MenuPageSetting::render(void) {
	// render page
	vue.setCursor(20, 20);
	vue.setTextSize(3);

	// print name
	vue.println(m_name);

	// print value
	vue.setCursor(50, 50);
	vue.setTextSize(4);

	vue.print("  ");
	vue.println(String(m_value));
}

MenuPagePairing::MenuPagePairing(Menuable &menu, MenuPage *parent, f_pairing_callback func) :
		MenuPageItems(menu, parent), m_func(func) {

}

void MenuPagePairing::render(void) {

	// populate through call
	if (m_func) m_func(0);

	// render page
	MenuPageItems::render();
}
