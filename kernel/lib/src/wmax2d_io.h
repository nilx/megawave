/*
 * wmax2d_io.h
 */

#ifndef _WMAX2D_IO_H
#define _WMAX2D_IO_H

Vchains_wmax _mw_load_vchains_wmax(char *);
short _mw_create_vchains_wmax(char *, Vchains_wmax);
Vchain_wmax _mw_load_vchain_wmax(char *);
short _mw_create_vchain_wmax(char *, Vchain_wmax);

#endif /* !_WMAX2D_IO_H */
