/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef OMWARG_INC
#define OMWARG_INC

#ifdef OMWARG_DEC
char optb[BUFSIZ], *popt;
#else
extern char optb[], *popt;
#endif

# ifdef __STDC__
extern void print_io_include(FILE *);
extern void print_io_function(FILE *);
extern void print_is_range(FILE *, Io, Paramtype *, DataIo *, char *);
extern void print_verify_io_arg(FILE *);
extern char *noquote(char*);
# else
extern int print_io_include();
extern int print_io_function();
extern int print_is_range();
extern int print_verify_io_arg();
extern char *noquote();
# endif

#endif
