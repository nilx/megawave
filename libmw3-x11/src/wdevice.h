/*
 * wdevice.h
 */

#ifndef _WDEVICE_H_
#define _WDEVICE_H_

/* src/wdevice.c */
int WIsAnActiveWindow(Wframe *window);
void WSetColorMap(void);
void WFlushWindow(Wframe *window);
void WFlushAreaWindow(Wframe *window, int x0, int y0, int x1, int y1);
int WColorsAvailable(void);
void WSetColorPencil(Wframe *window, int color);
void WSetForegroundColorPencil(Wframe *window);
void WSetBackgroundColorPencil(Wframe *window);
void WSetSpecialColorPencil(Wframe *window);
void WSetTypePencil(int opt);
void WDrawPoint(Wframe *window, int x, int y);
void WDrawLine(Wframe *window, int x0, int y0, int x1, int y1);
void WDrawString(Wframe *window, int x, int y, char *text);
void WDrawRectangle(Wframe *window, int x0, int y0, int x1, int y1);
void WFillRectangle(Wframe *window, int x0, int y0, int x1, int y1);
void WClearWindow(Wframe *window);
void WDestroyWdeviceWindow(Wframe *window);
void WDestroyWindow(Wframe *window);
void WMoveWindow(Wframe *window, int x, int y);
void WPutTitleWindow(Wframe *window, char *title);
void WSaveImageWindow(Wframe *window, int x, int y, int width, int height);
void WRestoreImageWindow(Wframe *window, int x, int y, int width, int height);
int WLoadBitMapImage(Wframe *window, unsigned char *bitmap, int width, int height);
int WLoadBitMapColorImage(Wframe *window, unsigned char *Red, unsigned char *Green, unsigned char *Blue, int width, int height);
void WSystemEvent(Wframe *window);
void WSetUserEvent(Wframe *window, long unsigned int user_event_mask);
int WUserEvent(Wframe *window);
int WGetStateMouse(Wframe *window, int *x, int *y, int *button_mask);
int WGetKeyboard(void);
Wframe *WNewImageWindow(void);
Wframe *WOpenImageWindow(int width, int height, int ltx, int lty, char *label);
void WReOpenImageWindow(Wframe *window, int width, int height, int ltx, int lty, char *label);

#endif /* !_WDEVICE_H_ */
