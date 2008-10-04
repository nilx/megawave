/*
 * wpanel.h
 */

#ifndef _WPANEL_H_
#define _WPANEL_H_

/* src/wpanel.c */
void Wp_SetButton(int type, Wpanel wp, void *b);
int Wp_handle(Wpanel wp, int event, int x, int y);
void Wp_main_loop(Wpanel wp);

#endif /* !_WPANEL_H_ */
