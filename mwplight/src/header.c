/*
 * header.c for megawave, section mwplight
 *
 * analyse header statements and fill the header tree
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2006)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mwplight-defs.h"

#include "io.h"
#include "tree.h"

#include "header.h"

#define MSG_ERROR_HEADER_SYNTAX_SSCANF \
     "(sscanf) t_header statement \"%s\" does not follow the syntax \"<name> = {<value>}\"."
#define MSG_ERROR_HEADER_SYNTAX_BRACES \
     "(v='%s') t_header statement \"%s\" does not follow the syntax \"<name> = {<value>}\"."
#define MSG_ERROR_INVALID_INDEX \
     "Invalid n-th index n=%d"
#define MSG_ERROR_USAGE_OPTIONAL_DUPLICATE \
     "Invalid '{' in the usage : no more than one optional arguments list is allowed"
#define MSG_ERROR_USAGE_OPTIONAL_NESTED \
     "Invalid '{' in the usage : already inside an optional arguments list"
#define MSG_ERROR_USAGE_OPTIONAL_OUTSIDE \
     "Invalid '}' in the usage : not inside an optional arguments list"

#define MSG_ERROR_UNKNOWN_STATEMENT \
     "Unimplemented \"%s\" statement in the module's header"

#define MSG_ERROR_USAGE_LEFT_QUOTES \
    "Invalid usage specification \"%s\" : left string delimiter \" not found"
#define MSG_ERROR_USAGE_RIGHT_QUOTES \
    "Invalid usage specification \"%s\" : right string delimiter \" not found"
#define MSG_ERROR_ARROW \
    "Invalid argument \"%s\" : no arrow -> or <- found"
#define MSG_ERROR_CID \
    "C_id not found in \"%s\""
#define MSG_ERROR_CID_FIELD \
    "Invalid field following C_id=\"%s\" in \"%s\"\nExpecting void or optional interval [Min, Max]."
#define MSG_ERROR_CID_FIELD2 \
    "Invalid field \"%s\" following interval (%s, %s) of C_id=\"%s\" in \"%s\"."
#define MSG_ERROR_DEFAULT_FIELD_BRACKET \
    "Invalid default value field in \"%s\".\nThis string does not end with ']'."
#define MSG_ERROR_DEFAULT_FIELD_SYNTAX \
    "Invalid default value field in \"%s]\".\nThis string does not follow the syntax [H_id=Val]."
#define MSG_ERROR_OPTION_THIRD_LETTER \
    "Invalid option field in \"%s\".\nThird letter : expecting \"'\" character instead of \"%c\".\nPossible error : user's options of more than one character are not allowed."
#define MSG_ERROR_OPTION_ARG_LIST \
    "Invalid option occurrence in \"%s\".\nOptions are not allowed inside an optional argument list."
#define MSG_ERROR_OPTION_FOURTH_LETTER \
    "Invalid option field in \"%s\".\nFourth letter : expecting \":\" or end of char instead of \"%c\"."
#define MSG_ERROR_INVALID_FIELD \
    "Invalid variable argument field (...) in \"%s\"."
#define MSG_ERROR_INVALID_VARIABLE \
    "Invalid variable argument occurrence in \"%s\".\nVariable arguments are not allowed inside an optional argument list."
#define MSG_ERROR_UNUSED_ARGUMENT \
    "Invalid notused occurrence in \"%s\".\nUnused arguments are not allowed inside an optional argument list."
#define MSG_ERROR_DEFAULT_VALUES \
    "Invalid default value occurrence in \"%s\".\nDefault values are not allowed with needed arguments."
#define MSG_ERROR_DUPLICATE_NAME \
    "Duplicate name statement (previous name is \"%s\") !"
#define MSG_ERROR_LIMIT \
    "Name \"%s\" exceeds limit of %d car."
#define MSG_ERROR_NAME \
    "Invalid name. The filename imposes the name to be \"%s\""
#define MSG_ERROR_DUPLICATE_AUTHOR \
    "Duplicate author statement (previous author is \"%s\") !"
#define MSG_ERROR_AUTHOR_QUOTES \
    "Author statement field must be enclosed by quotation marks"
#define MSG_ERROR_AUTHOR_LIMIT \
    "Author statement field \"%s\" exceeds limit of %d car."
#define MSG_ERROR_DUPLICATE_VERSION \
    "Duplicate version statement (previous version is \"%s\") !"
#define MSG_ERROR_VERSION_QUOTES \
    "Version statement field must be enclosed by quotation marks"
#define MSG_ERROR_VERSION_LIMIT \
    "Version statement field \"%s\" exceeds limit of %d car."
#define MSG_ERROR_DUPLICATE_FUNCTION \
    "Duplicate function statement (previous function is \"%s\") !"
#define MSG_ERROR_FUNCTION_QUOTES \
    "Function statement field must be enclosed by quotation marks"
#define MSG_ERROR_FUNCTION_LIMIT \
    "Function statement field \"%s\" exceeds limit of %d car."
#define MSG_ERROR_DUPLICATE_LABO \
    "Duplicate labo statement (previous labo is \"%s\") !"
#define MSG_ERROR_LABO_QUOTES \
    "Labo statement field must be enclosed by quotation marks"
#define MSG_ERROR_LABO_LIMIT \
    "Labo statement field \"%s\" exceeds limit of %d car."
#define MSG_ERROR_DUPLICATE_GROUP \
    "Duplicate group statement (previous group is \"%s\") !"
#define MSG_ERROR_GROUP_QUOTES \
    "Group statement field must be enclosed by quotation marks"
#define MSG_ERROR_GROUP_LIMIT \
    "Group statement field \"%s\" exceeds limit of %d car."
#define MSG_ERROR_GROUP \
    "Invalid group. The directory hierarchy imposes the group to be \"%s\""
#define MSG_ERROR_COMMENT_LIMIT \
    "Comment's string \"%s\" in usage exceeds maximum size length of %d char."
#define MSG_ERROR_MEMORY \
    "Not enough memory for a new usage statement"

#define MSG_DEBUG_GETUSAGESPEC \
     "[GetUsageSpec] arg='%s' str='%s'"
#define MSG_DEBUG_GETARGUSAGESPEC \
     "[GetArgUsageSpec] left='%s' right='%s' rw=%d"
#define MSG_DEBUG_ANALYSEHEADERSTATEMENT \
     "[AnalyseHeaderStatement] argclass='%s' value='%s'"

/*
 * get from sentence <s>
 * a header statement of the form "<name> = { <value> }"
 */
void GetHeaderStatement(char * s, char * name, char * value)
{
     char v[STRSIZE];

     * name  = '\0';
     * v     = '\0';
     * value = '\0';

     if ((sscanf(s, "%[a-z]=%[^\n]",   name, v) != 2) && \
         (sscanf(s, "%[a-z] =%[^\n]",  name, v) != 2) && \
         (sscanf(s, "%[a-z]= %[^\n]",  name, v) != 2) && \
         (sscanf(s, "%[a-z] = %[^\n]", name, v) != 2))
          error(MSG_ERROR_HEADER_SYNTAX_SSCANF, s);

     if (removebraces(v, value) != 1)
          error(MSG_ERROR_HEADER_SYNTAX_BRACES, v, s);
}

/*
 * get from usage value <s>
 * the n-th usage specification arg_n "string_n"
 * and put it in <arg>, <str>.
 * return 1 if found, 0 elsewhere (no more usage spec.)
 */
static int GetUsageSpec(char * s, size_t n, \
                        /*@out@*/ char * arg, /*@out@*/ char * str)
{
     size_t i, l, i0, i1, q0, q1;
     char us[STRSIZE];

     arg[0] = '\0';
     str[0] = '\0';

     /*
      * n-th spec. begins
      * after the (n-1)-th '", ' or '" , ' or '}, ' or '} , '
      * the two last cases corresponding to an usage spec.
      * following an optional argument list.
      */
     i0 = 0;
     for (i = 1; i < n; i++)
     {
     notasep0:
          while ((s[i0] != '\0') && (s[i0] != ','))
               i0++;
          if ((i0<2) || (s[i0]=='\0'))
               return 0;
          if ((s[i0-1] != '"') && (s[i0-1] != '}') &&  \
              (((s[i0-1] != ' ') || ((s[i0-2]!='"') && \
                                     (s[i0-2]!='}')))))
          {
               i0++;
               goto notasep0;
          }
          i0++;
          if (s[i0] == '\0')
               return 0;
     }
     while ((s[i0] != '\0') && (s[i0] == ' '))
          i0++;
     if (s[i0] == '\0')
          return 0;

     if (s[i0] == '{')
     {
          /* open optional arguments list */
          if (inside_optionarg < 0)
               error(MSG_ERROR_USAGE_OPTIONAL_DUPLICATE);
          if (inside_optionarg >= 1)
               error(MSG_ERROR_USAGE_OPTIONAL_NESTED);
          inside_optionarg = 1;
          i0++;
     }

     /* seek the right ', ' or end of <s> */
     i1 = i0;
notasep1:
     while ((s[i1] != '\0') && (s[i1] != ',') && (s[i1] != '}'))
          i1++;
     if ((s[i1-1] != '"') && (((s[i1-1] != ' ') || (s[i1-2] != '"'))))
     {
          i1++;
          goto notasep1;
     }
     if (s[i1] == '}')
     /* close optional arguments list */
     {
          if (inside_optionarg != 1)
               error(MSG_ERROR_USAGE_OPTIONAL_OUTSIDE);
          /* this n-th arg is the last optional argument */
          inside_optionarg = 2;
     }
     i1--;

     /* now the n-th usage spec. is in s[i0]...s[i1] */
     l = i1 - i0 + 1;
     strncpy(us, &s[i0], l);
     us[l] = '\0';

     /* seek the left '"' */
     q0 = 0;
     while ((q0 < l-1) && (us[q0] != '"'))
          q0++;
     q0++;
     if (q0 >= l-1)
       error(MSG_ERROR_USAGE_LEFT_QUOTES, us);
     q1 = q0;
     while ((q1 < l) && (us[q1] != '"'))
          q1++;
     if (us[q1] != '"')
          error(MSG_ERROR_USAGE_RIGHT_QUOTES, us);
     q1--;

     for (i = 0; i < q0 - 1; i++)
          arg[i] = us[i];
     arg[i] = '\0';
     for (i = q0; i <= q1; i++)
          str[i - q0] = us[i];
     str[i - q0] = '\0';

     removespaces(arg);

     if (debug_flag)
          debug(MSG_DEBUG_GETUSAGESPEC, arg, str);

     return 1 ;
}


/*
 * get from arg usage value <s>
 * (as returned in <arg> by GetUsageSpec())
 * the arg usage specification
 * following the syntax '<left> -> <right>' or '<left> <- <right>'.
 * return READ if the arrow is -> (input arg)
 * or WRITE if the arrow is <- (output arg).
 */
static int GetArgUsageSpec(char * s, \
                           /*@out@*/ char * left, /*@out@*/ char * right)
{
     size_t i;
     int rw;

     left[0]  = '\0';
     right[0] = '\0';

     /* seek the arrow */
     i = 1;

notanarrow:
     while ((s[i] != '\0') && (s[i] != '-') && (s[i] != '<'))
          i++;
     if ((s[i] == '\0') || (s[i + 1] == '\0'))
          error(MSG_ERROR_ARROW, s);

     if ((s[i] == '-') && (s[i + 1] == '>'))
          rw = READ;
     else
          if ((s[i] == '<') && (s[i + 1] == '-'))
               rw = WRITE;
          else
          {
               i++;
               goto notanarrow;
          }


     strncpy(left, s, i);
     left[i] = '\0';
     strcpy(right, &s[i + 2]);

     removespaces(left);
     removespaces(right);

     if (debug_flag)
          debug(MSG_DEBUG_GETARGUSAGESPEC, left, right, rw);

     return rw;
}


/*
 * analyse right part of the arg usage value
 * (as returned in <right> by GetArgUsageSpec()).
 * from <s> get <Cid> (needed) and optionally <ictype>, <min>, <max>.
 */
static void AnalyseRightArgUsage(char * s, char * Cid, int * ictype,
                          char * min, char * max)
{
     int i, j;

     i = getCid(s, Cid);
     if (i == 0) error(MSG_ERROR_CID, s);
     * ictype = getInterval(&s[i], min, max, &j);

     /* check that nothing is following */
     if (* ictype == NONE)
     {
          if (s[i] != '\0')
               error(MSG_ERROR_CID_FIELD, Cid, s);
     }
     else
     {
          if (s[i + j] != '\0')
               error(MSG_ERROR_CID_FIELD2, &s[j], min, max, Cid, s);
     }
}

/*
 * analyse the default input value following the syntax <s> = "[H_id=Val]".
 * return 1 (and fill <hid>, <val>)
 * if the input <s> follows this syntax, 0 or generate error elsewhere.
 */
static int GetDefaultInputValue(char * s, char * hid, char * val)
{
     size_t l;

     removespaces(s);
     if (s[0] != '[')
          return 0;
     l = strlen(s);
     if (s[l - 1] != ']')
          error(MSG_ERROR_DEFAULT_FIELD_BRACKET, s);
     s[l - 1] = '\0';

     if (sscanf(s, "[%[^=]=%[^\n]", hid, val) != 2)
          error(MSG_ERROR_DEFAULT_FIELD_SYNTAX, s);

     removespaces(hid);
     removespaces(val);
     /*  printf("[GetDefaultInputValue] hid=\"%s\" val=\"%s\"\n", hid, val);*/

     return 1;
}


/*
 * analyse left part of the arg usage value
 * (as returned in <left> by GetArgUsageSpec()).
 * from <s> get <atype>, <flg>, <hid>, <val>.
 */
static void AnalyseLeftArgUsage(char * s, int * atype, char * flg,
                         char * hid, char * val)
{
     char t[STRSIZE];

     /* some possibilities may be determined from the first letter */
     if (s[0] == '\'')
     {
          /* must be an option */
          if (s[2] != '\'')
               error(MSG_ERROR_OPTION_THIRD_LETTER, s, s[2]);
          if (inside_optionarg>0)
               error(MSG_ERROR_OPTION_ARG_LIST, s);

          * atype = OPTION;
          * flg = s[1];
          if ((s[3] != ':') && (s[3] != '\0'))
               error(MSG_ERROR_OPTION_FOURTH_LETTER, s, s[3]);
          if (s[3] == ':')
          {
               /* option with input value */
               strcpy(t, &s[4]);
               if (GetDefaultInputValue(t, hid, val)!=1)
                    /*
                     * option with NO default input value :
                     * remaining text is assumed to be hid only
                     */
                    strcpy(hid, t);
          }
          return;
     }

     if (s[0]=='.')
     {
          /* must be a variable argument */
          if ((s[1] != '.') || (s[2] != '.') || (s[3] != '\0'))
               error(MSG_ERROR_INVALID_FIELD, s);
          if (inside_optionarg > 0)
               error(MSG_ERROR_INVALID_VARIABLE, s);
          * atype = VARARG;
          return;
     }

     if (strcmp(s, "notused") == 0)
     {
          /* unused argument */
          if (inside_optionarg > 0)
               error(MSG_ERROR_UNUSED_ARGUMENT, s);
          * atype = NOTUSEDARG;
          return;
     }

     /*
      * now, only two possibilities :
      * needed or optional arg, the only difference being that
      * with needed arg input value is not allowed.
      */
     strcpy(t, s);
     if (GetDefaultInputValue(t, hid, val) == 1)
     {
          if (inside_optionarg <= 0)
               error(MSG_ERROR_DEFAULT_VALUES, s);
          * atype = OPTIONARG;
          return;
     }
     /*
      * argument with NO default input value :
      * remaining text is assumed to be hid only
      */
     strcpy(hid, s);
     if (inside_optionarg > 0)
          * atype = OPTIONARG;
     else
          * atype = NEEDEDARG;
}


/*
 * add name statement to the header tree
 */
static void AddNameStatement(char * value)
{

     if (H->Name[0] != '\0')
          error(MSG_ERROR_DUPLICATE_NAME, H->Name);
     if (strlen(value) >= TREESTRSIZE)
          error(MSG_ERROR_LIMIT, value, \
          TREESTRSIZE - 1);
     strcpy(H->Name, value);

     /* check whether or not this name matches the file name */
     if (strcmp(H->Name, module_name) != 0)
          error(MSG_ERROR_NAME, module_name);
}


/*
 * add author statement to the header tree
 */
static void AddAuthorStatement(char * value)
{
     char v[STRSIZE];

     if (H->Author[0] != '\0')
          error(MSG_ERROR_DUPLICATE_AUTHOR, H->Author);
     if (getenclosedstring(value, v) != 1)
          error(MSG_ERROR_AUTHOR_QUOTES);

     if (strlen(v) >= TREESTRSIZE)
          error(MSG_ERROR_AUTHOR_LIMIT, v, TREESTRSIZE - 1);

     strcpy(H->Author, v);
}

/*
 * add version statement to the header tree
 */
static void AddVersionStatement(char * value)
{
     char v[STRSIZE];

     if (H->Version[0] != '\0')
          error(MSG_ERROR_DUPLICATE_VERSION, H->Version);

     if (getenclosedstring(value, v) != 1)
          error(MSG_ERROR_VERSION_QUOTES);

     if (strlen(v) >= TREESTRSIZE)
          error(MSG_ERROR_VERSION_LIMIT, v, TREESTRSIZE - 1);

     strcpy(H->Version, v);
}


/*
 * add function statement to the header tree
 */
static void AddFunctionStatement(char * value)
{
     char v[STRSIZE];

     if (H->Function[0] != '\0')
          error(MSG_ERROR_DUPLICATE_FUNCTION, H->Function);

     if (getenclosedstring(value, v) != 1)
          error(MSG_ERROR_FUNCTION_QUOTES);

     if (strlen(v) >= TREESTRSIZE)
          error(MSG_ERROR_FUNCTION_LIMIT, v, TREESTRSIZE - 1);

     strcpy(H->Function, v);
}


/*
 * add labo statement to the header tree
 */
/* TODO: s/labo/orga/ */
static void AddLaboStatement(char * value)
{
     char v[STRSIZE];

     if (H->Labo[0] != '\0')
          error(MSG_ERROR_DUPLICATE_LABO, H->Labo);
     if (getenclosedstring(value, v) != 1)
          error(MSG_ERROR_LABO_QUOTES);

     if (strlen(v) >= TREESTRSIZE)
          error(MSG_ERROR_LABO_LIMIT, v, TREESTRSIZE - 1);

     strcpy(H->Labo, v);
}


/*
 * add group statement to the header tree
 */
/* TODO: s/group/team/ */
static void AddGroupStatement(char * value)
{
     char v[STRSIZE];

     if (H->Group[0] != '\0')
          error(MSG_ERROR_DUPLICATE_GROUP, H->Group);
     if (getenclosedstring(value, v) != 1)
          error(MSG_ERROR_GROUP_QUOTES);

     if (strlen(v) >= TREESTRSIZE)
          error(MSG_ERROR_GROUP_LIMIT, v, TREESTRSIZE - 1);

     strcpy(H->Group, v);

     /* check whether or not this group matches group_name */
     if (strcmp(H->Group, group_name) != 0)
          error(MSG_ERROR_GROUP, group_name);
}


/*
 * add usage statement to the header tree
 */
static void AddUsageStatement(char * value)
{
     size_t n;
     char arg[STRSIZE];
     char str[STRSIZE];
     char left[STRSIZE];
     char right[STRSIZE];
     t_argument * a, * a0;

     inside_optionarg = 0;

     n = 1;
     a0 = NULL;
     while (GetUsageSpec(value, n, arg, str) == 1)
     {
          if (strlen(str) >= TREESTRSIZE)
               error(MSG_ERROR_COMMENT_LIMIT, str, TREESTRSIZE-1);

          a = new_arg();
          if (a == NULL)
               error(MSG_ERROR_MEMORY);
          if (a0 == NULL)
               H->usage = a;
          else
          {
               a0->next = a;
               a->previous = a0;
          }
          a0 = a;

          /* set arg fields Cmt, IOtype */
          strcpy(a->Cmt, str);
          a->IOtype = GetArgUsageSpec(arg, left, right);

          /*
           * analyse right portion of the arg usage description.
           * set the following arg fields :
           * ICtype, C_id, Min, Max
           */
          AnalyseRightArgUsage(right, a->C_id, &(a->ICtype), a->Min, a->Max);

          /*
           * analyse left portion of the arg usage description.
           * set the following arg fields :
           * Atype, Flag, H_id, Val
           */
          AnalyseLeftArgUsage(left, &(a->Atype), &(a->Flag), a->H_id, a->Val);

          /*
           * remaining arg to set : Ctype, Vtype.
           * To be completed when the C body would be parsed.
           */

          /* Dump arg content for debug */
          if (debug_flag)
          {
               char dump[STRSIZE] = "";
               strdump_arg(dump, a);
               debug(dump);
          }

          n++;
          if (inside_optionarg == 2)
               /* does not allow new optional arguments list */
               inside_optionarg =- 1;
     }
}



/*
 * analyse the current header statement and add it to the header tree
 */
void AnalyseHeaderStatement(char * argclass, char * value)
{
     if (debug_flag)
          debug(MSG_DEBUG_ANALYSEHEADERSTATEMENT, argclass, value);

     if (strcmp(argclass, "name") == 0)
          AddNameStatement(value);
     else
          if (strcmp(argclass, "author") == 0)
               AddAuthorStatement(value);
          else
               if (strcmp(argclass, "version") == 0)
                    AddVersionStatement(value);
               else
                    if (strcmp(argclass, "function") == 0)
                         AddFunctionStatement(value);
                    else
                         if (strcmp(argclass, "labo") == 0)
                              AddLaboStatement(value);
                         else
                              if (strcmp(argclass, "group") == 0)
                                   AddGroupStatement(value);
                              else
                                   if (strcmp(argclass, "usage") == 0)
                                        AddUsageStatement(value);
                                   else
                                        error(MSG_ERROR_UNKNOWN_STATEMENT, \
                                              argclass);
}
