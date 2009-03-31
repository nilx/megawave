/*
 * gif_io.h
 */

#ifndef _GIF_IO_H_
#define _GIF_IO_H_

/* src/gif_io.c */
Cimage _mw_cimage_load_gif(char *fname);
short _mw_cimage_create_gif(char *fname, Cimage image);

#endif /* !_GIF_IO_H_ */
