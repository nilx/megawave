/*
 * window.h
 */

#ifndef _WINDOW_H_
#define _WINDOW_H_

/* src/window.c */
Wframe *mw_get_window(Wframe *window, int dx, int dy, int x0, int y0, char *title);
void mw_window_notify(Wframe *Win, void *param, int (*proc)(Wframe *, void *));
void mw_window_main_loop(void);

#endif /* !_WINDOW_H_ */
