/* mwpl_afile.c:47:NF */ extern void SetScalarConvFunction (Arg *a, char *fc); /* (a, fc) Arg *a; char *fc; */
/* mwpl_afile.c:86:NF */ extern void print_value_scalar_arg (char *s, Arg *a); /* (s, a) char *s; Arg *a; */
/* mwpl_afile.c:133:OF */ extern void writeAheader (void); /* () */
/* mwpl_afile.c:148:OF */ extern void writegendecl (void); /* () */
/* mwpl_afile.c:217:NF */ extern void writedefvar (Arg *a, int dt); /* (a, dt) Arg *a; int dt; */
/* mwpl_afile.c:279:OF */ extern void writeargdecl (void); /* () */
/* mwpl_afile.c:478:NF */ extern void print_interval_and_defval (Arg *a); /* (a) Arg *a; */
/* mwpl_afile.c:527:NF */ extern void print_check_interval_arg (Arg *a, char *s); /* (a, s) Arg *a; char *s; */
/* mwpl_afile.c:588:OF */ extern void print_check_io_arg (void); /* () */
/* mwpl_afile.c:858:OF */ extern void print_read_input (void); /* () */
/* mwpl_afile.c:1081:OF */ extern void print_write_output (void); /* () */
/* mwpl_afile.c:1307:OF */ extern void print_main_function_call (void); /* () */
/* mwpl_afile.c:1359:OF */ extern void writeusage (void); /* () */
/* mwpl_afile.c:1526:OF */ extern void writebody (void); /* () */
/* mwpl_afile.c:1569:OF */ extern void genAfile (void); /* () */
/* mwpl_header.c:37:NF */ extern void GetHeaderStatement (char *s, char *name, char *value); /* (s, name, value) char *s; char *name; char *value; */
/* mwpl_header.c:69:NF */ extern int GetUsageSpec (char *s, int n, char *arg, char *str); /* (s, n, arg, str) char *s; int n; char *arg; char *str; */
/* mwpl_header.c:171:NF */ extern int GetArgUsageSpec (char *s, char *left, char *right); /* (s, left, right) char *s; char *left; char *right; */
/* mwpl_header.c:224:NF */ extern void AnalyseRightArgUsage (char *s, char *Cid, int *ictype, char *min, char *max); /* (s, Cid, ictype, min, max) char *s; char *Cid; int *ictype; char *min; char *max; */
/* mwpl_header.c:258:NF */ extern int GetDefaultInputValue (char *s, char *hid, char *val); /* (s, hid, val) char *s; char *hid; char *val; */
/* mwpl_header.c:295:NF */ extern void AnalyseLeftArgUsage (char *s, int *atype, char *flg, char *hid, char *val); /* (s, atype, flg, hid, val) char *s; int *atype; char *flg; char *hid; char *val; */
/* mwpl_header.c:378:NF */ extern void AddNameStatement (char *value); /* (value) char *value; */
/* mwpl_header.c:404:NF */ extern void AddAuthorStatement (char *value); /* (value) char *value; */
/* mwpl_header.c:428:NF */ extern void AddVersionStatement (char *value); /* (value) char *value; */
/* mwpl_header.c:455:NF */ extern void AddFunctionStatement (char *value); /* (value) char *value; */
/* mwpl_header.c:481:NF */ extern void AddLaboStatement (char *value); /* (value) char *value; */
/* mwpl_header.c:507:NF */ extern void AddGroupStatement (char *value); /* (value) char *value; */
/* mwpl_header.c:538:NF */ extern void AddUsageStatement (char *value); /* (value) char *value; */
/* mwpl_header.c:610:NF */ extern void AnalyseHeaderStatement (char *argclass, char *value); /* (argclass, value) char *argclass; char *value; */
/* mwpl_ifile.c:48:OF */ extern void setprotobuf (void); /* () */
/* mwpl_ifile.c:83:OF */ extern void genIfile (void); /* () */
/* mwpl_instruction.c:76:OF */ extern void Init_Cuserdatatype (void); /* () */
/* mwpl_instruction.c:92:OF */ extern void Free_Cuserdatatype (void); /* () */
/* mwpl_instruction.c:110:NF */ extern void Add_Cuserdatatype (char *s); /* (s) char *s; */
/* mwpl_instruction.c:133:NF */ extern int Is_Ctype (char *s, char **type); /* (s, type) char *s; char **type; */
/* mwpl_instruction.c:152:NF */ extern int AnalyseInstruction (Cinstruction *c); /* (c) Cinstruction *c; */
/* mwpl_instruction.c:553:NF */ extern void SetType (Variable *v, Cinstruction *c, Cword *cw); /* (v, c, cw) Variable *v; Cinstruction *c; Cword *cw; */
/* mwpl_instruction.c:659:NF */ extern void FillNewVariable (VarFunc *v, Cinstruction *c); /* (v, c) VarFunc *v; Cinstruction *c; */
/* mwpl_instruction.c:719:NF */ extern void FillParam_Funcdecl_Ansi (VarFunc *f, Cinstruction *c, Cword *cwb); /* (f, c, cwb) VarFunc *f; Cinstruction *c; Cword *cwb; */
/* mwpl_instruction.c:776:NF */ extern void FillParam_Funcdecl_KR (VarFunc *f, Cinstruction *c, Cword *cwb); /* (f, c, cwb) VarFunc *f; Cinstruction *c; Cword *cwb; */
/* mwpl_instruction.c:864:NF */ extern void FillParam_Funcproto_Ansi (VarFunc *f, Cinstruction *c, Cword *cwb); /* (f, c, cwb) VarFunc *f; Cinstruction *c; Cword *cwb; */
/* mwpl_instruction.c:883:NF */ extern void FillParam_Funcproto_KR (VarFunc *f, Cinstruction *c, Cword *cwb); /* (f, c, cwb) VarFunc *f; Cinstruction *c; Cword *cwb; */
/* mwpl_instruction.c:900:NF */ extern void FillNewFunction (VarFunc *f, Cinstruction *c); /* (f, c) VarFunc *f; Cinstruction *c; */
/* mwpl_instruction.c:1010:OF */ extern int GetNextInstruction (void); /* () */
/* mwpl_io.c:42:NF */ extern void printfslocation (FILE *fd); /* (fd) FILE *fd; */
/* mwpl_io.c:101:NF */ extern void message (int type_error, FILE *fd, char *fmt, ...); /* (type_error, fd, fmt) int type_error; FILE *fd; char *fmt; */
/* mwpl_io.c:156:NF */ extern void rmpathextfilename (char *in, char *out); /* (in, out) char *in; char *out; */
/* mwpl_io.c:187:NF */ extern int lowerstring (char *in); /* (in) char *in; */
/* mwpl_io.c:211:NF */ extern char *getprintfstring (char *s); /* (s) char *s; */
/* mwpl_io.c:246:NF */ extern void removespaces (char *in); /* (in) char *in; */
/* mwpl_io.c:290:OF */ extern int skipline (void); /* () */
/* mwpl_io.c:319:NF */ extern int getline (char *line); /* (line) char *line; */
/* mwpl_io.c:344:OF */ extern void skipcomment (void); /* () */
/* mwpl_io.c:381:NF */ extern int getsentence (char *s); /* (s) char *s; */
/* mwpl_io.c:442:OF */ extern void skipblock (void); /* () */
/* mwpl_io.c:523:NF */ extern int getinstruction (char *s, long int *lbeg, long int *lend); /* (s, lbeg, lend) char *s; long int *lbeg; long int *lend; */
/* mwpl_io.c:658:NF */ extern int removebraces (char *in, char *out); /* (in, out) char *in; char *out; */
/* mwpl_io.c:692:NF */ extern void RemoveTerminatingSpace (char *in); /* (in) char *in; */
/* mwpl_io.c:716:NF */ extern int getenclosedstring (char *in, char *out); /* (in, out) char *in; char *out; */
/* mwpl_io.c:745:OF */ extern int getenvironmentvar (void); /* () */
/* mwpl_io.c:770:OF */ extern int getcurrentworkingdir (void); /* () */
/* mwpl_io.c:813:OF */ extern int getgroupname (void); /* () */
/* mwpl_io.c:858:NF */ extern int getword (char *s, char *w); /* (s, w) char *s; char *w; */
/* mwpl_io.c:896:NF */ extern int getCid (char *s, char *cid); /* (s, cid) char *s; char *cid; */
/* mwpl_io.c:933:NF */ extern int getInterval (char *s, char *min, char *max, int *ai); /* (s, min, max, ai) char *s; char *min; char *max; int *ai; */
/* mwpl_io.c:1003:NF */ extern int IsStringCid (char *s); /* (s) char *s; */
/* mwpl_io.c:1035:NF */ extern void WriteFuncPrototype (FILE *fd, VarFunc *f, int ansi); /* (fd, f, ansi) FILE *fd; VarFunc *f; int ansi; */
/* mwpl_io.c:1079:NF */ extern void fprinttex (FILE *fd, char *fmt, ...); /* (fd, fmt) FILE *fd; char *fmt; */
/* mwpl_mfile.c:39:OF */ extern void writeMheader (void); /* () */
/* mwpl_mfile.c:54:NF */ extern void writeMVarFunc (VarFunc *f); /* (f) VarFunc *f; */
/* mwpl_mfile.c:80:OF */ extern void writeMbody (void); /* () */
/* mwpl_mfile.c:115:OF */ extern void genMfile (void); /* () */
/* mwpl_parse.c:52:OF */ extern int EnterHeader (void); /* () */
/* mwpl_parse.c:81:OF */ extern int ParseHeader (void); /* () */
/* mwpl_parse.c:136:OF */ extern void ParseCbody (void); /* () */
/* mwpl_parse.c:166:OF */ extern void CompleteHC (void); /* () */
/* mwpl_parse.c:236:OF */ extern void parse (void); /* () */
/* mwpl_present.c:38:NF */ extern void xprint (char c, short int n); /* (c, n) char c; short int n; */
/* mwpl_present.c:52:NF */ extern void cutstring (char *Ch1, char *Ch2, char *Ch, short int l); /* (Ch1, Ch2, Ch, l) char *Ch1; char *Ch2; char *Ch; short int l; */
/* mwpl_present.c:78:NF */ extern int module_presentation (char *Prog, char *Vers, char *Auth, char *Func, char *Lab); /* (Prog, Vers, Auth, Func, Lab) char *Prog; char *Vers; char *Auth; char *Func; char *Lab; */
/* mwpl_tfile.c:36:OF */ extern void writeTheader (void); /* () */
/* mwpl_tfile.c:71:OF */ extern void writeSynopsis (void); /* () */
/* mwpl_tfile.c:142:NF */ extern void printtex_interval_and_defval (Arg *a); /* (a) Arg *a; */
/* mwpl_tfile.c:188:OF */ extern void writeArguments (void); /* () */
/* mwpl_tfile.c:271:OF */ extern void writeSummary (void); /* () */
/* mwpl_tfile.c:310:OF */ extern void writeDescription (void); /* () */
/* mwpl_tfile.c:331:OF */ extern void writeTfoot (void); /* () */
/* mwpl_tfile.c:372:OF */ extern void genTfile (void); /* () */
/* mwpl_tree.c:34:OF */ extern Arg *new_arg (void); /* () */
/* mwpl_tree.c:64:OF */ extern Header *new_header (void); /* () */
/* mwpl_tree.c:93:NF */ extern void dump_arg (Arg *a); /* (a) Arg *a; */
/* mwpl_tree.c:123:OF */ extern void CheckConsistencyH (void); /* () */
/* mwpl_tree.c:239:OF */ extern Variable *new_variable (void); /* () */
/* mwpl_tree.c:268:OF */ extern VarFunc *new_varfunc (void); /* () */
/* mwpl_tree.c:293:OF */ extern Cbody *new_cbody (void); /* () */
/* mwpl_tree.c:313:NF */ extern void dump_variable (Variable *v); /* (v) Variable *v; */
/* mwpl_tree.c:334:NF */ extern void dump_varfunc (VarFunc *f); /* (f) VarFunc *f; */
/* mwpl_tree.c:371:OF */ extern Cword *new_cword (void); /* () */
/* mwpl_tree.c:391:OF */ extern Cinstruction *new_cinstruction (void); /* () */
/* mwpl_tree.c:414:NF */ extern void delete_cinstruction (Cinstruction *c); /* (c) Cinstruction *c; */
/* mwpl_tree.c:442:NF */ extern void merge_cinstruction (Cinstruction *c); /* (c) Cinstruction *c; */
