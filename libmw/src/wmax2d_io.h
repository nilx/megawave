/*
 * wmax2d_io.h
 */

#ifndef _WMAX2D_IO_H_
#define _WMAX2D_IO_H_

/* src/wmax2d_io.c */
Vchains_wmax _mw_load_vchains_wmax(char *fname);
short _mw_create_vchains_wmax(char *fname, Vchains_wmax vchains);
Vchain_wmax _mw_load_vchain_wmax(char *fname);
short _mw_create_vchain_wmax(char *fname, Vchain_wmax vchain);

#endif                          /* !_WMAX2D_IO_H_ */
