#include "menu.h"

/** This is used when an invalid menu handle is required in
 *  a \ref MENU_ITEM() definition, i.e. to indicate that a
 *  menu has no linked parent, child, next or previous entry.
 */
Menu_Item_t NULL_MENU = {0};

/** \internal
 *  Pointer to the generic menu text display function
 *  callback, to display the configured text of a menu item
 *  if no menu-specific display function has been set
 *  in the select menu item.
 */
static void (*MenuWriteFunc)(Menu_Item_t* currentMenuItem, Menu_Item_t* currentMenuLayerTopItem) = NULL;

static Menu_Item_t* CurrentMenuItem = &NULL_MENU;
static Menu_Item_t* CurrentMenuLayerTopItem = &NULL_MENU;

Menu_Item_t* Menu_GetCurrentMenu(void)
{
	return CurrentMenuItem;
}

Menu_Item_t* Menu_GetCurrentLayerTopMenu(void)
{
	return CurrentMenuLayerTopItem;
}

void Menu_SetCurrentLayerTopMenu(Menu_Item_t* currentMenuLayerTopItem)
{
	CurrentMenuLayerTopItem = currentMenuLayerTopItem;
}

void Menu_Navigate(Menu_Item_t* const NewMenu, Menu_Item_t* const NewCurrentMenuLayerTopItem)
{
	if ((NewMenu == &NULL_MENU) || (NewMenu == NULL)) {
		return;
	}
	CurrentMenuItem = NewMenu;
	CurrentMenuLayerTopItem = NewCurrentMenuLayerTopItem;
	if (MenuWriteFunc) {
		MenuWriteFunc(CurrentMenuItem, CurrentMenuLayerTopItem);
	}
}

void Menu_SetGenericWriteCallback(void (*WriteFunc)(Menu_Item_t* currentMenuItem, Menu_Item_t* currentMenuLayerTopItem), Menu_Item_t* const NewCurrentMenuLayerTopItem)
{
	MenuWriteFunc = WriteFunc;
}

void Menu_EnterCurrentItem(void)
{
	if ((CurrentMenuItem == &NULL_MENU) || (CurrentMenuItem == NULL)) {
		return;
	}
	void (*EnterCallback)(void) = CurrentMenuItem->EnterCallback;
	if (EnterCallback) {
		EnterCallback();
	}
}
