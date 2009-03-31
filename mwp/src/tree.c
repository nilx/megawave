/**
 * @file tree.c
 *
 * syntax tree tools for the megawave parser
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2009)
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "definitions.h"
#include "io.h"

#include "tree.h"

#define MSG_ERROR_MEMORY \
     "Not enough memory to create a new %s"
#define MSG_ERROR_NULL \
     "NULL %s"
#define MSG_ERROR_HEADER \
     "Invalid header : incorrect or missing %s"
#define MSG_ERROR_USAGE_REQUIRED \
     "Invalid usage for Cmt=\"%s\" : C_id is always required"
#define MSG_ERROR_USAGE_DUPLICATE \
     "Invalid usage for C_id=\"%s\" : duplicate use of this C variable name"
#define MSG_ERROR_USAGE_TYPE \
     "Invalid usage for C_id=\"%s\" : incorrect argument type"
#define MSG_ERROR_USAGE_IOTYPE \
     "Invalid usage for C_id=\"%s\" : incorrect I/O type"
#define MSG_ERROR_USAGE_ICTYPE \
     "Invalid usage for C_id=\"%s\" : internal inconsistency " \
     "between ICtype (not NULL) and Min (NULL)"
#define MSG_ERROR_USAGE_IC_FLAG \
     "Invalid usage for C_id=\"%s\" : interval checking " \
     "is not allowed in flag option"
#define MSG_ERROR_USAGE_IC_OUT \
     "Invalid usage for C_id=\"%s\" : interval checking " \
     "is not allowed in output"
#define MSG_ERROR_USAGE_IC_UNUSED \
     "Invalid usage for C_id=\"%s\" : interval checking " \
     "is not allowed in notused argument"
#define MSG_ERROR_USAGE_UNKNOWN \
     "Invalid usage for C_id=\"%s\" : unknown flag associated to this option"
#define MSG_ERROR_USAGE_FLAG_FORBIDDEN \
     "Invalid usage for C_id=\"%s\" : flag is allowed in option only"
#define MSG_ERROR_USAGE_HID_REQUIRED \
     "Invalid usage for C_id=\"%s\" : H_id is required " \
     "for needed and optional arguments"
#define MSG_ERROR_USAGE_HID_FORBIDDEN \
     "Invalid usage for C_id=\"%s\" : H_id is not allowed " \
     "in variable or not used arguments"
#define MSG_ERROR_USAGE_COMMENT \
     "Invalid usage for C_id=\"%s\" : incorrect comment"
#define MSG_ERROR_USAGE_DEFAULT_INPUT_FLAG \
     "Invalid usage for C_id=\"%s\" : default input value " \
     "is not allowed in flag option"
#define MSG_ERROR_USAGE_DEFAULT_INPUT_NEEDED \
     "Invalid usage for C_id=\"%s\" : default input value " \
     "is not allowed in needed argument"
#define MSG_ERROR_USAGE_DEFAULT_INPUT \
     "Invalid usage for C_id=\"%s\" : default input value requires input"
#define MSG_ERROR_USAGE_DEFAULT_INPUT_VARIABLE \
     "Invalid usage for C_id=\"%s\" : default input value " \
     "is not allowed in variable argument"
#define MSG_ERROR_USAGE_DEFAULT_INPUT_NOTUSED \
     "Invalid usage for C_id=\"%s\" : default input value " \
     "is not allowed in notused argument"
#define MSG_ERROR_USAGE_IC_MISSING \
    "Invalid usage for C_id=\"%s\" : incorrect interval checking " \
    "specification (only Min or Max specified)"
#define MSG_ERROR_USAGE_IC_INCONSISTENT \
     "Invalid usage for C_id=\"%s\" : internal inconsistency " \
     "between ICtype (NULL) and Min (not NULL)"
#define MSG_ERROR_USAGE_IC_INCORRECT \
     "Invalid usage for C_id=\"%s\" : incorrect interval checking " \
     "specification (Min >= Max)"
#define MSG_ERROR_DELETE_NULL_CINSTRUCTION \
     "Cannot delete NULL t_statement"
#define MSG_ERROR_MERGE_NULL_CINSTRUCTION \
     "Cannot merge NULL t_statement"

t_argument *new_arg(void)
{
    t_argument *arg;

    arg = (t_argument *) malloc(sizeof(t_argument));
    if (arg == NULL)
        error(MSG_ERROR_MEMORY, "t_header");
    else
    {
        arg->Atype = NONE;
        arg->IOtype = NONE;
        arg->ICtype = NONE;

        arg->Flag = '\0';
        strcpy(arg->H_id, "");
        strcpy(arg->C_id, "");
        strcpy(arg->Cmt, "");
        strcpy(arg->Val, "");
        strcpy(arg->Min, "");
        strcpy(arg->Max, "");

        arg->var = NULL;

        arg->previous = NULL;
        arg->next = NULL;
    }
    return arg;
}

t_header *new_header(void)
{
    t_header *h;

    h = (t_header *) malloc(sizeof(t_header));
    if (h == NULL)
        error(MSG_ERROR_MEMORY, "Argument");
    {
        strcpy(h->Name, "");
        strcpy(h->Author, "");
        strcpy(h->Version, "");
        strcpy(h->Function, "");
        strcpy(h->Labo, "");
        strcpy(h->Group, "");

        h->usage = NULL;
        h->retmod = NULL;

        h->NbOption = 0;
        h->NbNeededArg = 0;
        h->NbVarArg = 0;
        h->NbOptionArg = 0;
        h->NbNotUsedArg = 0;

        return h;
    }
}

/* check consistency of the t_header (H) */
void CheckConsistencyH(void)
{
    t_argument *a, *b;
    double m0, m1;

    if ((H->Name[0] < 'a') || (H->Name[0] > 'z'))
        error(MSG_ERROR_HEADER, "name");
    if (strlen(H->Author) < 3)
        error(MSG_ERROR_HEADER, "author");
    if ((H->Version[0] < '0') || (H->Version[0] > '9'))
        error(MSG_ERROR_HEADER, "version number");
    if (strlen(H->Function) < 3)
        error(MSG_ERROR_HEADER, "function description");
    if (!H->usage)
        error(MSG_ERROR_HEADER, "usage");
    for (a = H->usage; a; a = a->next)
    {
        /* C_id is always required, check it first */
        if (a->C_id[0] == '\0')
            error(MSG_ERROR_USAGE_REQUIRED, a->Cmt);
        for (b = H->usage; b; b = b->next)
            if ((a != b) && (strcmp(a->C_id, b->C_id) == 0))
                error(MSG_ERROR_USAGE_DUPLICATE, a->C_id);
        if (a->Atype == NONE)
            error(MSG_ERROR_USAGE_TYPE, a->C_id);
        if ((a->IOtype != READ) && (a->IOtype != WRITE))
            error(MSG_ERROR_USAGE_IOTYPE, a->C_id);
        /* check cases where interval checking is not allowed */
        if (a->ICtype != NONE)
        {
            if (a->Min[0] == '\0')
                error(MSG_ERROR_USAGE_ICTYPE, a->C_id);
            if (ISARG_FLAGOPT(a))
                error(MSG_ERROR_USAGE_IC_FLAG, a->C_id);
            if (ISARG_OUTPUT(a))
                error(MSG_ERROR_USAGE_IC_OUT, a->C_id);
            if (ISARG_NOTUSED(a))
                error(MSG_ERROR_USAGE_IC_UNUSED, a->C_id);
        }

        /* check flag */
        if (ISARG_OPTION(a))
        {
            if (a->Flag == '\0')
                error(MSG_ERROR_USAGE_UNKNOWN, a->C_id);
        }
        else
        {
            if (a->Flag != '\0')
                error(MSG_ERROR_USAGE_FLAG_FORBIDDEN, a->C_id);
        }

        /* check H_id */
        if (ISARG_NEEDED(a) || ISARG_OPTARG(a))
        {
            if (a->H_id[0] == '\0')
                error(MSG_ERROR_USAGE_HID_REQUIRED, a->C_id);
        }
        else
        {
            if ((ISARG_VARIABLE(a) || ISARG_NOTUSED(a))
                && (a->H_id[0] != '\0'))
                error(MSG_ERROR_USAGE_HID_FORBIDDEN, a->C_id);
        }

        if (strlen(a->Cmt) < 3)
            error(MSG_ERROR_USAGE_COMMENT, a->C_id);

        /* Check cases where default input value is not allowed */
        if (ISARG_DEFAULT(a))
        {
            if (ISARG_FLAGOPT(a))
                error(MSG_ERROR_USAGE_DEFAULT_INPUT_FLAG, a->C_id);
            if (ISARG_NEEDED(a))
                error(MSG_ERROR_USAGE_DEFAULT_INPUT_NEEDED, a->C_id);
            if (!ISARG_INPUT(a))
                error(MSG_ERROR_USAGE_DEFAULT_INPUT, a->C_id);
            if (ISARG_VARIABLE(a))
                error(MSG_ERROR_USAGE_DEFAULT_INPUT_VARIABLE, a->C_id);
            if (ISARG_NOTUSED(a))
                error(MSG_ERROR_USAGE_DEFAULT_INPUT_NOTUSED, a->C_id);
        }

        /*
         * check interval checking
         * cases where interval checking is not allowed
         * are assumed to be checked with ICtype
         */
        if (((a->Min[0] == '\0') && (a->Max[0] != '\0')) ||
            ((a->Min[0] != '\0') && (a->Max[0] == '\0')))
            error(MSG_ERROR_USAGE_IC_MISSING, a->C_id);
        if (a->Min[0] != '\0')
        {
            if (a->ICtype == NONE)
                error(MSG_ERROR_USAGE_IC_INCONSISTENT, a->C_id);
            m0 = (double) atof(a->Min);
            m1 = (double) atof(a->Max);
            if (m0 >= m1)
                error(MSG_ERROR_USAGE_IC_INCORRECT, a->C_id);
        }
    }
}

t_variable *new_variable(void)
{
    t_variable *var;

    var = (t_variable *) malloc(sizeof(t_variable));
    if (var == NULL)
        error(MSG_ERROR_MEMORY, "t_variable");
    else
    {
        var->Ctype = NONE;

        strcpy(var->Stype, "");
        strcpy(var->Ftype, "");
        strcpy(var->Name, "");
        strcpy(var->Cstorage, "");

        var->PtrDepth = 0;
        var->DeclType = NONE;

        var->arg = NULL;

        var->previous = NULL;
        var->next = NULL;
    }
    return var;
}

t_varfunc *new_varfunc(void)
{
    t_varfunc *f;
    t_variable *var;

    f = (t_varfunc *) malloc(sizeof(t_varfunc));
    if (f == NULL)
        error(MSG_ERROR_MEMORY, "t_varfunc");
    else
    {
        var = new_variable();
        f->v = var;
        f->Itype = I_UNDEFINED;
        f->l0 = -1;
        f->l1 = -1;

        f->param = NULL;
        f->previous = NULL;
        f->next = NULL;
    }
    return f;
}

t_body *new_cbody(void)
{
    t_body *c;

    c = (t_body *) malloc(sizeof(t_body));
    if (c == NULL)
        error(MSG_ERROR_MEMORY, "t_body");
    else
    {
        c->varfunc = NULL;
        c->mfunc = NULL;
    }
    return c;
}

void strdump_arg(char *str, t_argument * a)
{
    if (a == NULL)
        error(MSG_ERROR_NULL, "t_argument");
    else
    {
        sprintf(str, "  dump of arg = %p\n"
                "    previous = %p  \t next=%p\n"
                "    atype   = %d\n"
                "    iotype  = %d\n"
                "    ictype  = %d\n"
                "    flag   = '%c'\n"
                "    h_id    = '%s'\n"
                "    c_id    = '%s'\n"
                "    cmt     = '%s'\n"
                "    val     = '%s'\n"
                "    min     = '%s'\n"
                "    max     = '%s'", (void *) a, (void *) a->previous,
                (void *) a->next, a->Atype, a->IOtype, a->ICtype,
                (a->Flag ? a->Flag : ' '), a->H_id, a->C_id, a->Cmt, a->Val,
                a->Min, a->Max);
    }
}

static void strdump_variable(char *str, t_variable * v)
{
    if (v == NULL)
        error(MSG_ERROR_NULL, "t_variable");
    else
    {
        sprintf(str, "  dump of variable = %p\n"
                "    previous    = %p \t next = %p\n"
                "    name        = '%s'\n"
                "    scalar type = '%s'\n"
                "    full type   = '%s'\n"
                "    ctype       = %d\n"
                "    ptrDepth    = %d\n"
                "    cstorage    = '%s'\n", (void *) v, (void *) v->previous,
                (void *) v->next, v->Name, v->Stype, v->Ftype, v->Ctype,
                v->PtrDepth, v->Cstorage);
    }
}

void strdump_varfunc(char *str, t_varfunc * f)
{
    t_variable *p;
    int i;
    char str_param[STRSIZE];

    if (f == NULL)
        error(MSG_ERROR_NULL, "t_varfunc");
    else
    {
        sprintf(str, "  dump of varfunc = %p\n"
                "    previous    = %p \t next = %p\n"
                "    itype       = %d\n"
                "    name        = '%s'\n"
                "    scalar type = '%s'\n"
                "    full type   = '%s'\n"
                "    ctype       = %d\n"
                "    ptrDepth    = %d\n"
                "    cstorage    = '%s'\n"
                "    l0          = %ld \t l1 = %ld\n", (void *) f,
                (void *) f->previous, (void *) f->next, f->Itype, f->v->Name,
                f->v->Stype, f->v->Ftype, f->v->Ctype, f->v->PtrDepth,
                f->v->Cstorage, f->l0, f->l1);

        if (f->param == NULL)
            strcat(str, " no param\n");
        else
        {
            for (p = f->param, i = 1; p; p = p->next, i++)
            {
                sprintf(str_param, " Parameter #%d :\n", i);
                strcat(str, str_param);
                strdump_variable(str_param, p);
                strcat(str, str_param);
            }
        }
    }
}

t_token *new_cword(void)
{
    t_token *cword;

    cword = (t_token *) malloc(sizeof(t_token));
    if (cword == NULL)
        error(MSG_ERROR_MEMORY, "t_token");
    else
    {
        cword->Wtype = W_UNDEFINED;
        strcpy(cword->Name, "");

        cword->previous = NULL;
        cword->next = NULL;
    }
    return cword;
}

t_statement *new_cinstruction(void)
{
    t_statement *c;

    c = (t_statement *) malloc(sizeof(t_statement));
    if (c == NULL)
        error(MSG_ERROR_MEMORY, "t_statement");
    else
    {
        strcpy(c->phrase, "");
        c->Itype = I_UNDEFINED;
        c->nparam = -1;
        c->ndatatype = -1;
        c->nvar = 0;
        c->wfirst = NULL;
        c->previous = NULL;
        c->next = NULL;
    }
    return c;
}

void delete_cinstruction(t_statement * c)
{
    t_token *w, *wn;

    if (c == NULL)
        error(MSG_ERROR_DELETE_NULL_CINSTRUCTION);
    else
    {
        for (w = wn = c->wfirst; w && wn; w = wn)
        {
            wn = w->next;
            free(w);
        }

        if (c->next)
            delete_cinstruction(c->next);
        free(c);
    }
}

/*
 * Merge Cwords in a t_statement
 * to avoid any c->next.
 */
void merge_cinstruction(t_statement * c)
{
    t_token *cw0, *cw1;
    char loop_flag;

    if (c == NULL)
        error(MSG_ERROR_MERGE_NULL_CINSTRUCTION);
    else if (c->next)
    {
        loop_flag = 1;
        while (loop_flag)
        {
            cw0 = c->wfirst;
            while (cw0->next)
                cw0 = cw0->next;

            cw1 = c->next->wfirst;
            cw0->next = cw1;
            cw1->previous = cw0;

            if (c->next->next)
                c->next = c->next->next;
            else
            {
                c->next = NULL;
                loop_flag = 0;
            }
        }
    }
}
