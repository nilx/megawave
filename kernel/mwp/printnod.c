/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <ctype.h>
#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "io.h"
#include "y.tab.h"

#define NL         putc('\n', fd); nl_flg = TRUE
#define PRLEFT(N)  printnode(fd,(N)->left)
#define PRRIGHT(N) printnode(fd,(N)->right)
#define PR0(S)     fprintf(fd,S); nl_flg = FALSE
#define PR1(S1,S2) fprintf(fd, S1, S2); nl_flg = FALSE

#define SYNC(N)    if ((N)->filein != NULL && (N)->lineno != 0) {\
                     if (nl_flg == FALSE) NL;\
                     fprintf(fd, "# %d \"%s\"\n", (N)->lineno,\
                                                    (N)->filein);\
                   }

#define TEX_NL         putc('\n',fd); putc('\n', fd); nl_flg = TRUE
#define TEX_PRLEFT(N)  texprintnode(fd,(N)->left)
#define TEX_PRRIGHT(N) texprintnode(fd,(N)->right)
#define TEX_PR0(S)     fprinttex(fd,S); nl_flg = FALSE
#define TEX_PR1(S1,S2) fprinttex(fd, S1, S2); nl_flg = FALSE

#define TEX_SYNC(N) 

static int nl_flg = FALSE;

#ifdef __STDC__
char *rename_func(char *);
#else
char *rename_func();
#endif

#ifdef __STDC__
void printnode(FILE *fd, Node *n)
#else
printnode(fd, n)
FILE *fd;
Node *n;
#endif
{
  Node *nn;
  int i;
  Symbol *s;

#ifdef DEBUG
  if (n != NULL)
    switch(n->name) {
      case NAME :
      case USRTYPID :
        PRDBG("printnode : Node = 0x%lx, name = %M \"%s\"\n", (unsigned long)n,
                                                        n->name, n->val.text);
        break;
      case CHARACTER :
        PRDBG("printnode : Node = 0x%lx, name = %M \"%s\"\n", (unsigned long)n,
                                           n->name, n->yytext);
        break;
      case INTEGER :
        PRDBG("printnode : Node = 0x%lx, name = %M \"%s\"\n", (unsigned long)n,
                                                        n->name, n->yytext);
        break;
      case FLOAT :
        PRDBG("printnode : Node = 0x%lx, name = %M \"%s\"\n", (unsigned long)n,
                                                        n->name, n->yytext);
        break;
      case QSTRING :
        PRDBG("printnode : Node = 0x%lx, name = %M \"%s\"\n", (unsigned long)n,
                                                        n->name, n->yytext);
        break;
      default :
        PRDBG("printnode : Node = 0x%lx, name = %M\n", (unsigned long)n,
                                                        n->name);
        break;
    }
#endif
  if (n != NULL) {
    switch(n->name) {
      case '!' :
        PR0("!"); PRRIGHT(n);
        break;

      case '#' :
        PRLEFT(n); PRRIGHT(n);
        break;

      case '%' :
        PRLEFT(n); PR0("%%"); PRRIGHT(n);
        break;

      case '&' :
        PRLEFT(n); PR0("&"); PRRIGHT(n);
        break;

      case '*' :
        PRLEFT(n); PR0(" *"); PRRIGHT(n);
        break;

      case '+' :
        PRLEFT(n); PR0("+"); PRRIGHT(n);
        break;

      case ',' :
        PRLEFT(n); PR0(", "); PRRIGHT(n);
        break;

      case '-' :
        PRLEFT(n); PR0("-"); PRRIGHT(n);
        break;

      case '.' :
        PRLEFT(n); PR0("."); PRRIGHT(n);
        break;

      case '/' :
        PRLEFT(n); PR0("/"); PRRIGHT(n);
        break;

      case ':' :
        PRLEFT(n); PR0(":"); PRRIGHT(n);
        break;

      case '=' :
        PRLEFT(n); PR0(" = "); PRRIGHT(n);
        break;

      case '?' :
        PRLEFT(n); PR0("?"); PRRIGHT(n);
        break;

      case '^' :
        PRLEFT(n); PR0("^"); PRRIGHT(n);
        break;

      case '|' :
        PRLEFT(n); PR0("|"); PRRIGHT(n);
        break;

      case '~' :
        PR0("~"); PRRIGHT(n);
        break;

      case ADDROF :
        PR0("&"); PRRIGHT(n);
        break;

      case ANDAND :
        PRLEFT(n); PR0("&&"); PRRIGHT(n);
        break;

      case ANDEQ :
        PRLEFT(n); PR0("&="); PRRIGHT(n);
        break;

      case ARRAY :
        PRLEFT(n); PR0("["); PRRIGHT(n); PR0("]");
        break;

      case ARRAYELT :
        PRLEFT(n); PR0("["); PRRIGHT(n); PR0("]");
        break;

      case AUTO :
        PR0("auto "); PRLEFT(n);
        break;

      case BALPAR :
        PR0("("); PRLEFT(n); PRRIGHT(n); PR0(")");
        break;

      case BLOC :
        PR0("{"); PRLEFT(n); PRRIGHT(n); PR0("}");
        break;

      case BREAK :
        SYNC(n); PR0("break ;"); NL;
        break;

      case CALL :
        PRLEFT(n); PR0("("); PRRIGHT(n); PR0(")");
        break;

      case CASE :
        PR0("case "); PRLEFT(n); PR0(" :"); NL; PRRIGHT(n);
        break;

      case CAST :
        PR0("("); PRLEFT(n); PR0(")"); PRRIGHT(n);
        break;

      case CHAR :
        PR0("char ");
        break;

      case CHARACTER :
        PR1("%s", n->yytext);
        break;

      case COMPOUND :
        PR0("{"); NL; PRLEFT(n); PRRIGHT(n); NL; SYNC(n); PR0("}"); NL;
        break;

      case CONST :
        PR0("const "); PRLEFT(n);
        break;

      case CONTINUE :
        SYNC(n); PR0("continue ;");
        break;

      case DECLARATION :
        PRLEFT(n); PRRIGHT(n); SYNC(n); PR0(";"); NL;
        break;

      case DECR :
        PRLEFT(n); PR0("--"); PRRIGHT(n);
        break;

      case DEFAULT :
        PR0("default :"); NL; PRLEFT(n);
        break;

      case DEREF :
        PR0(" *"); PRLEFT(n); PRRIGHT(n);
        break;

      case DIVEQ :
        PRLEFT(n); PR0(" /= "); PRRIGHT(n);
        break;

      case DO :
        PR0("do"); NL; PRLEFT(n); NL;
        PR0("while ("); PRRIGHT(n); PR0(")"); SYNC(n); PR0(";"); NL;
        break;

      case DOUBLE :
        PR0("double ");
        break;

      case ELLIPSIS :
        PR0("...");
        break;

      case ELSE :
        PRLEFT(n); NL; PR0("else"); NL; PRRIGHT(n);
        break;

      case ENUM :
        PR0("enum ");
        /* enum id ? */
        if (n->left != NULL) {
          PRLEFT(n); PR0(" ");
        }
        /* enum { ... } ? */
        if (n->right != NULL) {
         PR0("{"); PRRIGHT(n); PR0("}");
        }
        break;

      case ENUMERATION :
        PR1("%s ", n->val.text);
        break;

      case EQ :
        PRLEFT(n); PR0("=="); PRRIGHT(n);
        break;

      case EREQ :
        PRLEFT(n); PR0("^="); PRRIGHT(n);
        break;

      case EXTERN :
        PR0("extern "); PRLEFT(n);
        break;

      case FLOAT :
        PR0("float ");
        break;

      case FOR :
        /* for ( init; */
        PR0("for ("); PRLEFT(n); PR0("; ");
        /* test; */
        nn = n->right;
        PRLEFT(nn); SYNC(n); PR0("; ");
        /* next) stmt */
        nn = nn->right;
        PRLEFT(nn); PR0(")"); NL; PRRIGHT(nn); 
        break;

      case FUNCDECL :
        PRLEFT(n); PR0("("); PRRIGHT(n); PR0(")");
        break;

      case FUNCTION :
        /* DeclarationSpecifier */
        PRLEFT(n);
        /* Declarator DeclarationList */
        nn = n->right->left;
        PR0(" "); PRLEFT(nn); NL; PRRIGHT(nn); NL;
        /* CompoundStmt */
        nn = n->right;
        PRRIGHT(nn);
        break;

      case GE :
        PRLEFT(n); PR0(">="); PRRIGHT(n);
        break;

      case GOTO :
        PR0("goto "); PRLEFT(n); SYNC(n); PR0(";"); NL;
        break;

      case GT :
        PRLEFT(n); PR0(">"); PRRIGHT(n);
        break;

      case IF :
        PR0("if ("); PRLEFT(n); SYNC(n); PR0(")"); NL; PRRIGHT(n);
        break;

      case INCR :
        PRLEFT(n); PR0("++"); PRRIGHT(n);
        break;

      case INT :
        PR0("int ");
        break;

      case INTEGER :
        PR1("%s", n->yytext);
        break;

      case LARROW :
        PRLEFT(n); PR0("<-"); PRRIGHT(n);
        break;

      case LE :
        PRLEFT(n); PR0("<="); PRRIGHT(n);
        break;

      case LONG :
        PR0("long ");
        break;

      case LS :
        PRLEFT(n); PR0("<<"); PRRIGHT(n);
        break;

      case LSEQ :
        PRLEFT(n); PR0(" <<= "); PRRIGHT(n);
        break;

      case LT :
        PRLEFT(n); PR0("<"); PRRIGHT(n);
        break;

      case MINUSEQ :
        PRLEFT(n); PR0(" -= "); PRRIGHT(n);
        break;

      case MODEQ :
        PRLEFT(n); PR0(" %= "); PRRIGHT(n);
        break;

      case MULEQ :
        PRLEFT(n); PR0(" *= "); PRRIGHT(n);
        break;

      case NAME :
        if ((s = LOOKUP((char *)n->val.text)) != NULL) {
          if (GET_RENAME(s) == NULL) {
#ifdef DEBUG
            PRDBG("printnode : rename field is equal to NULL\n");
#endif
            PR1("%s ", GET_SNAME(s));
          }
          else {
            if ((int)GET_RENAME(s) == RENAME) {
#ifdef DEBUG
              PRDBG("printnode : rename field is equal to RENAME\n");
#endif
              SET_RENAME(s, rename_func(GET_SNAME(s)));
              PR1("%s ", GET_RENAME(s));
            }
            else {
#ifdef DEBUG
              PRDBG("printnode : rename field is equal to \"%s\"\n", 
                                                                 GET_RENAME(s));
#endif
              PR1("%s ", GET_RENAME(s));
            }
          }
        }
        else
          if (!isinternalname(n->val.text)) {
            PR1("%s ", n->val.text);
          }
        break;

      case NE :
        PRLEFT(n); PR0(" != "); PRRIGHT(n);
        break;

      case NULLINST :
        SYNC(n);
        PR0(";"); NL;
        break;

      case OREQ :
        PRLEFT(n); PR0(" |= "); PRRIGHT(n);
        break;

      case OROR :
        PRLEFT(n); PR0("||"); PRRIGHT(n);
        break;

      case PLUSEQ :
        PRLEFT(n); PR0(" += "); PRRIGHT(n);
        break;

      case QSTRING :
        PR1("%s ", n->val.text);
        break;

      case RARROW :
        PRLEFT(n); PR0("->"); PRRIGHT(n);
        break;

      case REAL :
        PR1("%s", n->yytext);
        break;

      case REGISTER :
        PR0("register "); PRLEFT(n);
        break;

      case RETURN :
        PRLEFT(n); PR0("return "); PRRIGHT(n); SYNC(n); PR0(";"); NL;
        break;

      case RS :
        PRLEFT(n); PR0(">>"); PRRIGHT(n);
        break;

      case RSEQ :
        PRLEFT(n); PR0(" >>= "); PRRIGHT(n);
        break;

      case SHORT :
        PRLEFT(n); PR0("short "); PRRIGHT(n);
        break;

      case SIGNED :
        PRLEFT(n); PR0("signed "); PRRIGHT(n);
        break;

      case SIZEOF :
        PRLEFT(n); PR0("sizeof "); PRRIGHT(n);
        break;

      case STATIC :
        PR0("static "); PRLEFT(n);
        break;

      case STREF :
        PRLEFT(n); PR0("->"); PRRIGHT(n);
        break;

     case STRUCT :
        PR0("struct ");
        /* struct id ? */
        if (n->left != NULL) {
          PRLEFT(n); PR0(" ");
        }
        /* struct { ... } ? */
        if (n->right != NULL) {
         PR0("{"); NL; PRRIGHT(n); NL; PR0("} ");
        }
        break;

      case SWITCH :
        PR0("switch ("); PRLEFT(n); SYNC(n); PR0(")"); NL; PRRIGHT(n);
        break;

      case TYPEDEF :
        PR0("typedef "); PRLEFT(n);
        break;

     case UNION :
        PR0("union ");
        /* union id ? */
        if (n->left != NULL) {
          PRLEFT(n); PR0(" ");
        }
        /* union { ... } ? */
        if (n->right != NULL) {
         PR0("{"); NL; PRRIGHT(n); NL; PR0("}");
        }
        break;

      case UNSIGNED :
        PR0("unsigned ");
        break;

      case USRTYPID :
        PR1("%s ", n->val.text);
        PRLEFT(n); PRRIGHT(n);
        break;

      case VOID :
        PR0("void ");
        break;

      case VOLATILE :
        PR0("volatile "); PRLEFT(n);
        break;

      case WHILE :
        PR0("while ("); PRLEFT(n); SYNC(n); PR0(")"); NL; PRRIGHT(n);
        break;

      default :
#ifdef DEBUG
        PRDBG("printnode : n->name = %M\n", n->name);
#endif
        INT_ERROR("printnode");
        break;
      }
  }
}


#ifdef __STDC__
void texprintnode(FILE *fd, Node *n)
#else
texprintnode(fd, n)
FILE *fd;
Node *n;
#endif
{
  Node *nn;
  int i;
  Symbol *s;

#ifdef DEBUG
  if (n != NULL)
    switch(n->name) {
      case NAME :
      case USRTYPID :
        PRDBG("texprintnode : Node = 0x%lx, name = %M \"%T\"\n", (unsigned long)n,
                                                        n->name, n->val.text);
        break;
      case CHARACTER :
        PRDBG("texprintnode : Node = 0x%lx, name = %M \"%T\"\n", (unsigned long)n,
                                           n->name, n->yytext);
        break;
      case INTEGER :
        PRDBG("texprintnode : Node = 0x%lx, name = %M \"%T\"\n", (unsigned long)n,
                                                        n->name, n->yytext);
        break;
      case FLOAT :
        PRDBG("texprintnode : Node = 0x%lx, name = %M \"%T\"\n", (unsigned long)n,
                                                        n->name, n->yytext);
        break;
      case QSTRING :
        PRDBG("texprintnode : Node = 0x%lx, name = %M \"%T\"\n", (unsigned long)n,
                                                        n->name, n->yytext);
        break;
      default :
        PRDBG("texprintnode : Node = 0x%lx, name = %M\n", (unsigned long)n,
                                                        n->name);
        break;
    }
#endif
  if (n != NULL) {
    switch(n->name) {
      case '!' :
        TEX_PR0("!"); TEX_PRRIGHT(n);
        break;

      case '#' :
        TEX_PRLEFT(n); TEX_PRRIGHT(n);
        break;

      case '%' :
        TEX_PRLEFT(n); TEX_PR0("%"); TEX_PRRIGHT(n);
        break;

      case '&' :
        TEX_PRLEFT(n); TEX_PR0("&"); TEX_PRRIGHT(n);
        break;

      case '*' :
        TEX_PRLEFT(n); TEX_PR0(" *"); TEX_PRRIGHT(n);
        break;

      case '+' :
        TEX_PRLEFT(n); TEX_PR0("+"); TEX_PRRIGHT(n);
        break;

      case ',' :
        TEX_PRLEFT(n); TEX_PR0(", "); TEX_PRRIGHT(n);
        break;

      case '-' :
        TEX_PRLEFT(n); TEX_PR0("-"); TEX_PRRIGHT(n);
        break;

      case '.' :
        TEX_PRLEFT(n); TEX_PR0("."); TEX_PRRIGHT(n);
        break;

      case '/' :
        TEX_PRLEFT(n); TEX_PR0("/"); TEX_PRRIGHT(n);
        break;

      case ':' :
        TEX_PRLEFT(n); TEX_PR0(":"); TEX_PRRIGHT(n);
        break;

      case '=' :
        TEX_PRLEFT(n); TEX_PR0(" = "); TEX_PRRIGHT(n);
        break;

      case '?' :
        TEX_PRLEFT(n); TEX_PR0("?"); TEX_PRRIGHT(n);
        break;

      case '^' :
        TEX_PRLEFT(n); TEX_PR0("^"); TEX_PRRIGHT(n);
        break;

      case '|' :
        TEX_PRLEFT(n); TEX_PR0("|"); TEX_PRRIGHT(n);
        break;

      case '~' :
        TEX_PR0("~"); TEX_PRRIGHT(n);
        break;

      case ADDROF :
        TEX_PR0("&"); TEX_PRRIGHT(n);
        break;

      case ANDAND :
        TEX_PRLEFT(n); TEX_PR0("&&"); TEX_PRRIGHT(n);
        break;

      case ANDEQ :
        TEX_PRLEFT(n); TEX_PR0("&="); TEX_PRRIGHT(n);
        break;

      case ARRAY :
        TEX_PRLEFT(n); TEX_PR0("["); TEX_PRRIGHT(n); TEX_PR0("]");
        break;

      case ARRAYELT :
        TEX_PRLEFT(n); TEX_PR0("["); TEX_PRRIGHT(n); TEX_PR0("]");
        break;

      case AUTO :
        TEX_PR0("auto "); TEX_PRLEFT(n);
        break;

      case BALPAR :
        TEX_PR0("("); TEX_PRLEFT(n); TEX_PRRIGHT(n); TEX_PR0(")");
        break;

      case BLOC :
        TEX_PR0("{"); TEX_PRLEFT(n); TEX_PRRIGHT(n); TEX_PR0("}");
        break;

      case BREAK :
        TEX_SYNC(n); TEX_PR0("break ;"); TEX_NL;
        break;

      case CALL :
        TEX_PRLEFT(n); TEX_PR0("("); TEX_PRRIGHT(n); TEX_PR0(")");
        break;

      case CASE :
        TEX_PR0("case "); TEX_PRLEFT(n); TEX_PR0(" :"); TEX_NL; TEX_PRRIGHT(n);
        break;

      case CAST :
        TEX_PR0("("); TEX_PRLEFT(n); TEX_PR0(")"); TEX_PRRIGHT(n);
        break;

      case CHAR :
        TEX_PR0("char ");
        break;

      case CHARACTER :
        TEX_PR1("%T", n->yytext);
        break;

      case COMPOUND :
        TEX_PR0("{"); TEX_NL; TEX_PRLEFT(n); TEX_PRRIGHT(n); TEX_NL; TEX_SYNC(n); TEX_PR0("}"); TEX_NL;
        break;

      case CONST :
        TEX_PR0("const "); TEX_PRLEFT(n);
        break;

      case CONTINUE :
        TEX_SYNC(n); TEX_PR0("continue ;");
        break;

      case DECLARATION :
        TEX_PRLEFT(n); TEX_PRRIGHT(n); TEX_SYNC(n); TEX_PR0(";"); TEX_NL;
        break;

      case DECR :
        TEX_PRLEFT(n); TEX_PR0("--"); TEX_PRRIGHT(n);
        break;

      case DEFAULT :
        TEX_PR0("default :"); TEX_NL; TEX_PRLEFT(n);
        break;

      case DEREF :
        TEX_PR0("*"); TEX_PRLEFT(n); TEX_PRRIGHT(n);
        break;

      case DIVEQ :
        TEX_PRLEFT(n); TEX_PR0(" /= "); TEX_PRRIGHT(n);
        break;

      case DO :
        TEX_PR0("do"); TEX_NL; TEX_PRLEFT(n); TEX_NL;
        TEX_PR0("while ("); TEX_PRRIGHT(n); TEX_PR0(")"); TEX_SYNC(n); TEX_PR0(";"); TEX_NL;
        break;

      case DOUBLE :
        TEX_PR0("double ");
        break;

      case ELLIPSIS :
        TEX_PR0("...");
        break;

      case ELSE :
        TEX_PRLEFT(n); TEX_NL; TEX_PR0("else"); TEX_NL; TEX_PRRIGHT(n);
        break;

      case ENUM :
        TEX_PR0("enum ");
        /* enum id ? */
        if (n->left != NULL) {
          TEX_PRLEFT(n); TEX_PR0(" ");
        }
        /* enum { ... } ? */
        if (n->right != NULL) {
         TEX_PR0("{"); TEX_PRRIGHT(n); TEX_PR0("}");
        }
        break;

      case ENUMERATION :
        TEX_PR1("%T ", n->val.text);
        break;

      case EQ :
        TEX_PRLEFT(n); TEX_PR0("=="); TEX_PRRIGHT(n);
        break;

      case EREQ :
        TEX_PRLEFT(n); TEX_PR0("^="); TEX_PRRIGHT(n);
        break;

      case EXTERN :
        TEX_PR0("extern "); TEX_PRLEFT(n);
        break;

      case FLOAT :
        TEX_PR0("float ");
        break;

      case FOR :
        /* for ( init; */
        TEX_PR0("for ("); TEX_PRLEFT(n); TEX_PR0("; ");
        /* test; */
        nn = n->right;
        TEX_PRLEFT(nn); TEX_SYNC(n); TEX_PR0("; ");
        /* next) stmt */
        nn = nn->right;
        TEX_PRLEFT(nn); TEX_PR0(")"); TEX_NL; TEX_PRRIGHT(nn); 
        break;

      case FUNCDECL :
        TEX_PRLEFT(n); TEX_PR0("("); TEX_PRRIGHT(n); TEX_PR0(")");
        break;

      case FUNCTION :
        /* DeclarationSpecifier */
        TEX_PRLEFT(n);
        /* Declarator DeclaratioTEX_NList */
        nn = n->right->left;
        TEX_PR0(" "); TEX_PRLEFT(nn); TEX_NL; TEX_PRRIGHT(nn); TEX_NL;
        /* CompoundStmt */
        nn = n->right;
        TEX_PRRIGHT(nn);
        break;

      case GE :
        TEX_PRLEFT(n); TEX_PR0(">="); TEX_PRRIGHT(n);
        break;

      case GOTO :
        TEX_PR0("goto "); TEX_PRLEFT(n); TEX_SYNC(n); TEX_PR0(";"); TEX_NL;
        break;

      case GT :
        TEX_PRLEFT(n); TEX_PR0(">"); TEX_PRRIGHT(n);
        break;

      case IF :
        TEX_PR0("if ("); TEX_PRLEFT(n); TEX_SYNC(n); TEX_PR0(")"); TEX_NL; TEX_PRRIGHT(n);
        break;

      case INCR :
        TEX_PRLEFT(n); TEX_PR0("++"); TEX_PRRIGHT(n);
        break;

      case INT :
        TEX_PR0("int ");
        break;

      case INTEGER :
        TEX_PR1("%T", n->yytext);
        break;

      case LARROW :
        TEX_PRLEFT(n); TEX_PR0("<-"); TEX_PRRIGHT(n);
        break;

      case LE :
        TEX_PRLEFT(n); TEX_PR0("<="); TEX_PRRIGHT(n);
        break;

      case LONG :
        TEX_PR0("long ");
        break;

      case LS :
        TEX_PRLEFT(n); TEX_PR0("<<"); TEX_PRRIGHT(n);
        break;

      case LSEQ :
        TEX_PRLEFT(n); TEX_PR0(" <<= "); TEX_PRRIGHT(n);
        break;

      case LT :
        TEX_PRLEFT(n); TEX_PR0("<"); TEX_PRRIGHT(n);
        break;

      case MINUSEQ :
        TEX_PRLEFT(n); TEX_PR0(" -= "); TEX_PRRIGHT(n);
        break;

      case MODEQ :
        TEX_PRLEFT(n); TEX_PR0(" %= "); TEX_PRRIGHT(n);
        break;

      case MULEQ :
        TEX_PRLEFT(n); TEX_PR0(" *= "); TEX_PRRIGHT(n);
        break;

      case NAME :
        if ((s = LOOKUP((char *)n->val.text)) != NULL) {
          if (GET_RENAME(s) == NULL) {
#ifdef DEBUG
            PRDBG("texprintnode : rename field is equal to NULL\n");
#endif
            TEX_PR1("%T ", GET_SNAME(s));
          }
          else {
            if ((int)GET_RENAME(s) == RENAME) {
#ifdef DEBUG
              PRDBG("texprintnode : rename field is equal to RENAME\n");
#endif
              SET_RENAME(s, rename_func(GET_SNAME(s)));
              TEX_PR1("%T ", GET_RENAME(s));
            }
            else {
#ifdef DEBUG
              PRDBG("texprintnode : rename field is equal to \"%T\"\n", 
                                                                 GET_RENAME(s));
#endif
              TEX_PR1("%T ", GET_RENAME(s));
            }
          }
        }
        else
          if (!isinternalname(n->val.text)) {
            TEX_PR1("%T ", n->val.text);
          }
        break;

      case NE :
        TEX_PRLEFT(n); TEX_PR0(" != "); TEX_PRRIGHT(n);
        break;

      case NULLINST :
        TEX_SYNC(n);
        TEX_PR0(";"); TEX_NL;
        break;

      case OREQ :
        TEX_PRLEFT(n); TEX_PR0(" |= "); TEX_PRRIGHT(n);
        break;

      case OROR :
        TEX_PRLEFT(n); TEX_PR0("||"); TEX_PRRIGHT(n);
        break;

      case PLUSEQ :
        TEX_PRLEFT(n); TEX_PR0(" += "); TEX_PRRIGHT(n);
        break;

      case QSTRING :
        TEX_PR1("%T ", n->val.text);
        break;

      case RARROW :
        TEX_PRLEFT(n); TEX_PR0("->"); TEX_PRRIGHT(n);
        break;

      case REAL :
        TEX_PR1("%T", n->yytext);
        break;

      case REGISTER :
        TEX_PR0("register "); TEX_PRLEFT(n);
        break;

      case RETURN :
        TEX_PRLEFT(n); TEX_PR0("return "); TEX_PRRIGHT(n); TEX_SYNC(n); TEX_PR0(";"); TEX_NL;
        break;

      case RS :
        TEX_PRLEFT(n); TEX_PR0(">>"); TEX_PRRIGHT(n);
        break;

      case RSEQ :
        TEX_PRLEFT(n); TEX_PR0(" >>= "); TEX_PRRIGHT(n);
        break;

      case SHORT :
        TEX_PRLEFT(n); TEX_PR0("short "); TEX_PRRIGHT(n);
        break;

      case SIGNED :
        TEX_PRLEFT(n); TEX_PR0("signed "); TEX_PRRIGHT(n);
        break;

      case SIZEOF :
        TEX_PRLEFT(n); TEX_PR0("sizeof "); TEX_PRRIGHT(n);
        break;

      case STATIC :
        TEX_PR0("static "); TEX_PRLEFT(n);
        break;

      case STREF :
        TEX_PRLEFT(n); TEX_PR0("->"); TEX_PRRIGHT(n);
        break;

     case STRUCT :
        TEX_PR0("struct ");
        /* struct id ? */
        if (n->left != NULL) {
          TEX_PRLEFT(n); TEX_PR0(" ");
        }
        /* struct { ... } ? */
        if (n->right != NULL) {
         TEX_PR0("{"); TEX_NL; TEX_PRRIGHT(n); TEX_NL; TEX_PR0("} ");
        }
        break;

      case SWITCH :
        TEX_PR0("switch ("); TEX_PRLEFT(n); TEX_SYNC(n); TEX_PR0(")"); TEX_NL; TEX_PRRIGHT(n);
        break;

      case TYPEDEF :
        TEX_PR0("typedef "); TEX_PRLEFT(n);
        break;

     case UNION :
        TEX_PR0("union ");
        /* union id ? */
        if (n->left != NULL) {
          TEX_PRLEFT(n); TEX_PR0(" ");
        }
        /* union { ... } ? */
        if (n->right != NULL) {
         TEX_PR0("{"); TEX_NL; TEX_PRRIGHT(n); TEX_NL; TEX_PR0("}");
        }
        break;

      case UNSIGNED :
        TEX_PR0("unsigned ");
        break;

      case USRTYPID :
        TEX_PR1("%T ", n->val.text);
        TEX_PRLEFT(n); TEX_PRRIGHT(n);
        break;

      case VOID :
        TEX_PR0("void ");
        break;

      case VOLATILE :
        TEX_PR0("volatile "); TEX_PRLEFT(n);
        break;

      case WHILE :
        TEX_PR0("while ("); TEX_PRLEFT(n); TEX_SYNC(n); TEX_PR0(")"); TEX_NL; TEX_PRRIGHT(n);
        break;

      default :
#ifdef DEBUG
        PRDBG("texprintnode : n->name = %M\n", n->name);
#endif
        INT_ERROR("texprintnode");
        break;
    }
  }
}


#define INTERNAL_LIMIT 10000
static unsigned long nameind = 1;


#ifdef __STDC__
char *rename_func(char *name)
#else
char *rename_func(name)
char *name;
#endif
{
  extern Node *mwname;
  char buffer[BUFSIZ], *ret;
  Symbol *s;

  /* First attempt for rename */
  sprintf(buffer, "%s_%s", mwname->val.text, name);
  if ((ret = (char *)malloc(strlen(buffer) + 1)) != NULL)
    strcpy(ret, buffer);

  /* While the new name exists in one of all symbol tables rename once more */
  while (LOOKUP(ret) != NULL || LOOKUPAGG(ret) != NULL) {
    if (nameind == 0)
      INT_ERROR("rename");
    sprintf(buffer, "%s%4.4lu_%s", mwname->val.text, nameind, name);
    if ((ret = (char *)malloc(strlen(buffer) + 1)) != NULL)
      strcpy(ret, buffer);
    nameind = (nameind +1) % INTERNAL_LIMIT;
  }
#ifdef DEBUG
  PRDBG("rename : %s -> %s\n", name, ret);
#endif
  return ret;
}
