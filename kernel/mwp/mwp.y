/**
 **  Analyse syntaxique du module et creation de l'arbre         
 **  syntaxique. On analyse egalement les erreurs grammaticale de 
 **  l'entete. 
 **  Version 1.2 (c)1993-2003 J.Froment - S.Parrino
 **/

/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
%{

/* Fichiers d'include */
#include <stdio.h>
#include <string.h>
#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "io.h"


#ifdef __STDC__
char *toktos(char *);
#else
char *toktos();
#endif

#ifdef __STDC__
void declarefunc(Node *);
void declaration(Node*, Node *);
Node *mkclass(int, Node*);
Node *mkqualifier(int, Node*);
Node *record(int, char *, Node *);
Node *forstmt(char *, int, Node *, Node *, Node *, Node *);
void mkenum(Node *, Node *);
Node *mkstatic(Node *);
void printpgm(FILE *);
void mkdeclaration(Node *, Node *);
#else
declarefunc();
declaration();
Node *mkclass();
Node *mkqualifier();
Node *record();
Node *forstmt();
mkenum();
Node *mkstatic();
printpgm();
mkdeclaration();
#endif

/* Flags */
int headerflg = FALSE;
static int declareflg = TRUE;
static int declistflg = FALSE;
static int mwfct_flg  = FALSE;
static int header_passed = FALSE;
static char modulefilename[BUFSIZ];
char modulename[BUFSIZ];

/* Pointer to agregate structure (struct, union, enum) name */
static char *aggname = NULL;

#define DE        declareflg = TRUE
#define DD        declareflg = FALSE
#define DL        if (declistflg == TRUE) DE; else DD
#define DLD       declistflg = FALSE
#define ST(I)     (void)record(STRUCT,I,NULL)
#define UN(I)     (void)record(UNION,I,NULL)
#define FILEIN(T) (T)->filein
#define LINENO(T) (T)->lineno
#define MWFUNC(N) if (mwfct_flg == TRUE) { mwfuncdecl = (N); mwfct_flg = FALSE; }

/* Syntax trees for header statement */
Node *     mwname = NULL;
Node *   mwauthor = NULL;
Node *     mwlabo = NULL;
Node * mwfunction = NULL;
Node *    mwusage = NULL;
Node *    mwgroup = NULL;
Node *  mwversion = NULL;
Node * mwfuncdecl = NULL;

/* Syntax tree for C program */
Node *translation = NULL;

#define ERRORCHAR '?'

/* Automata for verify if usage has good elements in good order */
#ifdef __STDC__
static void usage_error_message(short, short);
#else
static void usage_error_message();
#endif
#define OPTION         0
#define NEEDEDARG      1
#define OPTARG         2
#define VARARG         3
#define NOTUSEDARG     4
#define NEXT_STATE(I,N)(\
                         (((prev_usage_state=usage_state),\
                           (usage_state=usage_tab[(I)][usage_state])) < 0) ?\
                           (\
                             usage_error_message((I), prev_usage_state),\
                             (Node *)NULL\
                           )\
                         :\
                           (N)\
                       )
#define VERIFY_USAGE   (usage_state == 1 || usage_state == 3 || usage_state == 4 ||\
			usage_state == 5)
static short prev_usage_state = -1;
static short usage_state      = 0;
static short usage_tab[5][6]  = {
/*            \    STATE   |  0  |  1  |  2  |  3  |  4  |  5  |*/
/*    INPUT    \           |     |     |     |     |     |     |*/
/*-------------------------+-----+-----+-----+-----+-----+-----|*/
/*     OPTION */           {  2  , -1  ,  2  , -1  , -1  , -1  },
/*  NEEDEDARG */           {  1  ,  1  ,  1  ,  3  , -1  , -1  },
/*     OPTARG */           { -1  ,  3  , -1  ,  3  , -1  , -1  },
/*     VARARG */           { -1  ,  4  , -1  , -1  , -1  , -1  },
/* NOTUSEDARG */           { -1  ,  5  , -1  ,  5  ,  5  ,  5  }
};


static short author_passed = FALSE, usage_passed = FALSE;

/* For HP-UX yacc : allowed access to yytoks struct */
#define __YYSCLASS
%}

/*****************************************************************/
/*****************************************************************/
/**                                                             **/
/**                         MWC tokens                          **/
/**                                                             **/
/*****************************************************************/
/*****************************************************************/

%union {
	Sync *		sync;		/* Value return by ')', '' and ';' */
	char *          character;	/* Value of token CHARACTER */
	unsigned long * integer;	/* Value of token INTEGER */
	double *        real;		/* Value of token FLOAT */
	char *          qstring;	/* Value of token QSTRING */
	char *          text;		/* Value of token ID */
	Symbol *        symbol;		/* Value of token USRTYPID */
	Node *          node;		/* Value return by any non-terminal */
}

/* C token operations group */
%token BCOMMENT		/* / followed by * */
%token ECOMMENT 	/* * followed by / */
%token PLUSEQ		/* += */
%token MINUSEQ		/* -= */
%token MULEQ		/* *= */
%token DIVEQ		/* /= */
%token MODEQ		/* %= */
%token INCR		/* ++ */
%token DECR		/* -- */
%token ANDEQ		/* &= */
%token OREQ		/* |= */
%token EREQ		/* ^= */
%token LS		/* << */
%token LSEQ		/* <<= */
%token RS		/* >> */
%token RSEQ		/* >>= */
%token EQ		/* == */
%token NE		/* != */
%token LE		/* <= */
%token LT		/* < */
%token GE		/* >= */
%token GT		/* > */
%token ANDAND		/* && */
%token OROR		/* || */
%token STREF		/* -> */
%token LARROW		/* <- */
%token RARROW		/* -> */
%token ELLIPSIS		/* ... */
/* Header tokens group */
%token MWCOMMAND
%token NAME
%token AUTHOR
%token LABO
%token FUNCTION
%token USAGE
%token NOTUSED
%token GROUP
%token VERSION
/* Storage class specifier tokens group */
%token <node> AUTO
%token <node> REGISTER
%token <node> STATIC
%token <node> EXTERN
%token <node> TYPEDEF
/* Type specifier tokens group */
%token <node> VOID
%token <node> CHAR
%token <node> SHORT
%token <node> INT
%token <node> LONG
%token <node> FLOAT
%token <node> DOUBLE
%token <node> SIGNED
%token <node> UNSIGNED
/* Type qualifier tokens group */
%token <node> CONST
%token <node> VOLATILE
/* User ID token group */
%token <text> ID
%token <text> USRTYPID
/* Struct or union tokens group */
%token STRUCT
%token UNION
/* Labeled Statement tokens group */
%token CASE
%token DEFAULT
/* Selection Statement tokens group */
%token IF
%token ELSE
%token SWITCH
/* Iteration Statement tokens group */
%token <node> WHILE
%token DO
%token FOR
/* Jump Statement tokens group */
%token GOTO
%token CONTINUE
%token BREAK
%token RETURN
/* Miscellaneous tokens group */
%token ENUM
%token <integer>   SIZEOF
%token <qstring>   QSTRING
%token <integer>   INTEGER
%token <character> CHARACTER
%token <real>      REAL
%token <text>      ENUMERATION
/* Virtual token for syntaxic tree */
%token BALPAR		/* ( expr ... ) */
%token FUNCDECL		/* ( typelist ... ) */
%token NAME		/* Identifier */
%token BLOC		/* { init expr ... } */
%token NULLINST		/* ; */
%token COMPOUND		/* { decl... stmt... } */
%token CAST		/* (type)... */
%token ARRAY		/* type id[const] */
%token ARRAYELT		/* id[expr] */
%token CALL		/* Call function f(a, b, ...) */
%token DECLARATION	/* <class storage> <type> <var ...> ; */
%token VARIABLE		/* C variable */
%token '#'		/* Concatenation of binary tree */
%token NOTYPE		/* Tell us that no type is required */
%token DEFINITION
%token LOOKING
%token CLOSED_INTERVAL
%token OPEN_INTERVAL
%token MAX_EXCLUDED_INTERVAL
%token MIN_EXCLUDED_INTERVAL

/* Operators priorities */
%left ','
%right PLUSEQ MINUSEQ MULEQ DIVEQ MODEQ ANDEQ OREQ EREQ LSEQ RSEQ '='
%right '?' ':'
%left OROR
%left ANDAND
%left '|'
%left '^'
%left '&'
%left EQ NE
%left LE LT GE GT
%left LS RS
%left '+' '-'
%left '*' '/' '%'
%right '!' '~' INCR DECR UPLUS UMINUS DEREF ADDROF CAST SIZEOF
/*%left '(' ')' '[' ']' STREF '.'*/
%left GROUP ARRAY STREF '.'


/* Debugger synchronisation */
%type <sync> '}' ')' ';'

/* Header non-terminal type */
%type <node> AuthorList UsageList UsageSpec OptionSpec OptionDesc
%type <node> ArgInWithoutDefault ArgOutWithoutDefault ArgInWithNumericalDefaultDesc
%type <node> OptArgSpec OptArgDesc NeededArgSpec NeededArgDesc NotusedSpec NotusedDesc
%type <node> VarArgSpec VarArgDesc VarArgIn VarArgOut NumericalConstant
%type <node> Integer Real Character
%type <node> DirIo OptionDescriptor OptionParameter OptionInOrOutWithoutDefault
%type <node> OptionInWithoutDefault OptionOutWithoutDefault
%type <node> OptionInOrOutWithQstringDefault OptionIn
%type <node> Interval SquareBrackets ArgInOrOutWithQstringDefault
%type <node> InOrOutWithQstringDefault
%type <node> InWithNumericalDefaultDesc QstringDefault InWithNumericalDefault
%type <node> NumericalDefault CKeyWords

/* C non-terminal type */
%type <node> TranslationUnit ExternalDeclaration FunctionDefinition Declaration
%type <node> Declarator DeclarationList CompoundStmt
%type <node> DeclarationSpecifiers InitDeclaratorList
%type <node> StructOrUnionSpecifier EnumSpecifier StructDeclarationList
%type <node> StructDeclaration InitDeclarator
%type <node> initializer SpecifierQualifierList StructDeclaratorList
%type <node> StructDeclarator ConstantExpression EnumeratorList Enumerator
%type <node> Pointer DirectDeclarator ParameterTypeList IdentifierList
%type <node> Identifier TypeQualifierList ParameterList ParameterDeclaration
%type <node> AbstractDeclarator	AssignmentExpression InitializerList
%type <node> TypeName DirectAbstractDeclarator Statement
%type <node> LabeledStatement ExpressionStatement
%type <node> SelectionStatement IterationStatement JumpStatement
%type <node> Expression StatementList
%type <node> ConditionalExpression UnaryExpression LogicalORExpression
%type <node> LogicalANDExpression InclusiveORExpression
%type <node> ExclusiveORExpression ANDExpression EqualityExpression
%type <node> RelationalExpression ShiftExpression AdditiveExpression
%type <node> MultiplicativeExpression CastExpression PostfixExpression
%type <node> PrimaryExpression ArgumentExpressionList

/*****************************************************************/
/*****************************************************************/
/**                                                             **/
/**                       MWC grammar                           **/
/**                                                             **/
/*****************************************************************/
/*****************************************************************/

%start ProgramUnit

%%
ProgramUnit :
	Header TranslationUnit
		{ translation = $2; }
	;

/*****************************************************************/
/* Header syntax.                                                */
/* It comes from hard discussion between Jacques Froment and     */
/* Sylvain Parrino and had been added to the original C grammar. */
/*****************************************************************/

Header :
	HeaderBegin ObligatoryHeaderCmd ECOMMENT
		{
		  if (mwname == NULL)
		    fatal_error("Missing module name in the header !\n");
		  if (mwauthor == NULL) {
		    if (author_passed == TRUE)
		      fatal_error("Too many errors in author statement !\n");
		    else
		      fatal_error("Missing author field in the header !\n");
		  }
		  if (mwfunction == NULL)
		    fatal_error("Missing function field in the header !\n");
		  if (mwversion == NULL)
		    fatal_error("Missing version field in the header !\n");
		  if (mwusage == NULL) {
		    if (usage_passed == TRUE)
		      fatal_error("Too many errors in usage statement !\n");
		    else
		      fatal_error("Missing usage field in the header !\n");
		  }
		  else if (VERIFY_USAGE == FALSE)
		    fatal_error("Usage of '%s' must contains an input or an output\n",
		                 mwname->val.text);
		  headerflg = FALSE;
		  header_passed = TRUE;
		}
	;


HeaderBegin :
	BCOMMENT MWCOMMAND
		{ headerflg = TRUE; }
	| error
		{
		  if (header_passed == FALSE)
		    fatal_error("Something is wrong with MegaWave2 header.\n");
		}
	;


ObligatoryHeaderCmd :
	ObligatoryHeaderStmt
	| ObligatoryHeaderCmd ObligatoryHeaderStmt
	;


ObligatoryHeaderStmt :
	NameStmt
	| AuthorStmt
	| FunctionStmt
	| LaboStmt
	| UsageStmt
	| GroupStmt
	| VersionStmt
	;


NameStmt :
	NAME '=' '{' ID '}' ';'
		{
                  if (mwname == NULL) {
                    extern char filein[];
		    mwname = mkleaf(NAME, NULL, 0, (void *)$4);
#ifdef DEBUG
		    PRDBG("MegaWave function name is \"%s\"\n",
                                                              mwname->val.text);
#endif
                    strcpy(modulefilename, filein);
		    strcpy(modulename, mwname->val.text);
                  }
                  else
                    error("Module name already declared in file %s\n", 
                                                                modulefilename);
		}
	| NAME error '{' ID '}' ';'
		{ yyerrok; yyerrmsg("missing '='\n"); }	
	| NAME '=' error ID '}' ';'
		{ yyerrok; yyerrmsg("missing '{'\n"); }	
	| NAME '=' '{' error '}' ';'
		{ yyerrok; yyerrmsg("missing MegaWave name\n"); }	
	| NAME '=' '{' ID error  ';'
		{ yyerrmsg("missing '}'\n"); }	
	| NAME '=' '{' ID '}' error
		{ yyerrok; yyerrmsg("missing ';'\n"); }	
	| NAME  error '=' '{' ID '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '='\n"); }	
	| NAME '=' error '{' ID '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '='\n"); }	
	| NAME '=' '{' error ID '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '%s'\n", $5); }	
	| NAME '=' '{' ID error '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '}'\n"); }	
	| NAME '=' '{' ID '}' error ';'
		{ yyerrok; yyerrmsg("unexpected token after '}'\n"); }	
	;


AuthorStmt :
	AUTHOR '=' '{' AuthorList '}' ';'
		{
		  mwauthor = $4;
		  author_passed = TRUE;
		}
	| AUTHOR error '{' AuthorList '}' ';'
		{ yyerrok; yyerrmsg("missing '='\n"); author_passed = TRUE; }	
	| AUTHOR '=' error AuthorList '}' ';'
		{ yyerrok; yyerrmsg("missing '{'\n");  author_passed = TRUE;}	
	| AUTHOR '=' '{' AuthorList error ';'
		{ yyerrok; yyerrmsg("missing '}'\n"); author_passed = TRUE; }	
	| AUTHOR '=' '{' AuthorList '}' error
		{ yyerrok; yyerrmsg("missing ';'\n"); author_passed = TRUE; }	
	| AUTHOR  error '=' '{' AuthorList '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '='\n"); author_passed = TRUE; }	
	| AUTHOR '=' error '{' AuthorList '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '='\n"); author_passed = TRUE; }	
	| AUTHOR '=' '{' error AuthorList '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '{'\n"); author_passed = TRUE; }	
	| AUTHOR '=' '{' AuthorList error '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '}'\n"); author_passed = TRUE; }	
	| AUTHOR '=' '{' AuthorList '}' error ';'
		{ yyerrok; yyerrmsg("unexpected token after '}'\n"); author_passed = TRUE; }	
	;


AuthorList :
	QSTRING
		{ $$ = mkleaf(QSTRING, NULL, 0, (void *)$1); }
	| QSTRING ',' AuthorList
		{
		  if ($3 != NULL)
		    $$ = mknode('#', NULL, 0,
                                mkleaf(QSTRING, NULL, 0, (void *)$1),
                                $3);
		  else
		    $$ = NULL;
		}
	| QSTRING error AuthorList
		{ yyerrok; $$ = NULL; yyerrmsg("missing ','\n"); }	
	| QSTRING error ',' AuthorList
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before ','\n"); }	
	| QSTRING ',' error  AuthorList
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after ','\n"); }	
	;


FunctionStmt :
	FUNCTION '=' '{' QSTRING '}' ';'
		{ mwfunction = mkleaf(QSTRING, NULL, 0, (void *)$4); }
	| FUNCTION error '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("missing '='\n"); }
	| FUNCTION '=' error QSTRING '}' ';'
		{ yyerrok; yyerrmsg("missing '{'\n"); }
	| FUNCTION '=' '{' error '}' ';'
		{ yyerrok; yyerrmsg("missing quoted string\n"); }
	| FUNCTION '=' '{' QSTRING error ';'
		{ yyerrok; yyerrmsg("missing '}'\n"); }
	| FUNCTION '=' '{' QSTRING '}' error
		{ yyerrok; yyerrmsg("missing ';'\n"); }
	| FUNCTION  error '=' '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '='\n"); }
	| FUNCTION '=' error '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '='\n"); }
	| FUNCTION '=' '{' error QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '{'\n"); }
	| FUNCTION '=' '{' QSTRING error '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '}'\n"); }
	| FUNCTION '=' '{' QSTRING '}' error ';'
		{ yyerrok; yyerrmsg("unexpected token after '}'\n"); }
	;


LaboStmt :
	LABO '=' '{' QSTRING '}' ';'
		{ mwlabo = mkleaf(QSTRING, NULL, 0, (void *)$4); }
	| LABO error  '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("missing '='\n"); }	
	| LABO '=' error  QSTRING '}' ';'
		{ yyerrok; yyerrmsg("missing '{'\n"); }	
	| LABO '=' '{' error  '}' ';'
		{ yyerrok; yyerrmsg("missing quoted string\n"); }	
	| LABO '=' '{' QSTRING error  ';'
		{ yyerrok; yyerrmsg("missing '}'\n"); }	
	| LABO '=' '{' QSTRING '}' error
		{ yyerrok; yyerrmsg("missing ';'\n"); }	
	| LABO  error  '=' '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '='\n"); }	
	| LABO '=' error  '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '='\n"); }	
	| LABO '=' '{' error  QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '{'\n"); }	
	| LABO '=' '{' QSTRING error  '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '}'\n"); }	
	| LABO '=' '{' QSTRING '}' error  ';'
		{ yyerrok; yyerrmsg("unexpected token after '}'\n"); }
	;


UsageStmt :
	USAGE '=' '{' UsageList '}' ';'
		{
		  mwusage = $4;
		  usage_passed = TRUE;
		}
	| USAGE error  '{' UsageList '}' ';'
		{ yyerrok; yyerrmsg("missing '='\n"); usage_passed = TRUE; }	
	| USAGE '=' error  UsageList '}' ';'
		{ yyerrok; yyerrmsg("missing '{'\n"); usage_passed = TRUE; }	
	| USAGE '=' '{' error  '}' ';'						
		{
		  yyerrok;
		  yyerrmsg("missing argument and option specifications\n");
		  usage_passed = TRUE;
		}	
	| USAGE '=' '{' UsageList error  ';'
		{ yyerrok; yyerrmsg("missing '}'\n"); usage_passed = TRUE;}	
	| USAGE '=' '{' UsageList '}' error
		{ yyerrok; yyerrmsg("missing ';'\n"); usage_passed = TRUE; }
	;


UsageList :
	UsageSpec
		{ $$ = $1; }
	| UsageSpec ',' UsageList
		{
		  if ($1 == NULL || $3 == NULL) {
		    if ($1 == NULL)
		      $$ = $3;
		    /*		    else if ($3 = NULL) */
		    /* JF 01/07/03 : bug reported by E. Villeger */
		    else if ($3 == NULL)
		      $$ = $1;
		    else
		      $$ = NULL;
		  } else
		    $$ = mknode(',', NULL, 0, $1, $3);
		}
	;


UsageSpec :
	OptionSpec
		{ $$ = NEXT_STATE(OPTION, $1); }
	| NeededArgSpec
		{ $$ = NEXT_STATE(NEEDEDARG, $1); }
	| OptArgSpec
		{ $$ = NEXT_STATE(OPTARG, $1); }
	| VarArgSpec
		{ $$ = NEXT_STATE(VARARG, $1); }
	| '{' { (void *)NEXT_STATE(OPTARG, NULL); } UsageList '}'
		{
		  if ($3 != NULL)
		    $$ = mknode(BLOC, NULL, 0, NULL, $3);
		  else
		    $$ = NULL;
		}
	| NotusedSpec
		{ $$ = NEXT_STATE(NOTUSEDARG, $1); }
	;


NotusedSpec :
	NotusedDesc QSTRING
		{
		  if ($1 != NULL) {
		    Node *n1;
		    n1 = $1;
		    if (n1->right != NULL) {
		      if (n1->right->name == NAME) {
		        Node *n2;
		        n2 = mknode('#', NULL, 0,
		                    n1->right,
                                    mkleaf(QSTRING, NULL, 0, (void *)$2)
		                   );
		        n1->right = n2;
		        $$ = $1;
		      }
		      else
		        INT_ERROR("yyparse");
		    }
		    else
		      INT_ERROR("yyparse");
		  }
		  else
		    $$ = NULL;
		}
	| NotusedDesc error
		{ yyerrok; $$ = NULL; yyerrmsg("missing quoted string\n"); }
	| NotusedDesc error QSTRING
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before quoted string\n"); }
	;

NotusedDesc :
	NOTUSED RARROW ID
		{
		  $$ = mknode(RARROW, NULL, 0,
		              mkleaf(NOTUSED, NULL, 0, 0),
		              mkleaf(NAME,    NULL, 0, (void *)$3)
		             );
		}
	| NOTUSED error ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing '->' in \"not used\" argument\n"); }
	| NOTUSED RARROW error
		{ yyerrok; $$ = NULL; yyerrmsg("missing C identifier in \"not used\" argument\n"); }
	| NOTUSED error RARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after 'notused' keyword \n"); }
	| NOTUSED RARROW error ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '->' in \"not used\" argument\n"); }
	;

OptArgSpec :
	OptArgDesc QSTRING
		{
		  if ($1 != NULL) {
		    Node *n1;
		    n1 = $1;
		    if (n1->right != NULL) {
		      if (n1->right->name == NAME) {
		        Node *n2;
		        n2 = mknode('#', NULL, 0,
		                    n1->right,
                                    mkleaf(QSTRING, NULL, 0, (void *)$2)
		                   );
		        n1->right = n2;
		        $$ = $1;
		      }
		      else if (n1->right->name == '#' &&
		               n1->right->left != NULL &&
		               n1->right->left->name == NAME) {
		        Node *n2;
		        n2 = mknode('#', NULL, 0,
		                    n1->right->left,
                                    mkleaf(QSTRING, NULL, 0, (void *)$2)
		                   );
		        n1->right->left = n2;
		        $$ = $1;
		      }
		      else
		        INT_ERROR("yyparse");
		    }
		    else
		      INT_ERROR("yyparse");
		  }
		  else
		    $$ = NULL;
		}
	| OptArgDesc error
		{ yyerrok; $$ = NULL; yyerrmsg("missing quoted string\n"); }
	| OptArgDesc error QSTRING
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before quoted string\n"); }
	;


VarArgSpec :
	VarArgDesc QSTRING
		{
		  if ($1 != NULL) {
		    Node *n1;
		    n1 = $1;
		    if (n1->right != NULL) {
		      if (n1->right->name == NAME) {
		        Node *n2;
		        n2 = mknode('#', NULL, 0,
		                    n1->right,
                                    mkleaf(QSTRING, NULL, 0, (void *)$2)
		                  );
		        n1->right = n2;
		        $$ = $1;
		      }
		      else if (n1->right->name == '#' &&
		               n1->right->left != NULL &&
		               n1->right->left->name == NAME) {
		        Node *n2;
		        n2 = mknode('#', NULL, 0,
		                   n1->right->left,
                                   mkleaf(QSTRING, NULL, 0, (void *)$2)
		                  );
		        n1->right->left = n2;
		        $$ = $1;
		      }
		      else
		        INT_ERROR("yyparse");
		    }
		    else
		      INT_ERROR("yyparse");
		  }
		  else
		    $$ = NULL;
		}
	| VarArgDesc error
		{ yyerrok; $$ = NULL; yyerrmsg("missing quoted string\n"); }
	| VarArgDesc error QSTRING
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before quoted string\n"); }
	;


VarArgDesc :
	VarArgIn
		{ $$ = $1; }
	| VarArgIn Interval
		{
		  if ($1 != NULL && $2 != NULL) {
		    if (($1)->right != NULL && ($1)->right->name == NAME) {
		      Node *n;
		      n = mknode('#', NULL, 0, ($1)->right, $2);
		      ($1)->right = n;
		      $$ = $1;
		    }
		    else
		      INT_ERROR("yyparse");
		  }
		  else {
		    if ($1 != NULL)
		      clrtree($1);
		    if ($2 != NULL)
		      clrtree($2);
		    $$ = NULL;
		  }
		}
	| VarArgOut
		{ $$ = $1; }
	;


VarArgIn :
	ELLIPSIS RARROW ID
		{
		  $$ = mknode(RARROW, NULL, 0,
		              mkleaf(ELLIPSIS, NULL, 0, NULL),
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| ELLIPSIS error ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing '->' or '<-' in variable argument\n"); }
	| ELLIPSIS RARROW error
		{ yyerrok; $$ = NULL; yyerrmsg("missing C identifier in variable argument\n"); }
	| ELLIPSIS error RARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '...' in variable argument\n"); }
	| ELLIPSIS RARROW error ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '->' in variable argument\n"); }
	;


VarArgOut :
	ELLIPSIS LARROW ID
		{
		  $$ = mknode(LARROW, NULL, 0,
		              mkleaf(ELLIPSIS, NULL, 0, NULL),
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| ELLIPSIS LARROW error
		{ yyerrok; $$ = NULL; yyerrmsg("missing C identifier in variable argument\n"); }
	| ELLIPSIS error LARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '...' in variable argument\n"); }
	| ELLIPSIS LARROW error ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '<-' in variable argument\n"); }
	;


OptionSpec :
	OptionDesc QSTRING
		{
		  if ($1 != NULL) {
		    Node *n1;
		    n1 = $1;
		    if (n1->right != NULL) {
		      if (n1->name == RARROW && n1->right->name == NAME) {
		        Node *n2;
		        n2 = mknode('#', NULL, 0,
		                   n1->right,
		                   mkleaf(QSTRING, NULL, 0, (void *)$2)
		                  );
		        n1->right = n2;
		        $$ = $1;		    
		      }
		      else if (n1->right->right != NULL) {
		        if (n1->name == ':' && n1->right->right->name == NAME) {
		          Node *n2;
		          n2 = mknode('#', NULL, 0,
		                   n1->right->right,
		                   mkleaf(QSTRING, NULL, 0, (void *)$2)
		                );
		          n1->right->right = n2;
		          $$ = $1;
		        }
		        else if (n1->name == ':' && n1->right->right->name == '#') {
		          Node *n2;
		          n2 = mknode('#', NULL, 0,
		                     n1->right->right->left,
		                     mkleaf(QSTRING, NULL, 0, (void *)$2)
		                    );
		          n1->right->right->left = n2;
		          $$ = $1;		    
		        }
		        else
		          INT_ERROR("yyparse");
		      }
		      else
		        INT_ERROR("yyparse");
		    }
		    else
		      INT_ERROR("yyparse");
		  }
		  else
		    $$ = NULL;
		}
	| OptionDesc error
		{ yyerrok; $$ = NULL; yyerrmsg("missing quoted string\n"); }
	| OptionDesc error QSTRING
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before quoted string\n"); }
	;

NeededArgSpec :
	NeededArgDesc QSTRING
		{
		  if ($1 != NULL) {
		    Node *n1;
		    n1 = $1;
		    if (n1->right != NULL) {
		      if (n1->right->name == NAME) {
		        Node *n2;
		        n2 = mknode('#', NULL, 0,
		                   n1->right,
                                   mkleaf(QSTRING, NULL, 0, (void *)$2)
		                  );
		        n1->right = n2;
		        $$ = $1;
		      }
		      else if (n1->right->name == '#' &&
                               n1->right->left != NULL &&
		               n1->right->left->name == NAME ){
		        Node *n2;
		        n2 = mknode('#', NULL, 0,
		                   n1->right->left,
                                   mkleaf(QSTRING, NULL, 0, (void *)$2)
		                  );
		        n1->right->left = n2;
		        $$ = $1;
		      }
		      else
		        INT_ERROR("yyparse");
		    }
		    else
		      INT_ERROR("yyparse");
		  }
		  else
		    $$ = NULL;
		}
	| NeededArgDesc error
		{ yyerrok; $$ = NULL; yyerrmsg("missing quoted string\n"); }
	| NeededArgDesc error QSTRING
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before quoted string\n"); }
	;


OptionDesc :
	Character OptionDescriptor
		{
		  if ($2 != NULL) {
		    static char optlist[BUFSIZ] = {'\0'};
		    if (*($1->val.character) == ERRORCHAR ||
                        *($1->val.character) == '.' ||
                        isdigit(*($1->val.character))) {
		      error("'%c' cannot be used for option flag\n",
		                                         *($1->val.character));
		      $2->left = $1;
		      $$ = $2;
		    }
		    else if (strlen(optlist) != 0 &&
                             strchr(optlist, *($1->val.character)) != NULL) {
		      error("'%c' is already used in usage statement as user option\n",
		                                         *($1->val.character));
		      $2->left = $1;
		      $$ = $2;
		    }
		    else {
		      char buffer[2];
		      sprintf(buffer, "%c", *($1->val.character));
		      strcat(optlist, buffer);
		      $2->left = $1;
		      $$ = $2;
		    }
		  }
		  else
		    $$ = NULL;
		}	
	| error Character OptionDescriptor					
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before char '%c'\n",	
		                                        *($2->val.character));}	
	| Character error OptionDescriptor
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after char '%c'\n", 
		                                        *($1->val.character)); }
	;

OptionDescriptor :
	':' OptionParameter
		{
		  if ($2 != NULL)
		    $$ = mknode(':', NULL, 0, NULL, $2);
		  else
		    $$ = NULL;
		}
	| RARROW ID
		{ $$ = mknode(RARROW, NULL, 0, NULL, mkleaf(NAME, NULL, 0,
		                                                 (void *)$2)); }
	| RARROW error  ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '->'\n"); }
	;

OptionParameter :
	OptionInOrOutWithoutDefault
		{ $$ = $1; }
	| OptionInOrOutWithQstringDefault
		{ $$ = $1; }
	| OptionIn
		{ $$ = $1; }
	;

OptionInOrOutWithoutDefault :
	OptionInWithoutDefault
		{ $$ = $1; }
	| OptionInWithoutDefault Interval
		{
		  if ($1 != NULL && $2 != NULL) {
		    if (($1)->right != NULL && ($1)->right->name == NAME) {
		      Node *n;
		      n = mknode('#', NULL, 0, ($1)->right, $2);
		      ($1)->right = n;
		      $$ = $1;
		    }
		    else
		      INT_ERROR("yyparse");
		  }
		  else {
		    if ($1 != NULL)
		      clrtree($1);
		    if ($2 != NULL)
		      clrtree($2);
		    $$ = NULL;
		  }
		}
	| OptionOutWithoutDefault
		{ $$ = $1; }
	;

CKeyWords :
	AUTO
		{ $$ = mkleaf(NAME, NULL, 0, "auto"); }
	| BREAK
		{ $$ = mkleaf(NAME, NULL, 0, "break"); }
	| CASE
		{ $$ = mkleaf(NAME, NULL, 0, "case"); }
	| CHAR
		{ $$ = mkleaf(NAME, NULL, 0, "char"); }
	| CONST
		{ $$ = mkleaf(NAME, NULL, 0, "const"); }
	| CONTINUE
		{ $$ = mkleaf(NAME, NULL, 0, "continue"); }
	| DEFAULT
		{ $$ = mkleaf(NAME, NULL, 0, "default"); }
	| DO
		{ $$ = mkleaf(NAME, NULL, 0, "do"); }
	| DOUBLE
		{ $$ = mkleaf(NAME, NULL, 0, "double"); }
	| ELSE
		{ $$ = mkleaf(NAME, NULL, 0, "else"); }
	| ENUM
		{ $$ = mkleaf(NAME, NULL, 0, "enum"); }
	| EXTERN
		{ $$ = mkleaf(NAME, NULL, 0, "extern"); }
	| FLOAT
		{ $$ = mkleaf(NAME, NULL, 0, "float"); }
	| FOR
		{ $$ = mkleaf(NAME, NULL, 0, "for"); }
	| GOTO
		{ $$ = mkleaf(NAME, NULL, 0, "goto"); }
	| IF
		{ $$ = mkleaf(NAME, NULL, 0, "if"); }
	| INT
		{ $$ = mkleaf(NAME, NULL, 0, "int"); }
	| LONG
		{ $$ = mkleaf(NAME, NULL, 0, "long"); }
	| REGISTER
		{ $$ = mkleaf(NAME, NULL, 0, "register"); }
	| RETURN
		{ $$ = mkleaf(NAME, NULL, 0, "return"); }
	| SHORT
		{ $$ = mkleaf(NAME, NULL, 0, "short"); }
	| SIGNED
		{ $$ = mkleaf(NAME, NULL, 0, "signed"); }
	| SIZEOF
		{ $$ = mkleaf(NAME, NULL, 0, "sizeof"); }
	| STATIC
		{ $$ = mkleaf(NAME, NULL, 0, "static"); }
	| STRUCT
		{ $$ = mkleaf(NAME, NULL, 0, "struct"); }
	| SWITCH
		{ $$ = mkleaf(NAME, NULL, 0, "switch"); }
	| TYPEDEF
		{ $$ = mkleaf(NAME, NULL, 0, "typedef"); }
	| UNION
		{ $$ = mkleaf(NAME, NULL, 0, "union"); }
	| UNSIGNED
		{ $$ = mkleaf(NAME, NULL, 0, "unsigned"); }
	| VOLATILE
		{ $$ = mkleaf(NAME, NULL, 0, "volatile"); }
	| WHILE
		{ $$ = mkleaf(NAME, NULL, 0, "while"); }
	;

OptionInWithoutDefault :
	ID RARROW ID
		{
		  $$ = mknode(RARROW, NULL, 0,
		              mkleaf(NAME, NULL, 0, (void *)$1),
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| CKeyWords RARROW ID
		{
		  $$ = mknode(RARROW, NULL, 0,
		              $1,
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}	
	| error RARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing TeX option name or '...'\n"); }
	| ID error ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing '->' or '<-' after \"%s\" TeX name\n", $1); }
	| ID RARROW error
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected error after '->'\n"); }
	| ID error RARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after TeX name '%s'\n", $1); }
	| ID RARROW error ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before C identifier '%s'\n", $4);}	
	;

OptionOutWithoutDefault :
	ID LARROW ID
		{
		  $$ = mknode(LARROW, NULL, 0,
		              mkleaf(NAME, NULL, 0, (void *)$1),
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| CKeyWords LARROW ID
		{
		  $$ = mknode(LARROW, NULL, 0,
		              $1,
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| error LARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing TeX option name or '...'\n"); }
	| ID LARROW error
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected error after '<-'\n"); }
	| ID error LARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after TeX name '%s'\n", $1); }
	| ID LARROW error ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before C identifier '%s'\n", $4);}	
	;


OptionInOrOutWithQstringDefault :
	InOrOutWithQstringDefault
		{ $$ = $1; }
	;


OptionIn :
	InWithNumericalDefaultDesc
		{ $$ = $1; }
	;


Interval :
	SquareBrackets NumericalConstant ',' NumericalConstant SquareBrackets
		{
		  int token;
		  if ($2 != NULL && $4 != NULL) {
		    if ($1->name == '[' && $5->name == '[')
		      token = MAX_EXCLUDED_INTERVAL;
		    else if ($1->name == '[' && $5->name == ']')
		      token = CLOSED_INTERVAL;
		    else if ($1->name == ']' && $5->name == '[')
		      token = OPEN_INTERVAL;
		    else if ($1->name == ']' && $5->name == ']')
		      token = MIN_EXCLUDED_INTERVAL;
		    clrtree($1);
		    clrtree($5);
		    $$ = mknode(token, NULL, 0, (void *)$2, (void *)$4);
		  }
		  else {
		    if ($2 != NULL)
		      clrtree($2);
		    if ($4 != NULL)
		      clrtree($4);
		    $$ = NULL;
		  }
		}
	| SquareBrackets error ',' NumericalConstant SquareBrackets
		{
		  yyerrok;
		  $$ = NULL;
		  yyerrmsg("missing min constant in interval\n");
		  clrtree($1);
		  clrtree($5);
		}
	| SquareBrackets NumericalConstant error NumericalConstant SquareBrackets
		{
		  yyerrok;
		  $$ = NULL;
		  yyerrmsg("missing ',' in interval\n");
		  clrtree($1);
		  clrtree($5);
		}
	| SquareBrackets NumericalConstant ',' error SquareBrackets
		{
		  yyerrok;
		  $$ = NULL;
		  yyerrmsg("missing max constant in interval\n");
		  clrtree($1);
		  clrtree($5);
		}
	| SquareBrackets error NumericalConstant ',' NumericalConstant SquareBrackets
		{
		  yyerrok;
		  $$ = NULL;
		  yyerrmsg("unexpected token after '%s' in interval\n",
		          ($1->name=='[') ? "[" : "]");
		  clrtree($1);
		  clrtree($5);
		}
	| SquareBrackets NumericalConstant error ',' NumericalConstant SquareBrackets
		{
		  yyerrok;
		  $$ = NULL;
		  yyerrmsg("unexpected token before ',' in interval\n");
		  clrtree($1);
		  clrtree($5);
		}
	| SquareBrackets NumericalConstant ',' error NumericalConstant SquareBrackets
		{
		  yyerrok;
		  $$ = NULL;
		  yyerrmsg("unexpected token after ',' in interval\n");
		  clrtree($1);
		  clrtree($5);
		}
	| SquareBrackets NumericalConstant ',' NumericalConstant error SquareBrackets
		{
		  yyerrok;
		  $$ = NULL;
		  yyerrmsg("unexpected token before '%s' in interval\n",
		          ($6->name=='[') ? "[" : "]");
		  clrtree($1);
		  clrtree($6);
		}
	;

SquareBrackets :
	'['
		{ $$ = mkleaf('[', NULL, 0, NULL); }
	| ']'
		{ $$ = mkleaf(']', NULL, 0, NULL); }
	;


NumericalConstant :
	Integer
		{ $$ = $1; }
	| Real
		{ $$ = $1; }
	| Character
		{ $$ = $1; }
	;

Integer :
	'-' INTEGER
		{
#		ifdef FLEX
		  extern char *yytext;
#		else
		  extern char yytext[];
#		endif
		  Node *n;
		  char buffer[BUFSIZ];
		  sprintf(buffer, "-%s", (char *)yytext);
		  *$2 = -*$2;
		  n = mkleaf(INTEGER, NULL, 0, (void *)$2);
		  n->yytext = toktos(buffer);
		  $$ = n;
		}
	| '-' error INTEGER
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '-'\n"); }
	| INTEGER
		{
#		ifdef FLEX
		  extern char *yytext;
#		else
		  extern char yytext[];
#		endif
		  Node *n;
		  n = mkleaf(INTEGER, NULL, 0, (void *)$1);
		  n->yytext = toktos((char *)yytext);
		  $$ = n;
		}
	;

Real :
	'-' REAL
		{
#		ifdef FLEX
		  extern char *yytext;
#		else
		  extern char yytext[];
#		endif
		  Node *n;
		  char buffer[BUFSIZ];
		  sprintf(buffer, "-%s", (char *)yytext);
		  *$2 = -*$2;
		  n = mkleaf(REAL, NULL, 0, (void *)$2);
		  n->yytext = toktos(buffer);
		  $$ = n;
		}
	| '-' error REAL
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '-'\n"); }
	| REAL
		{
#		ifdef FLEX
		  extern char *yytext;
#		else
		  extern char yytext[];
#		endif
		  Node *n;
		  n = mkleaf(REAL, NULL, 0, (void *)$1);
		  n->yytext = toktos((char *)yytext);
		  $$ = n;
		}
	;

Character :
	CHARACTER
		{
#		ifdef FLEX
		  extern char *yytext;
#		else
		  extern char yytext[];
#		endif
		  Node *n;
		  n = mkleaf(CHARACTER, NULL, 0, (void *)$1);
		  n->yytext = toktos((char *)yytext);
		  $$ = n;
		}
	;

NeededArgDesc :
	ArgInWithoutDefault
		{ $$ = $1; }
	| ArgInWithoutDefault Interval
		{
		  if ($1 != NULL && $2 != NULL) {
		    if (($1)->right != NULL && ($1)->right->name == NAME) {
		      Node *n;
		      n = mknode('#', NULL, 0, ($1)->right, $2);
		      ($1)->right = n;
		      $$ = $1;
		    }
		    else
		    INT_ERROR("yyparse");
		  }
		  else {
		    if ($1 != NULL)
		      clrtree($1);
		    if ($2 != NULL)
		      clrtree($2);
		    $$ = NULL;
		  }
		}
	| ArgOutWithoutDefault
		{ $$ = $1; }
	;

OptArgDesc :
	ArgInOrOutWithQstringDefault
		{ $$ = $1; }
	| ArgInWithNumericalDefaultDesc
		{ $$ = $1; }
	;


ArgInWithoutDefault :
	ID RARROW ID
		{
		  $$ = mknode(RARROW, NULL, 0,
		              mkleaf(NAME, NULL, 0, (void *)$1),
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| CKeyWords RARROW ID
		{
		  $$ = mknode(RARROW, NULL, 0,
		              $1,
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| error RARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing character or TeX name\n"); }
	| ID RARROW error
		{ yyerrok; $$ = NULL; yyerrmsg("missing C argument name\n"); }
	| ID error RARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '%s'\n", $1); }
	| ID RARROW error  ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '%s'\n", $4); }
	| ID error ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing '->' or '<-'\n"); }
	;

ArgOutWithoutDefault :
	ID LARROW ID
		{
		  $$ = mknode(LARROW, NULL, 0,
		              mkleaf(NAME, NULL, 0, (void *)$1),
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| CKeyWords LARROW ID
		{
		  $$ = mknode(LARROW, NULL, 0,
		              $1,
		              mkleaf(NAME, NULL, 0, (void *)$3)
		             );
		}
	| error LARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing TeX name\n"); }
	| ID LARROW error
		{ yyerrok; $$ = NULL; yyerrmsg("missing C argument name\n"); }
	| ID error LARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '%s'\n", $1); }
	| ID LARROW error  ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '%s'\n", $4); }
	;


ArgInOrOutWithQstringDefault :
	InOrOutWithQstringDefault
		{ $$ = $1; }
	;


InOrOutWithQstringDefault :
	QstringDefault DirIo ID
		{
		  if ($1 != NULL && $2 != NULL) {
		    Node *n;
		    n = $2;
		    n->left = $1;
		    n->right = mkleaf(NAME, NULL, 0, (void *)$3);
		    $$ = n;
		  }
		  else {
		    if ($1 != NULL)
		      clrtree($1);
		    if ($2 != NULL)
		      clrtree($2);
		    $$ = NULL;
		  }
		}
	| QstringDefault error ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing '->'or '<-'\n"); }
	| QstringDefault DirIo error
		{ yyerrok; $$ = NULL; yyerrmsg("missing C argument name\n"); }
	| QstringDefault error DirIo ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after ']'\n"); }
	| QstringDefault DirIo error ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '%s'\n", $4); }
	;


QstringDefault :
	'[' ID '=' QSTRING ']'				%prec ARRAY
		{
		  $$ = mknode('=', NULL, 0,
		              mkleaf(NAME, NULL, 0, (void *)$2),
		              mkleaf(QSTRING, NULL, 0, (void *)$4));
		}
	| '[' CKeyWords '=' QSTRING ']'				%prec ARRAY
		{
		  $$ = mknode('=', NULL, 0,
		              $2,
		              mkleaf(QSTRING, NULL, 0, (void *)$4));
		}
	| '[' error '=' QSTRING ']'			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("missing TeX name\n"); }
	| '[' ID error QSTRING ']'			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("missing '='\n"); }
	| '[' ID '=' error  ']'				%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("missing quoted string or numerical constant\n"); }
	| '[' ID '=' QSTRING error			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("missing ']'\n"); }
	| '[' error ID '=' QSTRING ']'			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '%s'\n", $3); }
	| '[' ID error '=' QSTRING ']'			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '='\n"); }
	| '[' ID '=' error QSTRING ']'			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '='\n"); }
	| '[' ID '=' QSTRING error ']'			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before ']'\n"); }
	;



ArgInWithNumericalDefaultDesc :
	InWithNumericalDefaultDesc
		{ $$ = $1; }
	;


InWithNumericalDefaultDesc :
	InWithNumericalDefault
		{ $$ = $1; }
	| InWithNumericalDefault Interval
		{
		  if ($1 != NULL && $2 != NULL) {
		    if (($1)->right != NULL && ($1)->right->name == NAME) {
		      Node *n;
		      n = mknode('#', NULL, 0, ($1)->right, $2);
		      ($1)->right = n;
		      $$ = $1;
		    }
		    else
		      INT_ERROR("yyparse");
		  }
		  else {
		    if ($1 != NULL)
		      clrtree($1);
		    if ($2 != NULL)
		      clrtree($2);
		    $$ = NULL;
		  }
		}
	;



InWithNumericalDefault :
	NumericalDefault RARROW ID
		{
		  if ($1 != NULL)
		    $$ = mknode(RARROW, NULL, 0, $1, mkleaf(NAME, NULL, 0, (void *)$3));
		  else
		    $$ = NULL;
		}
	| NumericalDefault error ID
		{ yyerrok; $$ = NULL; yyerrmsg("missing '->'\n"); }
	| NumericalDefault RARROW error
		{ yyerrok; $$ = NULL; yyerrmsg("missing C argument name\n"); }
	| NumericalDefault error RARROW ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '->'\n"); }
	| NumericalDefault RARROW error ID
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '%s'\n", $4); }
	;


NumericalDefault :
	'[' ID '=' NumericalConstant ']'			%prec ARRAY
		{
		  if ($4 != NULL)
		    $$ = mknode('=', NULL, 0, mkleaf(NAME, NULL, 0, (void *)$2), $4);
		  else
		    $$ = NULL;
		}
	| '[' CKeyWords '=' NumericalConstant ']'		%prec ARRAY
		{
		  if ($4 != NULL)
		    $$ = mknode('=', NULL, 0, $2, $4);
		  else
		    $$ = NULL;
		}
	| '[' error '=' NumericalConstant ']'			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("missing TeX name\n"); }
	| '[' ID error NumericalConstant ']'			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("missing '='\n"); }
	| '[' ID '=' NumericalConstant error			%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("missing ']'\n"); }
	| '[' error ID '=' NumericalConstant ']'		%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '%s'\n", $3); }
	| '[' ID error '=' NumericalConstant ']'		%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before '='\n"); }
	| '[' ID '=' error NumericalConstant ']'		%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token after '='\n"); }
	| '[' ID '=' NumericalConstant error ']'		%prec ARRAY
		{ yyerrok; $$ = NULL; yyerrmsg("unexpected token before ']'\n"); }
	;
DirIo :
	RARROW
		{ $$ = mknode(RARROW, NULL, 0, NULL, NULL); }
	| LARROW
		{ $$ = mknode(LARROW, NULL, 0, NULL, NULL); }
	;

GroupStmt :
	GROUP '=' '{' QSTRING '}' ';'
		{ mwgroup = mkleaf(QSTRING, NULL, 0, (void *)$4); }
	| GROUP error  '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("missing '='\n"); }	
	| GROUP '=' error  QSTRING '}' ';'
		{ yyerrok; yyerrmsg("missing '{'\n"); }	
	| GROUP '=' '{' error  '}' ';'
		{ yyerrok; yyerrmsg("missing quoted string\n"); }	
	| GROUP '=' '{' QSTRING error  ';'
		{ yyerrok; yyerrmsg("missing '}'\n"); }	
	| GROUP '=' '{' QSTRING '}' error
		{ yyerrok; yyerrmsg("missing ';'\n"); }	
	| GROUP  error  '=' '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '='\n"); }	
	| GROUP '=' error  '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '='\n"); }	
	| GROUP '=' '{' error  QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '{'\n"); }	
	| GROUP '=' '{' QSTRING error  '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '}'\n"); }	
	| GROUP '=' '{' QSTRING '}' error  ';'
		{ yyerrok; yyerrmsg("unexpected token after '}'\n"); }
	;

VersionStmt :
	VERSION '=' '{' QSTRING '}' ';'
		{ mwversion = mkleaf(QSTRING, NULL, 0, (void *)$4); }
	| VERSION error  '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("missing '='\n"); }	
	| VERSION '=' error  QSTRING '}' ';'
		{ yyerrok; yyerrmsg("missing '{'\n"); }	
	| VERSION '=' '{' error  '}' ';'
		{ yyerrok; yyerrmsg("missing quoted string\n"); }	
	| VERSION '=' '{' QSTRING error  ';'
		{ yyerrok; yyerrmsg("missing '}'\n"); }	
	| VERSION '=' '{' QSTRING '}' error
		{ yyerrok; yyerrmsg("missing ';'\n"); }	
	| VERSION  error  '=' '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '='\n"); }	
	| VERSION '=' error  '{' QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '='\n"); }	
	| VERSION '=' '{' error  QSTRING '}' ';'
		{ yyerrok; yyerrmsg("unexpected token after '{'\n"); }	
	| VERSION '=' '{' QSTRING error  '}' ';'
		{ yyerrok; yyerrmsg("unexpected token before '}'\n"); }	
	| VERSION '=' '{' QSTRING '}' error  ';'
		{ yyerrok; yyerrmsg("unexpected token after '}'\n"); }
	;

/*****************************************************************/
/* C Language syntax                                             */
/* The grammar comes from "The C programming Language -"         */
/*                         2nd ed.                               */
/*                         Brian W. Kernighan/Denis M. Ritchie   */
/*                         Prentice Hall Software Series         */
/*****************************************************************/

TranslationUnit :
	ExternalDeclaration
		{ $$ = $1; }
	| TranslationUnit ExternalDeclaration
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	;

ExternalDeclaration :
	FunctionDefinition
		{
		  Node *n;
		  n = $1;
		  MWFUNC(n);
		  $$ = n;
		}
	| Declaration
		{ $$ = $1; }
	;

FunctionDefinition :
	DeclarationSpecifiers Declarator {DL;} DeclarationList {DD; DLD ;} CompoundStmt
		{
		  Node *n, *n1, *n2;
		  DE;
		  n1 = mknode('#', NULL, 0, $2, $4);
		  n2 = mknode('#', NULL, 0, n1, $6);
		  n = mknode(FUNCTION, NULL, 0, $1, n2);
		  declarefunc(n);
		  $$ = n;
		}
	| DeclarationSpecifiers Declarator { DD; DLD; } CompoundStmt
		{
		  Node *n, *n1, *n2;
		  DE;
		  n1 = mknode('#', NULL, 0, $2, NULL);
		  n2 = mknode('#', NULL, 0, n1, $4);
		  n = mknode(FUNCTION, NULL, 0, $1, n2);
		  declarefunc(n);
		  $$ = n;
		}
	| Declarator {DL;} DeclarationList { DD; DLD; } CompoundStmt
		{
		  Node *n, *n1, *n2;
		  DE;
		  n1 = mknode('#', NULL, 0, $1, $3);
		  n2 = mknode('#', NULL, 0, n1, $5);
		  n = mknode(FUNCTION, NULL, 0, NULL, n2);
		  declarefunc(n);
		  $$ = n;
		}
	| Declarator { DD; DLD; } CompoundStmt
		{
		  Node *n, *n1, *n2;
		  DE;
		  n1 = mknode('#', NULL, 0, $1, NULL);
		  n2 = mknode('#', NULL, 0, n1, $3);
		  n = mknode(FUNCTION, NULL, 0, NULL, n2);
		  declarefunc(n);
		  $$ = n;
		}
	;

Declaration :
	DeclarationSpecifiers InitDeclaratorList ';'
		{
		  Node *n;
		  mkdeclaration($2, $1);
		  n = mknode(DECLARATION, FILEIN($3), LINENO($3), $1, $2);
                  if (declareflg && !declistflg)
		    $$ = mkstatic(n);
		  else
		    $$ = n;
#ifdef DEBUG
		  PRDBG("yyparse : Declaration 1-2 %N %N\n", $1, $2);
#endif
		}
	| DeclarationSpecifiers ';'
		{
		  $$ = mknode(DECLARATION, FILEIN($2), LINENO($2), $1, NULL);
#ifdef DEBUG
		  PRDBG("yyparse : Declaration 2 %N\n", $1);
#endif
		}
	;

DeclarationList :
	Declaration
		{ $$ = $1; }
	| Declaration DeclarationList
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	;
			  
DeclarationSpecifiers :
	AUTO DeclarationSpecifiers
		{ $$ = mkclass(AUTO, $2); }
	| REGISTER DeclarationSpecifiers
		{ $$ = mkclass(REGISTER, $2); }
	| STATIC DeclarationSpecifiers
		{ $$ = mkclass(STATIC, $2); }
	| EXTERN DeclarationSpecifiers
		{ $$ = mkclass(EXTERN, $2); }
	| TYPEDEF DeclarationSpecifiers
		{ $$ = mkclass(TYPEDEF, $2); }
	| AUTO
		{ $$ = mkclass(AUTO, NULL); }
	| REGISTER
		{ $$ = mkclass(REGISTER, NULL); }
	| STATIC
		{ $$ = mkclass(STATIC, NULL); }
	| EXTERN
		{ $$ = mkclass(EXTERN, NULL); }
	| TYPEDEF
		{ $$ = mkclass(TYPEDEF, NULL); }
	| VOID DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, mkleaf(VOID, NULL, 0, NULL), $2); }
	| CHAR DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, mkleaf(CHAR, NULL, 0, NULL), $2); }
	| SHORT DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, mkleaf(SHORT, NULL, 0, NULL), $2); }
	| INT DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, mkleaf(INT, NULL, 0, NULL), $2); }
	| LONG DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, mkleaf(LONG, NULL, 0, NULL), $2); }
	| FLOAT DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, mkleaf(FLOAT, NULL, 0, NULL), $2); }
	| DOUBLE DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, mkleaf(DOUBLE, NULL, 0, NULL),$2); }
	| SIGNED DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, mkleaf(SIGNED, NULL, 0, NULL),$2); }
	| UNSIGNED DeclarationSpecifiers
		{
		  $$ = mknode('#', NULL, 0, mkleaf(UNSIGNED, NULL, 0,NULL),$2);
#ifdef DEBUG
		  PRDBG("yyparse : UNSIGNED %N\n", $$);
#endif
		}
	| StructOrUnionSpecifier DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	| EnumSpecifier DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	| USRTYPID DeclarationSpecifiers
		{
		  Symbol *s;
		  Node *n;
		  if ((s=LOOKUP($1)) != NULL) {
#ifdef DEBUG
		    PRDBG("User type (\"%s\", %M, 0x%lx)\n",
                                                    GET_SNAME(s), GET_OBJ(s),
		                                    (unsigned int)GET_SDESC(s));
#endif
		    n = mknode(USRTYPID, NULL, 0, (Node *)GET_SDESC(s), $2);
		    n->val.text = $1;
		    $$ = n;
		  }
		  else {
		    error("type %s not define\n", $1);
		    $$ = (Node*)NULL;
		  }
		}
	| VOID
		{ $$ = mkleaf(VOID, NULL, 0, NULL); }
	| CHAR
		{ $$ = mkleaf(CHAR, NULL, 0, NULL); }
	| SHORT
		{ $$ = mkleaf(SHORT, NULL, 0, NULL); }
	| INT
		{ $$ = mkleaf(INT, NULL, 0, NULL); }
	| LONG
		{ $$ = mkleaf(LONG, NULL, 0, NULL); }
	| FLOAT
		{ $$ = mkleaf(FLOAT, NULL, 0, NULL); }
	| DOUBLE
		{ $$ = mkleaf(DOUBLE, NULL, 0, NULL); }
	| SIGNED
		{
		  Node *n1, *n2;
		  n1 = mkleaf(SIGNED, NULL, 0, NULL);
		  n2 = mkleaf(INT, NULL, 0, NULL);
		  $$ = mknode('#', NULL, 0, n1, n2);
		}
	| UNSIGNED
		{
		  Node *n1, *n2;
		  n1 = mkleaf(UNSIGNED, NULL, 0, NULL);
		  n2 = mkleaf(INT, NULL, 0, NULL);
		  $$ = mknode('#', NULL, 0, n1, n2);
		}
	| StructOrUnionSpecifier
		{ $$ = $1; }
	| EnumSpecifier
		{ $$ = $1; }
	| USRTYPID
		{
		  Symbol *s;
		  Node *n;
		  if ((s=LOOKUP($1)) != NULL) {
#ifdef DEBUG
		    PRDBG("User type (\"%s\", %M, 0x%lx)\n",
                                                    GET_SNAME(s), GET_OBJ(s),
		                                    (unsigned int)GET_SDESC(s));
#endif
		    n = mknode(USRTYPID, NULL, 0, (Node *)GET_SDESC(s), NULL);
		    n->val.text = $1;
		    $$ = n;
		  }
		  else {
		    error("type %s not define\n", $1);
		    $$ = (Node*)NULL;
		  }
		}
	| CONST DeclarationSpecifiers
		{ $$ = mkqualifier(CONST, $2); }
	| VOLATILE DeclarationSpecifiers
		{ $$ = mkqualifier(VOLATILE, $2); }
	| CONST
		{ $$ = mkqualifier(CONST, NULL); }
	| VOLATILE
		{ $$ = mkqualifier(VOLATILE, NULL); }
	;

StructOrUnionSpecifier :
	STRUCT ID { ST($2) ; } '{' StructDeclarationList '}'
		{ $$ = record(STRUCT, $2, $5); }
	| STRUCT ID
		{ $$ = record(STRUCT, $2, NULL); }
	| STRUCT '{' StructDeclarationList '}'
		{ $$ = record(STRUCT, genname(), $3); }
	| UNION ID { UN($2) ; } '{' StructDeclarationList '}'
		{ $$ = record(UNION, $2, $5); }
	| UNION ID
		{ $$ = record(UNION, $2, NULL); }
	| UNION '{' StructDeclarationList '}'
		{ $$ = record(UNION, genname(), $3); }
	;

StructDeclarationList :
	StructDeclaration
		{ $$ = $1; }
	| StructDeclarationList StructDeclaration
		{ $$ = mknode('#',NULL, 0, $1, $2); }
	;

InitDeclaratorList :
	InitDeclarator
		{ $$ = $1; }
	| InitDeclaratorList ',' InitDeclarator
		{ $$ = mknode(',', NULL, 0, $1, $3); }
	;

InitDeclarator :
	Declarator
		{ $$ = $1; }
	| Declarator '=' initializer
		{ $$ = mknode('=', NULL, 0, $1, $3); }
	; 

StructDeclaration :
	SpecifierQualifierList StructDeclaratorList ';'
		{ $$ = mknode(DECLARATION, NULL, 0, $1, $2); }
	;

SpecifierQualifierList :
	VOID SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(VOID, NULL, 0, NULL), $2); }
	| CHAR SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(CHAR, NULL, 0, NULL), $2); }
	| SHORT SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(SHORT, NULL, 0, NULL), $2); }
	| INT SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(INT, NULL, 0, NULL), $2); }
	| LONG SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(LONG, NULL, 0, NULL), $2); }
	| FLOAT SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(FLOAT, NULL, 0, NULL), $2); }
	| DOUBLE SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(DOUBLE, NULL, 0, NULL), $2);}
	| SIGNED SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(SIGNED, NULL, 0, NULL), $2);}
	| UNSIGNED SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(UNSIGNED, NULL, 0,NULL),$2);}
	| StructOrUnionSpecifier SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	| EnumSpecifier SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	| USRTYPID SpecifierQualifierList
		{
		  Symbol *s;
		  Node *n;
		  if ((s=LOOKUP($1)) != NULL) {
#ifdef DEBUG
		    PRDBG("User type (\"%s\", %M, 0x%lx)\n",
                                                   GET_SNAME(s), GET_OBJ(s),
		                                   (unsigned int)GET_SDESC(s));
#endif
		    n = mknode(USRTYPID, NULL, 0, (Node *)GET_SDESC(s), $2);
		    n->val.text = $1;
		    $$ = n;
		  }
		  else {
		    error("type %s not define\n", $1);
		    $$ = (Node*)NULL;
		  }
		}
	| VOID
		{ $$ = mkleaf(VOID, NULL, 0, NULL); }
	| CHAR
		{ $$ = mkleaf(CHAR, NULL, 0, NULL); }
	| SHORT
		{ $$ = mkleaf(SHORT, NULL, 0, NULL); }
	| INT
		{ $$ = mkleaf(INT, NULL, 0, NULL); }
	| LONG
		{ $$ = mkleaf(LONG, NULL, 0, NULL); }
	| FLOAT
		{ $$ = mkleaf(FLOAT, NULL, 0, NULL); }
	| DOUBLE
		{ $$ = mkleaf(DOUBLE, NULL, 0, NULL); }
	| SIGNED
		{ $$ = mkleaf(SIGNED, NULL, 0, NULL); }
	| UNSIGNED
		{ $$ = mkleaf(UNSIGNED, NULL, 0, NULL); }
	| StructOrUnionSpecifier
		{ $$ = $1; }
	| EnumSpecifier
		{ $$ = $1; }
	| USRTYPID
		{
		  Symbol *s;
		  Node *n;
		  if ((s=LOOKUP($1)) != NULL) {
#ifdef DEBUG
		    PRDBG("User type (\"%s\", %M, 0x%lx)\n",
                                                   GET_SNAME(s), GET_OBJ(s),
		                                   (unsigned int)GET_SDESC(s));
#endif
		    n = mknode(USRTYPID, NULL, 0, (Node *)GET_SDESC(s), NULL);
		    n->val.text = $1;
		    $$ = n;
		  }
		  else {
		    error("type %s not define\n", $1);
		    $$ = (Node*)NULL;
		  }
		}
	| CONST SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(CONST, NULL, 0, NULL), $2); }
	| VOLATILE SpecifierQualifierList
		{ $$ = mknode('#', NULL, 0, mkleaf(VOLATILE, NULL, 0,NULL),$2);}
	| CONST
		{ $$ = mkleaf(CONST, NULL, 0, NULL); }
	| VOLATILE
		{ $$ = mkleaf(VOLATILE, NULL, 0, NULL); }
	;

StructDeclaratorList :
	StructDeclarator
		{ $$ = $1; }
	| StructDeclaratorList ',' StructDeclarator
		{ $$ = mknode(',', NULL, 0, $1, $3); }
	;

StructDeclarator :
	Declarator
		{ $$ = $1; }
	| Declarator ':' ConstantExpression
		{ $$ = mknode(':', NULL, 0, $1, $3); }
	| ':' ConstantExpression
		{ $$ = mknode(':', NULL, 0, NULL, $2); }
	;

EnumSpecifier :
	ENUM ID '{' EnumeratorList '}'
		{
		  Node *n;
		  n = record(ENUM, $2, $4);
		  mkenum(n, $4);
		  $$ = n;
		}
	| ENUM ID
		{ $$ = record(ENUM, $2, NULL); }
	| ENUM '{' EnumeratorList '}'
		{
		  Node *n;
		  n = record(ENUM, genname(), $3);
		  mkenum(n, $3);
		  $$ = n;
		}
	;

EnumeratorList :
	Enumerator
		{ $$ = $1; }
	| EnumeratorList ',' Enumerator
		{ $$ = mknode(',', NULL, 0, $1, $3); }
	;

Enumerator :
	ID
		{
		  $$ = mkleaf(NAME, NULL, 0, $1);
		}
	| ID '=' ConstantExpression
		{
		  Node *n;
		  n = mkleaf(NAME, NULL, 0, $1);
		  $$ = mknode('=', NULL, 0, n, $3);
		}
	;

Declarator :
	Pointer DirectDeclarator
		{
		  Node *n;
		  if ((n=$1->left) != NULL) {
		    for (; n->left != NULL; n = n->left);
		    n->left = $2;
		  }
		  else
		    $1->left = $2;
		  $$ = $1;
		}
	| DirectDeclarator
		{ $$ = $1; }
	;

DirectDeclarator :
	ID
		{
		  if (strcmp(mwname->val.text, $1) == 0) {
#ifdef DEBUG
		    PRDBG("Arguments declaration for %s is allowed\n", $1);
#endif
		    declistflg = TRUE;
		    mwfct_flg = TRUE;
		  }
		  $$ = mkleaf(NAME, NULL, 0, $1);
		}
	| '(' Declarator ')'
		{ $$ = mknode(BALPAR, NULL, 0, $2, NULL); }
	| DirectDeclarator '[' ConstantExpression ']'		%prec ARRAY
		{ $$ = mknode(ARRAY, NULL, 0, $1, $3); }
	| DirectDeclarator '[' ']'				%prec ARRAY
		{
		  $$ = mknode(ARRAY, NULL, 0, $1, NULL);
		  /*$$ = mknode(DEREF, NULL, 0, $1, NULL);*/
		}
	| DirectDeclarator '(' ParameterTypeList ')'
		{
#ifdef __STDC__
		  $$ = mknode(FUNCDECL, NULL, 0, $1, $3);
#else
/* Le compilateur qui a compile mwp doit-il etre ANSI pour que mwp
   accepte la declaration ANSI des fonctions ? il semblerait que non !
*/
/*		  $$ = (Node *) NULL;
		  fatal_error("ANSI C declarations are not allowed on this version !\n");
*/
		  $$ = mknode(FUNCDECL, NULL, 0, $1, $3);
#endif
		}
	| DirectDeclarator '(' IdentifierList ')'
		{ $$ = mknode(FUNCDECL, NULL, 0, $1, $3); }
	| DirectDeclarator '(' ')'
		{ $$ = mknode(FUNCDECL, NULL, 0, $1, NULL); }
	;

Pointer :
	'*' TypeQualifierList
		{ $$ = mknode(DEREF, NULL, 0, $2, NULL); }
	| '*'
		{ $$ = mknode(DEREF, NULL, 0, NULL, NULL); }
	| '*' TypeQualifierList Pointer
		{ $$ = mknode(DEREF, NULL, 0, $2, $3); }
	| '*' Pointer
		{ $$ = mknode(DEREF, NULL, 0, $2, NULL); }
	;

TypeQualifierList :
	CONST
		{ $$ = mkleaf(CONST, NULL, 0, NULL); }
	| VOLATILE
		{ $$ = mkleaf(VOLATILE, NULL, 0, NULL); }
	| TypeQualifierList CONST
		{ $$ = mknode('#', NULL, 0, $1, mkleaf(CONST, NULL, 0, NULL)); }
	| TypeQualifierList VOLATILE
		{ $$ = mknode('#', NULL, 0, $1, mkleaf(VOLATILE, NULL,0,NULL));}
	;

ParameterTypeList :
	ParameterList
		{ $$ = $1; }
	| ParameterList ',' ELLIPSIS
		{ $$ = mknode(',', NULL, 0, $1, mkleaf(ELLIPSIS, NULL,0,NULL));}
	;
	
ParameterList :
	ParameterDeclaration
		{ $$ = $1; }
	| ParameterList ',' ParameterDeclaration
		{ $$ = mknode(',', NULL, 0, $1, $3); }
	;

ParameterDeclaration :
	DeclarationSpecifiers Declarator
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	| DeclarationSpecifiers AbstractDeclarator
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	| DeclarationSpecifiers
		{ $$ = mknode('#', NULL, 0, $1, NULL); }
	;

IdentifierList :
	Identifier
		{ $$ = $1; }
	| Identifier ',' IdentifierList
		{ $$ = mknode(',', NULL, 0, $1, $3); }
	;

Identifier :
	ID
		{ $$ = mkleaf(NAME, NULL, 0, $1); }
	;

initializer :
	AssignmentExpression
		{ $$ = $1; }
	| '{' InitializerList '}'
		{ $$ = mknode(BLOC, NULL, 0, $2, NULL); }
	| '{' InitializerList ',' '}'
		{
		  Node *n;
		  n = mknode(',', NULL, 0, $2, NULL);
		  $$ = mknode(BLOC, NULL, 0, n, NULL);
		}
	;

InitializerList :
	initializer
		{ $$ = $1; }
	| InitializerList ',' initializer
		{ $$ = mknode(',', NULL, 0, $1, $3); }
	;

TypeName :
	SpecifierQualifierList
		{ $$ = $1; }
	| SpecifierQualifierList AbstractDeclarator
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	;

AbstractDeclarator :
	Pointer
		{ $$ = $1; }
	| Pointer DirectAbstractDeclarator
		{
		  Node *n;
		  if ((n=$1->left) != NULL) {
		    for (n = $1->left; n->left != NULL; n = n->left);
		    n->left = $2;
		  }
		  else
		    $1->left = $2;
		  $$ = $1;
		}
	| DirectAbstractDeclarator
		{ $$ = $1; }
	;

DirectAbstractDeclarator :
	'(' AbstractDeclarator ')'
		{ $$ = mknode(BALPAR, NULL, 0, $2, NULL); }
	| DirectAbstractDeclarator '[' ConstantExpression ']'	%prec ARRAY
		{ $$ = mknode(ARRAY, NULL, 0, $1, $3); }
	| DirectAbstractDeclarator '[' ']'
		{ $$ = mknode(ARRAY, NULL, 0, $1, NULL); }
	| '[' ConstantExpression ']'				%prec ARRAY
		{ $$ = mknode(ARRAY, NULL, 0, NULL, $2); }
	| '[' ']'
		{ $$ = mknode(ARRAY, NULL, 0, NULL, NULL); }
	| DirectAbstractDeclarator '(' ParameterTypeList ')'
		{ $$ = mknode(FUNCDECL, NULL, 0, $1, $3); }
	| DirectAbstractDeclarator '(' ')'
		{ $$ = mknode(FUNCDECL, NULL, 0, $1, NULL); }
	| '(' ParameterTypeList ')'
		{ $$ = mknode(FUNCDECL, NULL, 0, NULL, $2); }
	| '(' ')'
		{ $$ = mknode(FUNCDECL, NULL, 0, NULL, NULL); }
	;

Statement :
	LabeledStatement
		{ $$ = $1; }
	| ExpressionStatement
		{ $$ = $1; }
	| CompoundStmt
		{ $$ = $1; }
	| SelectionStatement
		{ $$ = $1; }
	| IterationStatement
		{ $$ = $1; }
	| JumpStatement
		{ $$ = $1; }
	;

LabeledStatement :
	ID ':' Statement
		{ $$ = mknode(':', NULL, 0, mkleaf(NAME, NULL, 0, $1), $3); }
	| CASE ConstantExpression ':' Statement
		{ $$ = mknode(CASE, NULL, 0, $2, $4); }
	| DEFAULT ':' Statement
		{ $$ = mknode(DEFAULT, NULL, 0, $3, NULL); }
	;

ExpressionStatement :
	Expression ';'
		{ $$ = mknode('#', NULL, 0, $1,
		              mkleaf(NULLINST, FILEIN($2), LINENO($2) ,NULL));}
	| ';'
		{ $$ = mkleaf(NULLINST,  FILEIN($1), LINENO($1), NULL); }
	;

CompoundStmt :
	'{' DeclarationList StatementList '}'
		{ $$ = mknode(COMPOUND, FILEIN($4), LINENO($4), $2, $3); }
	| '{' DeclarationList '}'
		{ $$ = mknode(COMPOUND, FILEIN($3), LINENO($3), $2, NULL); }
	| '{' StatementList '}'
		{ $$ = mknode(COMPOUND, FILEIN($3), LINENO($3), NULL, $2); }
	| '{' '}'
		{ $$ = mknode(COMPOUND, FILEIN($2), LINENO($2), NULL, NULL); }
	;

StatementList :
	Statement
		{ $$ = $1; }
	| StatementList Statement
		{ $$ = mknode('#', NULL, 0, $1, $2); }
	;

SelectionStatement :
	IF '(' Expression  ')' Statement
		{ $$ = mknode(IF, FILEIN($4), LINENO($4), $3, $5); }
	| IF '(' Expression ')' Statement ELSE Statement
		{
		  Node *n;
		  n = mknode(ELSE, NULL, 0, $5, $7);
		  $$ = mknode(IF, FILEIN($4), LINENO($4), $3, n);
		}
	| SWITCH '(' Expression ')' Statement
		{ $$ = mknode(SWITCH, FILEIN($4), LINENO($4), $3, $5); }
	;

IterationStatement :
	WHILE '(' Expression ')' Statement
		{ $$ = mknode(WHILE, FILEIN($4), LINENO($4), $3, $5); }
	| DO Statement WHILE '(' Expression ')' ';'
		{ $$ = mknode(DO, FILEIN($7), LINENO($7), $2, $5); }
	| FOR '(' Expression ';' Expression ';' Expression ')' Statement
		{ $$ = forstmt(FILEIN($6), LINENO($6), $3, $5, $7, $9); }
	| FOR '(' Expression ';' Expression ';' ')' Statement
		{ $$ = forstmt(FILEIN($6), LINENO($6), $3, $5, NULL, $8); }
	| FOR '(' Expression ';' ';' Expression ')' Statement
		{ $$ = forstmt(FILEIN($5), LINENO($6), $3, NULL, $6, $8); }
	| FOR '(' Expression ';' ';' ')' Statement
		{ $$ = forstmt(FILEIN($5), LINENO($5), $3, NULL, NULL, $7); }
	| FOR '(' ';' Expression ';' Expression ')' Statement
		{ $$ = forstmt(FILEIN($5), LINENO($5), NULL, $4, $6, $8); }
	| FOR '(' ';' Expression ';' ')' Statement
		{ $$ = forstmt(FILEIN($5), LINENO($5), NULL, $4, NULL, $7); }
	| FOR '(' ';' ';' Expression ')' Statement
		{ $$ = forstmt(FILEIN($4), LINENO($4), NULL, NULL, $5, $7); }
	| FOR '(' ';' ';' ')' Statement
		{ $$ = forstmt(FILEIN($4), LINENO($4), NULL, NULL, NULL, $6); }
	;

JumpStatement :
	GOTO ID ';'
		{ $$ = mknode(GOTO, FILEIN($3), LINENO($3),
		              mkleaf(NAME, NULL, 0, $2), NULL); }
	| CONTINUE ';'
		{ $$ = mkleaf(CONTINUE, FILEIN($2), LINENO($2), NULL); }
	| BREAK ';'
		{ $$ = mkleaf(BREAK, FILEIN($2), LINENO($2), NULL); }
	| RETURN Expression ';'
		{ $$ = mknode(RETURN, FILEIN($3), LINENO($3), NULL, $2); }
	| RETURN ';'
		{ $$ = mknode(RETURN, FILEIN($2), LINENO($2), NULL, NULL); }
	;

Expression :
	AssignmentExpression
		{ $$ = $1; }
	| Expression ',' AssignmentExpression
		{ $$ = mknode(',', NULL, 0, $1, $3); }
	;

AssignmentExpression :
	ConditionalExpression
		{ $$ = $1; }
	| UnaryExpression '=' AssignmentExpression
		{ $$ = mknode('=', NULL, 0, $1, $3); }
	| UnaryExpression MULEQ AssignmentExpression
		{ $$ = mknode(MULEQ, NULL, 0, $1, $3); }
	| UnaryExpression DIVEQ AssignmentExpression
		{ $$ = mknode(DIVEQ, NULL, 0, $1, $3); }
	| UnaryExpression MODEQ AssignmentExpression
		{ $$ = mknode(MODEQ, NULL, 0, $1, $3); }
	| UnaryExpression PLUSEQ AssignmentExpression
		{ $$ = mknode(PLUSEQ, NULL, 0, $1, $3); }
	| UnaryExpression MINUSEQ AssignmentExpression
		{ $$ = mknode(MINUSEQ, NULL, 0, $1, $3); }
	| UnaryExpression LSEQ AssignmentExpression
		{ $$ = mknode(LSEQ, NULL, 0, $1, $3); }
	| UnaryExpression RSEQ AssignmentExpression
		{ $$ = mknode(RSEQ, NULL, 0, $1, $3); }
	| UnaryExpression ANDEQ AssignmentExpression
		{ $$ = mknode(ANDEQ, NULL, 0, $1, $3); }
	| UnaryExpression EREQ AssignmentExpression
		{ $$ = mknode(EREQ, NULL, 0, $1, $3); }
	| UnaryExpression OREQ AssignmentExpression
		{ $$ = mknode(OREQ, NULL, 0, $1, $3); }
	;

ConditionalExpression :
	LogicalORExpression
		{ $$ = $1; }
	| LogicalORExpression '?' Expression ':' ConditionalExpression
		{
		  Node *n;
		  n = mknode(':', NULL, 0, $3, $5);
		  $$ = mknode('?', NULL, 0, $1, n);
		} 
	;

ConstantExpression :
	ConditionalExpression
		{ $$ = $1; }
	;

LogicalORExpression :
	LogicalANDExpression
		{ $$ = $1; }
	| LogicalORExpression OROR LogicalANDExpression
		{ $$ = mknode(OROR, NULL, 0, $1, $3); }
	;

LogicalANDExpression :
	InclusiveORExpression
		{ $$ = $1; }
	| LogicalANDExpression ANDAND InclusiveORExpression
		{ $$ = mknode(ANDAND, NULL, 0, $1, $3); }
	;

InclusiveORExpression :
	ExclusiveORExpression
		{ $$ = $1; }
	| InclusiveORExpression '|' ExclusiveORExpression
		{ $$ = mknode('|', NULL, 0, $1, $3); }
	;

ExclusiveORExpression :
	ANDExpression
		{ $$ = $1; }
	| ExclusiveORExpression '^' ANDExpression
		{ $$ = mknode('^', NULL, 0, $1, $3); }
	;

ANDExpression :
	EqualityExpression
		{ $$ = $1; }
	| ANDExpression '&' EqualityExpression
		{ $$ = mknode('&', NULL, 0, $1, $3); }
	;

EqualityExpression :
	RelationalExpression
		{ $$ = $1; }
	| EqualityExpression EQ RelationalExpression
		{ $$ = mknode(EQ, NULL, 0, $1, $3); }
	| EqualityExpression NE RelationalExpression
		{ $$ = mknode(NE, NULL, 0, $1, $3); }
	;

RelationalExpression :
	ShiftExpression
		{ $$ = $1; }
	| RelationalExpression LT ShiftExpression
		{ $$ = mknode(LT, NULL, 0, $1, $3); }
	| RelationalExpression GT ShiftExpression
		{ $$ = mknode(GT, NULL, 0, $1, $3); }
	| RelationalExpression LE ShiftExpression
		{ $$ = mknode(LE, NULL, 0, $1, $3); }
	| RelationalExpression GE ShiftExpression
		{ $$ = mknode(GE, NULL, 0, $1, $3); }
	;

ShiftExpression :
	AdditiveExpression
		{ $$ = $1; }
	| ShiftExpression LS AdditiveExpression
		{ $$ = mknode(LS, NULL, 0, $1, $3); }
	| ShiftExpression RS AdditiveExpression
		{ $$ = mknode(RS, NULL, 0, $1, $3); }
	;
	
AdditiveExpression :
	MultiplicativeExpression
		{ $$ = $1; }
	| AdditiveExpression '+' MultiplicativeExpression
		{ $$ = mknode('+', NULL, 0, $1, $3); }
	| AdditiveExpression '-' MultiplicativeExpression
		{ $$ = mknode('-', NULL, 0, $1, $3); }
	;

MultiplicativeExpression :
	CastExpression
		{ $$ = $1; }
	| MultiplicativeExpression '*' CastExpression
		{ $$ = mknode('*', NULL, 0, $1, $3); }
	| MultiplicativeExpression '/' CastExpression
		{ $$ = mknode('/', NULL, 0, $1, $3); }
	| MultiplicativeExpression '%' CastExpression
		{ $$ = mknode('%', NULL, 0, $1, $3); }
	;

CastExpression :
	UnaryExpression
		{ $$ = $1; }
	| '(' TypeName ')' CastExpression	%prec CAST
		{ $$ = mknode(CAST, NULL, 0, $2, $4); }
	;

UnaryExpression :
	PostfixExpression
		{ $$ = $1; }
	| INCR UnaryExpression
		{ $$ = mknode(INCR, NULL, 0, NULL, $2); }
	| DECR UnaryExpression
		{ $$ = mknode(DECR, NULL, 0, NULL, $2); }
	| '&' CastExpression			%prec ADDROF
		{ $$ = mknode(ADDROF, NULL, 0, NULL, $2); }
	| '*' CastExpression			%prec DEREF
		{ $$ = mknode(DEREF, NULL, 0, NULL, $2); }
	| '+' CastExpression			%prec UPLUS
		{ $$ = mknode('+', NULL, 0, NULL, $2); }
	| '-' CastExpression			%prec UMINUS
		{ $$ = mknode('-', NULL, 0, NULL, $2); }
	| '~' CastExpression
		{ $$ = mknode('~', NULL, 0, NULL, $2); }
	| '!' CastExpression
		{ $$ = mknode('!', NULL, 0, NULL, $2); }
	| SIZEOF UnaryExpression
		{ $$ = mknode(SIZEOF, NULL, 0, NULL, $2); }
	| SIZEOF '(' TypeName ')'
		{
		  Node *n;
		  n = mknode(CAST, NULL, 0, $3, NULL);
		  $$ = mknode(SIZEOF, NULL, 0, NULL, n);
		}
	;

PostfixExpression :
	PrimaryExpression
		{ $$ = $1; }
	| PostfixExpression '[' Expression ']'			%prec ARRAY
		{ $$ = mknode(ARRAYELT, NULL, 0, $1, $3); }
	| PostfixExpression '(' ArgumentExpressionList ')'	%prec GROUP
		{ $$ = mknode(CALL, NULL, 0, $1, $3); }
	| PostfixExpression '(' ')'
		{ $$ = mknode(CALL, NULL, 0, $1, NULL); }
	| PostfixExpression '.' ID
		{ $$ = mknode('.', NULL, 0, $1, mkleaf(NAME, NULL, 0, $3)); }
	| PostfixExpression STREF ID
		{ $$ = mknode(STREF, NULL, 0, $1, mkleaf(NAME, NULL, 0, $3)); }
	| PostfixExpression INCR
		{ $$ = mknode(INCR, NULL, 0, $1, NULL); }
	| PostfixExpression DECR
		{ $$ = mknode(DECR, NULL, 0, $1, NULL); }
	;

PrimaryExpression :
	ID
		{ $$ = mkleaf(NAME, NULL, 0, (void *)$1); }
	| INTEGER
		{
#		ifdef FLEX
		  extern char *yytext;
#		else
		  extern char yytext[];
#		endif
		  Node *n;
		  n = mkleaf(INTEGER, NULL, 0, (void *)$1);
		  n->yytext = toktos((char *)yytext);
		  $$ = n;
		}
	| CHARACTER
		{
#		ifdef FLEX
		  extern char *yytext;
#		else
		  extern char yytext[];
#		endif
		  Node *n;
		  n = mkleaf(CHARACTER, NULL, 0, (void *)$1);
		  n->yytext = toktos((char *)yytext);
		  $$ = n;
		}
	| REAL
		{
#		ifdef FLEX
		  extern char *yytext;
#		else
		  extern char yytext[];
#		endif
		  Node *n;
		  n = mkleaf(REAL, NULL, 0, (void *)$1);
		  n->yytext = toktos((char *)yytext);
		  $$ = n;
		}
	| ENUMERATION
		{ $$ = mkleaf(ENUMERATION, NULL, 0, (void *)$1); }
	| QSTRING
		{ $$ = mkleaf(QSTRING, NULL, 0, (void *)$1); }
	| '(' Expression ')'					%prec GROUP
		{ $$ = mknode(BALPAR, NULL, 0, $2, NULL); }
	;

ArgumentExpressionList :
	AssignmentExpression
		{ $$ = $1; }
	| ArgumentExpressionList ',' AssignmentExpression
		{ $$ = mknode(',', NULL, 0, $1, $3); }
%%
/*
 * CREATION
 * AUTEUR
 *
 * NOM		
 * FONCTION	
 * ENTREES	
 * SORTIES	
 *
 * EVOLUTIONS
 * DATE		PAR		MODIFICATIONS
 */
#ifdef __STDC__
static void usage_error_message(short in, short prev_state)
#else
static void usage_error_message(in, prev_state)
short in;
short prev_state;
#endif
{
  switch(in) {
    case OPTION :
      switch (prev_state) {
        case 1:
          fatal_error("You can't describe options after needed arguments list\n");
          break;
        case 3:
          fatal_error("You can't describe options after optionals arguments list\n");
          break;
        case 4:
          fatal_error("You can't describe options after variable argument\n");
          break;
        default:
	  fprintf(stderr,"(prev_state=%d)\n",prev_state);
          INT_ERROR("usage_error_message");
          break;
      }
      break;
    case NEEDEDARG:
      switch (prev_state) {
        case 4:
          fatal_error("You can't describe needed argument after variable argument\n");
          break;
        default:
	  fprintf(stderr,"(prev_state=%d)\n",prev_state);
          INT_ERROR("usage_error_message");
          break;
      }
      break;
    case OPTARG:
      switch (prev_state) {
        case 0:
          fatal_error("You can't begin to describe usage with optional argument\n");
          break;
        case 2:
          fatal_error("You can't describe optional argument after option list\n");
          break;
        case 4:
          fatal_error("You can't describe optional argument after variable argument\n");
          break;
        case 5:
          fatal_error("Please put notused argument at the end of the usage description\n");
          break;
        default:
	  fprintf(stderr,"(prev_state=%d)\n",prev_state);
          INT_ERROR("usage_error_message");
          break;
      }
      break;
    case VARARG:
      switch (prev_state) {
        case 0:
          fatal_error("You can't begin to describe usage with variable argument\n");
          break;
        case 2:
          fatal_error("You can't describe variable argument after option list\n");
          break;
        case 3:
          fatal_error("You can't describe variable argument after optionals arguments list\n");
          break;
        case 4:
          fatal_error("Only one variable argument description is allowed\n");
          break;
        default:
	  fprintf(stderr,"(prev_state=%d)\n",prev_state);
          INT_ERROR("usage_error_message");
          break;
      }
      break;

    case  NOTUSEDARG:
      switch (prev_state) {
        case 0: case 2:
          fatal_error("Please put notused argument at the end of the usage description\n");
          break;
        default:
	  fprintf(stderr,"(prev_state=%d)\n",prev_state);
          INT_ERROR("usage_error_message");
          break;
      }
      break;

    default:
      fprintf(stderr,"(in=%d,prev_state=%d)\n",in,prev_state);
      INT_ERROR("usage_error_message");
      break;
  }
}

/*
 * CREATION
 * AUTEUR
 *
 * NOM		
 * FONCTION	
 * ENTREES	
 * SORTIES	
 *
 * EVOLUTIONS
 * DATE		PAR		MODIFICATIONS
 */
#ifdef __STDC__
Node *record(int name, char *id, Node *field)
#else
Node *record(name, id, field)
int name;
char *id;
Node *field;
#endif
{
  Node *ret;

  if (declareflg == TRUE) {
#ifdef DEBUG
    PRDBG("record(%M, %s, 0x%lx)\n", name, id, (unsigned long)field);
#endif
    if (id != NULL && field != NULL) {
      Symbol *s;
      if ((s=LOOKUPAGG(id)) != NULL) {
        Node *n;
        n = (Node *)GET_SDESC(s);
        if (n != (Node *)NULL) {
          error("Redefine %s", id);
          ret = (Node *)NULL;
        }
        else {
          ret = mknode(name, NULL, 0, mkleaf(NAME, NULL, 0, id), field);
          SET_SDESC(s, ret);
        }
      }
      else {
        ret = mknode(name, NULL, 0, mkleaf(NAME, NULL, 0, id), field);
        s = ADDAGG(id, name);
        SET_SDESC(s, ret);
        SET_OBJ(s, name);
      }
    } else if (id != NULL && field == NULL) {
      Symbol *s;
      if ((s=LOOKUPAGG(id)) != NULL) {
        Node *n;
        n = (Node *)GET_SDESC(s);
        if (n != (Node *)NULL) {
          if (n->name != name) {
            error("Redefine %s", id);
            ret = (Node *)NULL;
          }
          else
            ret = mknode(name, NULL, 0, mkleaf(NAME, NULL, 0, id), NULL);
        }
        else
          ret = mknode(name, NULL, 0, mkleaf(NAME, NULL, 0, id), NULL);
      }
      else {
        ret = mknode(name, NULL, 0, mkleaf(NAME, NULL, 0, id), NULL);
        s = ADDAGG(id, name);
        SET_RENAME(s, GET_SNAME(s));	/* Don't rename struct/union/enum id */
        SET_SDESC(s, NULL);
        SET_OBJ(s, name);
      }
    }
    else if (id == NULL)
      ret = mknode(name, NULL, 0, NULL, field);
  }
  else {
    if (id == NULL)
      ret = mknode(name, NULL, 0, NULL, field);
    else
      ret = mknode(name, NULL, 0, mkleaf(NAME, NULL, 0, id), field);
  }
  return ret;
}

/*
 * CREATION
 * AUTEUR
 *
 * NOM		
 * FONCTION	
 * ENTREES	
 * SORTIES	
 *
 * EVOLUTIONS
 * DATE		PAR		MODIFICATIONS
 */
#ifdef __STDC__
Node *forstmt(char *f, int l, Node *init, Node *test, Node *next, Node *stmt)
#else
Node *forstmt(f, l, init, test, next, stmt)
char *f;
int l;
Node *init, *test, *next, *stmt;
#endif
{
  Node *n1, *n2, *ret;
  n1 = mknode('#', NULL, 0, next, stmt);
  n2 = mknode('#', NULL, 0, test, n1);
  ret = mknode(FOR, f, l, init, n2);
  return ret;
}

/*
 * CREATION
 * AUTEUR
 *
 * NOM		
 * FONCTION	
 * ENTREES	
 * SORTIES	
 *
 * EVOLUTIONS
 * DATE		PAR		MODIFICATIONS
 */
#ifdef __STDC__
Node *mkclass(int class, Node *n)
#else
Node *mkclass(class, n)
int class;
Node *n;
#endif
{
  Node *ret;

  if (n != NULL) {
#ifdef ACCEPT_ANSI
    Node *i;

    if (n->left != NULL)
      for(i=n->left;
          i->name==CONST || i->name==VOLATILE ||
          i->name == '*';
          i=i->left)
        ;
    else
      i = n;
    switch(i->name) {
#else
    switch(n->name) {
#endif
      case AUTO:
      case REGISTER:
      case STATIC:
      case EXTERN:
      case TYPEDEF:
        error("More than one storage class");
        ret = NULL;
        break;
      default:
        ret = mknode(class, NULL, 0, n, NULL);
        break;
    }
  }
  else
    ret = mkleaf(class, NULL, 0, NULL);
#ifdef DEBUG
  PRDBG("mkclass(%M, 0x%lx)->0x%lx\n", class, (unsigned long)n,
                                              (unsigned long)ret);
#endif
  return ret;
}

#ifdef __STDC__
Node *mkqualifier(int qual, Node *n)
#else
Node *mkqualifier(qual, n)
int qual;
Node *n;
#endif
{
  Node *ret;
  if (n != NULL)
    ret = mknode(qual, NULL, 0, n, NULL);
  else
    ret = mkleaf(qual, NULL, 0, NULL);
#ifdef DEBUG
  PRDBG("mkqualifier(%M, 0x%lx)->0x%lx\n", qual, (unsigned long)n,
                                                  (unsigned long)ret);
#endif
  return ret;
}

#ifdef __STDC__
static Symbol *declarate(Node *var, int obj, int qual, Node *type, Node* access)
#else
static Symbol *declarate(var, obj, qual, type, access)
Node *var;
int obj;
int qual;
Node *type;
Node *access;
#endif
{
  Symbol *s;
  Node *n;
#ifdef DEBUG
  if (qual == (int) NULL)
    PRDBG("declarate(0x%lx, %M, NULL, 0x%lx, 0x%lx)\n", (unsigned long)var, obj,
                                                      (unsigned long)type,
                                                      (unsigned long)access);
  else
    PRDBG("declarate(0x%lx, %M, %M, 0x%lx, 0x%lx)\n", (unsigned long)var, obj,
                                                      qual, (unsigned long)type,
                                                      (unsigned long)access);

  PRDBG("declarate : type = %N, access = %N\n", type, access);
  PRDBG("declarate : var->name = %M\n", var->name);
#endif
  switch(var->name) {
    /* case : type ... vari, varj ... */
    case ',' :
      s = declarate(var->left, obj, qual, type, NULL);
      s = declarate(var->right, obj, qual, type, NULL);
      break;
    /* case : type ... var = cstexpr */
    case '=' :
      s = declarate(var->left, obj, qual, type, access);
      SET_VALUE(s, var->right);
      break;
    /* case type ... (var) ... */
    case BALPAR :
      s = declarate(var->left, obj, qual, type, access);
      break;
    /* case : type ... *var ... */
    case DEREF:
      n = mknode(var->name, NULL, 0, access, NULL);
      s = declarate(var->left, obj, qual, type, n);
      break;
    /* case : type ... var[cstexpr] */
    case ARRAY:
      n = mknode(var->name, NULL, 0, access, var->right);
      s = declarate(var->left, obj, qual, type, n);
      break;
    /* case : type ... var() ... */
    case FUNCDECL :
      n = mknode(var->name, NULL, 0, access, NULL);
      s = declarate(var->left, FUNCDECL, qual, type, n);
      break;
    /* Identifier */
    case NAME:
      if ((s=LOOKUP((char *)var->val.text)) != NULL) {
        Node *n;
        switch(GET_OBJ(s)) {
          case TYPEDEF :
            error("'%s' redefinition\n", GET_SNAME(s));            
            break;
          case FUNCDECL :
            if (GET_STORAGE(s) == STATIC)
              /*warning("'%s' : incompatible storage class specifier\n",
                       GET_SNAME(s))*/;
            else if (nodecmp(type, GET_TYPE(s)) != TRUE)
              warning("'%s' redefinition\n", GET_SNAME(s));
            else if (nodecmp(access, GET_ACCESS(s)) != TRUE)
              warning("'%s' redefinition\n", GET_SNAME(s));
            break;
          case VARIABLE :
	    if (GET_STORAGE(s) == EXTERN)
              switch(qual) {
                case STATIC :
                case REGISTER :
                case AUTO :
                  error("'%s' : incompatible storage class specifier\n",
                       GET_SNAME(s));
                  break;
                case EXTERN :
                  break;
              }
            else if (GET_STORAGE(s) == STATIC && qual != STATIC)
              error("'%s' : incompatible storage class specifier\n",
                       GET_SNAME(s));
            else
              error("'%s' : redefinition\n", (char*)var->val.text,
                       GET_SNAME(s));
            break;
          case ENUMERATION :
            error("'%s' redefinition\n", GET_SNAME(s));
            break;
          default:
#ifdef DEBUG
            PRDBG("declarate : error : GET_OBJ(s) = %M\n", GET_OBJ(s));
#endif
            INT_ERROR("declarate");
            break;
        }
      }
      else {
        if (qual == TYPEDEF) {
          s = ADD((char *)var->val.text, TYPEDEF);
          SET_RENAME(s, GET_SNAME(s));    /* Don't rename typedef */
        }
        else {
          s = ADD((char *)var->val.text, obj);
          SET_STORAGE(s, qual);
          SET_RENAME(s, GET_SNAME(s));
        }
        SET_TYPE(s, type);
        SET_ACCESS(s, access); 
#ifdef DEBUG
        PRDBG("declarate : name = \"%s\", obj = %M, desc = 0x%lx)\n",
                                              GET_SNAME(s), GET_OBJ(s),
                                              (unsigned long)GET_SDESC(s));
#endif
      }
#ifdef DEBUG
      PRDBG("declarate : s->type = %N, s->access = %N\n", s->type, s->access);
#endif
      return s;
      break;
    default:
#ifdef DEBUG
      PRDBG("declarate, error : var->name = %M\n", (int)var->name);
#endif
      INT_ERROR("declarate");
      return (Symbol *)NULL;
      break;
  }
  return s;
}

#ifdef __STDC__
static void static_warning(Symbol *s, int q)
#else
static static_warning(s, q)
Symbol *s;
int q;
#endif
{
  char buffer[BUFSIZ];
  switch(q) {
    case TYPEDEF :
      strcat(buffer, "typedef");
      break;
    case AUTO :
      strcat(buffer, "auto");
      break;
    case REGISTER :
      strcat(buffer, "register");
      break;
    case EXTERN :
      strcat(buffer, "extern");
      break;
    default :
      INT_ERROR("static_warning");
      break;
  }

  /* << BUG >>
     There is no mean to associate the number line where tokens are with
     them. So, informations about number line in warning message
     are not always true and here we must disable this information.
     << END BUG >>
  */
  prline(DISABLE);
  warning("'%s' : storage class %s is changed into static\n", GET_SNAME(s),
                                                                        buffer);
  restoreprline();
}


#ifdef __STDC__
void declarefunc(Node *f)
#else
declarefunc(f)
Node *f;
#endif
{
  Symbol *s;
  Node *type, *id, *n, *n_qual;
  int qualifier, oldqualifier;

#ifdef DEBUG
  PRDBG("declarefunc(0x%lx)\n", (unsigned long)f);
#endif
  /* Find qualifier of function definition and init the type */
  n = f->left;
#ifdef DEBUG
  PRDBG("declarefunc : n = %N\n", n);
#endif
  if (n == NULL)
    type = mkleaf(INT, NULL, 0, NULL);
  else {
    for (qualifier = (int) NULL, n_qual = NULL; n != NULL; n = n->left) {
      switch(n->name) {
        case STATIC :
        case TYPEDEF :
        case AUTO :
        case REGISTER :
        case EXTERN :
          if (qualifier != (int) NULL)
            error("more than one qualifier\n");
          else {
            qualifier = n->name;
            n_qual = n;
            type = n->left;
          }
          break;
        default :
          type = n;
          break;
      }
#ifdef DEBUG
      if (qualifier == (int) NULL)
        PRDBG("declarefunc : qualifier = NULL, type = %N\n", type);
      else
        PRDBG("declarefunc : qualifier = %M, type = %N\n", qualifier, type);
#endif
    }
  }

  id = f->right->left->left;
  s = declarate(id, FUNCDECL, qualifier, type, NULL);
  if (strcmp(GET_SNAME(s), mwname->val.text)) {
    if (f->left == NULL)
      f->left = mkclass(STATIC, NULL);
    else if (f->left != NULL && qualifier == (int) NULL)
      f->left = mkclass(STATIC, f->left);
    else if (f->left != NULL && qualifier != STATIC) {
      static_warning(s, qualifier);
      n_qual->name = STATIC;
    }
    SET_STORAGE(s, STATIC);
  }
  SET_RENAME(s, GET_SNAME(s));
  SET_SDESC(s, f->right);
}

#ifdef __STDC__
void mkdeclaration(Node *var, Node *spec)
#else
mkdeclaration(var, spec)
Node *var, *spec;
#endif
{
  if (declareflg == TRUE) {
    int qualifier;
    Node *n, *type;
#ifdef DEBUG
    PRDBG("mkdeclaration(0x%lx, 0x%lx)\n", (unsigned long)var,
                                         (unsigned long)spec);
#endif

    /* Find qualifier of variable/type/function declaration/definition and
       init the type */
    n = spec;
    if (n == NULL)
      type = mkleaf(INT, NULL, 0, NULL);
    else {
      type = n;
      for (qualifier = (int) NULL; n != NULL; n = n->left)
        switch(n->name) {
          case STATIC :
          case TYPEDEF :
          case AUTO :
          case REGISTER :
          case EXTERN :
            if (qualifier != (int) NULL)
              error("more than one qualifier\n");
            qualifier = n->name;
            type = n->left;
            break;
          default :
            break;
        }
    }

    switch(qualifier) {
      case TYPEDEF :
        (void)declarate(var, TYPEDEF, qualifier, type, NULL);
        break;

      case AUTO :
      case REGISTER :
      case STATIC :
      case EXTERN :
        (void)declarate(var, VARIABLE, qualifier, type, NULL);
        break;

      default :
        (void)declarate(var, VARIABLE, (int) NULL, type, NULL);
        break;
    }
  }
}

#ifdef __STDC__
Node *mkstaticnode(int *phase, Node *n, Node *spec, Node *i, Node *dec)
#else
Node *mkstaticnode(phase, n, spec, i, dec)
int *phase;
Node *n, *spec, *i, *dec;
#endif
{
  if (nodeispresent(dec, FUNCDECL)) {
    if (*phase == VARIABLE) {
      i->father->father->right = i->father->left;
      free(i->father);
      n = mknode('#', NULL, 0, n, mknode(DECLARATION, n->filein, n->lineno,
                                                              cpnode(spec), i));
      *phase = FUNCTION;
    }
  }
  else {
    Symbol *s;
    if (*phase == FUNCTION) {
      i->father->father->right = i->father->left;
      free(i->father);
      n = mknode('#', NULL, 0, n, mknode(DECLARATION, n->filein, n->lineno,
                                           mkclass(STATIC, cpnode(spec)), i));
      *phase = VARIABLE;
    }
    s = LOOKUP(getnameintree(dec));
    if (GET_STORAGE(s) == REGISTER || GET_STORAGE(s) == AUTO)
      static_warning(s, GET_STORAGE(s));
    SET_STORAGE(s, STATIC);
  }
  return n;
}

#ifdef __STDC__
Node *mkstatic(Node *n)
#else
Node *mkstatic(n)
Node *n;
#endif
{
  Node *initdec;

  if ((initdec = n->right) != NULL) {
    int oldqual = (int) NULL;
    Node *spec;
    spec = n->left;
    if (spec->name != EXTERN && spec->name != STATIC && spec->name != TYPEDEF) {
      Node *i, *i_prec, *dec;
      if (initdec->name != ',') {
        if (!nodeispresent(initdec, FUNCDECL)) {
          Symbol *s;
          s = LOOKUP(getnameintree(initdec));
          if (GET_STORAGE(s) == REGISTER || GET_STORAGE(s) == AUTO)
            static_warning(s, GET_STORAGE(s));
          n->left = mkclass(STATIC, spec);
          SET_STORAGE(s, STATIC);
        }
      }
      else {
        int phase;
        if (nodeispresent(initdec->left, FUNCDECL))
          phase = FUNCTION;
        else {
          n->left = mkclass(STATIC, spec);
          phase = VARIABLE;
        }
        for (i=initdec; i->name == ','; i=i->right)
          n = mkstaticnode(&phase, n, spec, i, i->left);
        n = mkstaticnode(&phase, n, spec, i, i);
      }
    }
  }
  return n;
}

#ifdef __STDC__
void mkenum(Node *enumid, Node *enumlist)
#else
mkenum(enumid, enumlist)
Node *enumid, *enumlist;
#endif
{
  Symbol *s;
#ifdef DEBUG
  PRDBG("mkenum(0x%lx, 0x%lx)\n", (unsigned long)enumid,
                                         (unsigned long)enumlist);
#endif
  switch(enumlist->name) {
    case ',' :
      mkenum(enumid, enumlist->left);
      mkenum(enumid, enumlist->right);
      break;
    case '=' :
      mkenum(enumid, enumlist->left);
      break;
    case NAME :
      if (declareflg == TRUE) {
        if (LOOKUP(enumlist->val.text) != NULL)
          error("'%s' : redefinition\n", enumlist->val.text);
        else {
          s = ADD(enumlist->val.text, ENUMERATION);
          SET_RENAME(s, GET_SNAME(s));
          SET_SDESC(s, enumid);
        }
      }
      break;
    default :
#ifdef DEBUG
      PRDBG("mkenum, error : enumlist->name = %M\n", enumlist->name);
#endif
      INT_ERROR("mkenum");
      break;
  }
}


#ifdef __STDC__
void printpgm(FILE *fd)
#else
printpgm(fd)
FILE *fd;
#endif
{
  fprintf(fd, "# 1 \"%s\"\n", modulefilename);
  printnode(fd, translation);
}
