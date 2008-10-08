/*
 * wpanel.h
 */

#ifndef _WPANEL_H_
#define _WPANEL_H_

/* src/wpanel.c */
int Wp_DrawButton(Wframe *window, int x, int y, char *str, int color);
void Wp_DrawScale(Wframe *window, int x, int y, int pos, int divisions, int length, int color);
Wpanel Wp_Init(Wframe *window);
void Wp_SetButton(int type, Wpanel wp, void *b);
int Wp_handle(Wpanel wp, int event, int x, int y);
int Wp_notify(Wframe *window, void *wp);
void Wp_main_loop(Wpanel wp);

#endif /* !_WPANEL_H_ */
