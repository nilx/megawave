/*
 * mt19937ar.h
 */

#ifndef _MT19937AR_H_
#define _MT19937AR_H_

/* src/mt19937ar.c */
void mw_srand_mt(unsigned long s);
void mw_srand_mt_array(unsigned long init_key[], int key_length);
unsigned long mw_rand32_mt(void);
double mw_drand53_mt(void);

#endif                          /* !_MT19937AR_H_ */
