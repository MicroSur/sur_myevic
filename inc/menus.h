#ifndef __MENUS_H__
#define __MENUS_H__


typedef struct menu_s menu_t;

extern menu_t const	*CurrentMenu;
extern unsigned char	CurrentMenuItem;

extern const menu_t GameMenu;
extern const menu_t GameTtMenu;

void DrawMenu();
int MenuEvent( int event );


#endif /* __MENUS_H__ */
