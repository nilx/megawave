/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef DATA_IO_INC
#define DATA_IO_INC

typedef struct DataIo {
#ifdef __STDC__
  Type   t;
#else
  short  t;
#endif
  char *type;
#ifdef __STDC__
  Io     rw;
#else
  short  rw;
#endif
  char * function;
  char * include;
} DataIo;

#ifdef DATA_IO_DEC
Header data_io_list;
#else
extern Header data_io_list;
#endif

#ifdef __STDC__
extern void read_data_io(FILE *, char *, Header *);
extern Cell *lookup_data_io(Header *, Mwarg *, Io *);
extern Cell *lookup_data_io_for_node(Header *, Node *, Node *, Io *);
char *mwio(Io, Mwarg *);
#else
extern void read_data_io();
extern Cell *lookup_data_io();
extern Cell *lookup_data_io_for_node();
char *mwio();
#endif

#define _SET_IO_T(C, T)          ((C)->t = (T))
#define _GET_IO_T(C)             ((C)->t)
#define SET_IO_T(C, T)           (_GET_IO_T((DataIo *)GET_ELT(C)) = (T))
#define GET_IO_T(C)              (_GET_IO_T((DataIo *)GET_ELT(C)))

#define _SET_IO_TYPE(C, T)       ((C)->type = (T))
#define _GET_IO_TYPE(C)          ((C)->type)
#define SET_IO_TYPE(C, T)        (_GET_IO_TYPE((DataIo *)GET_ELT(C)) = (T))
#define GET_IO_TYPE(C)           (_GET_IO_TYPE((DataIo *)GET_ELT(C)))

#define _SET_RW(C, RW)           ((C)->rw = (RW))
#define _GET_RW(C)               ((C)->rw)
#define SET_RW(C, RW)            (_GET_RW((DataIo *)GET_ELT(C)) = (RW))
#define GET_RW(C)                (_GET_RW((DataIo *)GET_ELT(C)))

#define _SET_FUNCTION(C, F)      ((C)->function = (F))
#define _GET_FUNCTION(C)         ((C)->function)
#define SET_FUNCTION(C, F)       (_GET_FUNCTION((DataIo *)GET_ELT(C)) = (F))
#define GET_FUNCTION(C)          (_GET_FUNCTION((DataIo *)GET_ELT(C)))

#define _SET_INCLUDE(C, I)       ((C)->include = (I))
#define _GET_INCLUDE(C)          ((C)->include)
#define SET_INCLUDE(C, I)        (_GET_INCLUDE((DataIo *)GET_ELT(C)) = (I))
#define GET_INCLUDE(C)           (_GET_INCLUDE((DataIo *)GET_ELT(C)))

#define ALL                      NULL

#define READ_DATA_IO(FD, NAME)            read_data_io((FD), (NAME), &data_io_list)
#define LOOKUP_DATA_IO(A, RW)             lookup_data_io(&data_io_list, (A), (RW))
#define LOOKUP_DATA_IO_FOR_NODE(T, A, RW) lookup_data_io_for_node(&data_io_list, (T),\
                                                                  (A), (RW))

#endif
