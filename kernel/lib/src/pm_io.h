/*
 * pm_io.h
 */

#ifndef _PM_IO_H
#define _PM_IO_H

Cimage _mw_cimage_load_pm(char *);
short _mw_cimage_create_pm(char *, Cimage);
Fimage _mw_fimage_load_pm(char *);
short _mw_fimage_create_pm(char *, Fimage);
Ccimage _mw_ccimage_load_pm(char *);
short _mw_ccimage_create_pm(char *, Ccimage);
Cfimage _mw_cfimage_load_pm(char *);
short _mw_cfimage_create_pm(char *, Cfimage);

#endif /* !_PM_IO_H */
