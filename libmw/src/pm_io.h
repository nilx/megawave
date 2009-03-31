/*
 * pm_io.h
 */

#ifndef _PM_IO_H_
#define _PM_IO_H_

/* src/pm_io.c */
Cimage _mw_cimage_load_pm(char *file);
short _mw_cimage_create_pm(char *file, Cimage image);
Fimage _mw_fimage_load_pm(char *file);
short _mw_fimage_create_pm(char *file, Fimage image);
Ccimage _mw_ccimage_load_pm(char *file);
short _mw_ccimage_create_pm(char *file, Ccimage image);
Cfimage _mw_cfimage_load_pm(char *file);
short _mw_cfimage_create_pm(char *file, Cfimage image);

#endif                          /* !_PM_IO_H_ */
