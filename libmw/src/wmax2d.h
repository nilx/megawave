/*
 * wmax2d.h
 */

#ifndef _WMAX2D_H_
#define _WMAX2D_H_

/* src/wmax2d.c */
Vpoint_wmax mw_new_vpoint_wmax(void);
Vpoint_wmax mw_change_vpoint_wmax(Vpoint_wmax vpoint);
void mw_delete_vpoint_wmax(Vpoint_wmax vpoint);
Vchain_wmax mw_new_vchain_wmax(void);
Vchain_wmax mw_change_vchain_wmax(Vchain_wmax vchain);
void mw_delete_vchain_wmax(Vchain_wmax vchain);
Vchains_wmax mw_new_vchains_wmax(void);
Vchains_wmax mw_change_vchains_wmax(Vchains_wmax vchains);
void mw_delete_vchains_wmax(Vchains_wmax vchains);
Vpoint_wmax mw_copy_vpoint_wmax(Vpoint_wmax vpoint1, Vpoint_wmax vpoint0);
Vchain_wmax mw_copy_vchain_wmax(Vchain_wmax vchain1, Vchain_wmax vchain0);
int mw_give_nlevel_vchain(Vchain_wmax vchain);

#endif                          /* !_WMAX2D_H_ */
