/*
 * wave_io.h
 */

#ifndef _WAVE_IO_H_
#define _WAVE_IO_H_

/* src/wave_io.c */
Fsignal _mw_fsignal_load_wave_pcm(char *fname, Fsignal signal,
                                  int need_flipping);
short _mw_fsignal_create_wave_pcm(char *fname, Fsignal signal);

#endif                          /* !_WAVE_IO_H_ */
