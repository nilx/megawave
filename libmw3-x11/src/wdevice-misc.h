/*
 * wdevice-misc.h
 */

#ifndef _WDEVICE_MISC_H_
#define _WDEVICE_MISC_H_

/* src/wdevice-misc.c */
int WX_ErrorHandler(Display *display, XErrorEvent *error);
int WX_Init(char *theDisplayName);
void WX_FreeColors(void);
void WX_AllocColors(void);
void WX_CreateXImage(Wframe *window, int dx, int dy);
void WX_AllocXImage(Wframe *window, int dx, int dy);
void WX_AllocXPixmap(Wframe *window, int dx, int dy);
void WX_Ditherize(Wframe *window, int dx, int dy);
int WX_KeyPress(XKeyEvent *event);

#endif /* !_WDEVICE_MISC_H_ */
