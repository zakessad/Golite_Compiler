#ifndef TREE_H
#define TREE_H


#ifdef __cplusplus
extern "C"
{
#endif


/*************************************************************************************************/
/* Forward declarations                                                                          */
/*************************************************************************************************/

struct expr;
struct stmt;
struct type;
struct decl;
struct symbol;

/*************************************************************************************************/
/* AST `struct expr'-related data structures                                                     */
/*************************************************************************************************/

struct identexpr
{
    struct symbol* symbol;
    char* value;
};


enum literalexprkind
{
    k_INTLITERALEXPR = 1,
    k_FLTLITERALEXPR,
    k_STRLITERALEXPR,
    k_CHRLITERALEXPR,
};

struct literalexpr
{
    enum literalexprkind kind;

    union
    {
        int intval;
        double fltval;
        char* strval;
        char* chrval;
    };
};


enum unaryexprkind
{
    k_POSUNARYEXPR = 1,
    k_NEGUNARYEXPR,
    k_NOTUNARYEXPR,
    k_BITNOTUNARYEXPR
};

struct unaryexpr
{
    enum unaryexprkind kind;

    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* expr;
};


enum binaryexprkind
{
    k_ADDBINARYEXPR = 1,
    k_SUBBINARYEXPR,
    k_MULBINARYEXPR,
    k_DIVBINARYEXPR,
    k_MODBINARYEXPR,
    k_LSHIFTBINARYEXPR,
    k_RSHIFTBINARYEXPR,
    k_BITWISEANDBINARYEXPR,
    k_BITWISEORBINARYEXPR,
    k_BITWISEXORBINARYEXPR,
    k_BITWISECLRBINARYEXPR,
    k_EQBINARYEXPR,
    k_NEBINARYEXPR,
    k_LTBINARYEXPR,
    k_LEBINARYEXPR,
    k_GTBINARYEXPR,
    k_GEBINARYEXPR,
    k_LOGICALANDBINARYEXPR,
    k_LOGICALORBINARYEXPR
};

struct binaryexpr
{
    enum binaryexprkind kind;

    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* l_expr;
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* r_expr;
};


struct funccallexpr
{
    struct expr* name;

    /* Valid AST program kind(s) => k_EXPRprogram (for all programs) */
    struct expr* args;
    struct symbol* symbol;
};


struct indexingexpr
{
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* array;
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* index;
};


struct fieldselectorexpr
{
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* receiver;
    /* Valid AST program kind(s) => k_EXPRprogram + k_IDENTEXPR */
    struct expr* selector;
};

struct seqexpr
{
    struct expr* main;
    struct expr* next;
};

struct parenthesesexpr
{
    struct expr* main;
};

enum exprkind
{
    k_IDENTEXPR = 1,
    k_LITERALEXPR,
    k_UNARYEXPR,
    k_BINARYEXPR,
    k_FUNCCALLEXPR,
    k_BUILTINCALLEXPR,
    k_INDEXINGEXPR,
    k_FIELDSELECTOREXPR,
    k_SEQEXPR,
    k_PARENTHESESEXPR
};

struct expr
{
    enum exprkind kind;
    int lineno;
    struct symboltype* resolved_type;
    union
    {
        struct identexpr identexpr;
        struct literalexpr literalexpr;
        struct unaryexpr unaryexpr;
        struct binaryexpr binaryexpr;
        struct funccallexpr funccallexpr;
        struct indexingexpr indexingexpr;
        struct fieldselectorexpr fieldselectorexpr;
        struct seqexpr seqexpr;
        struct parenthesesexpr parenthesesexpr;
    };
};


/*************************************************************************************************/
/* AST `struct decl'-related data structures                                                     */
/*************************************************************************************************/

struct pkgdecl
{
    char* name;
};

enum vardeclkind 
{
    k_FUNCVARDECL = 1,
    k_IDENTVARDECL,
    k_STRUCTVARDECL
};

struct vardecl
{
    enum vardeclkind kind;
    struct expr* names;
    struct expr* exprs;
    struct type* type;
};

struct typedecl
{
    /* Valid AST program kind(s) => k_DECLprogram + k_VARDECL (for all programs) */
    struct symbol* symbol;
    char* name;
    struct type* type;
};

struct funcdecl
{
    struct symbol* symbol;
    char* name;
    struct type* returntype;

    /* Valid AST program kind(s) => k_DECLprogram + k_VARDECL (for all programs) */
    struct decl* args;
    /* Valid AST program kind(s) => k_STMTprogram (for all programs) */
    struct stmt* stmts;
};

struct seqdecl{
    struct decl* decl;
    struct decl* next;
};

enum declkind
{
    k_PKGDECL = 1,
    k_VARDECL,
    k_TYPEDECL,
    k_FUNCDECL,
    k_SEQDECL
};

struct decl
{
    enum declkind kind;
    int lineno;
    union
    {
        struct pkgdecl pkgdecl;
        struct vardecl vardecl;
        struct typedecl typedecl;
        struct funcdecl funcdecl;
        struct seqdecl seqdecl;
    };
};


/*************************************************************************************************/
/* AST `struct stmt'-related data structures                                                     */
/*************************************************************************************************/

struct blockstmt
{
    struct stmt* stmts;
};

struct assignstmt
{
    /* Valid AST program kind(s) => k_EXPRprogram + k_IDENTEXPR (for all programs) */
    struct expr* names;
    /* Valid AST program kind(s) => k_EXPRprogram (for all programs) */
    struct expr* exprs;
};


enum opassignstmtkind
{
    k_ADDASSIGN = 1,
    k_SUBASSIGN,
    k_MULASSIGN,
    k_DIVASSIGN,
    k_MODASSIGN,
    k_BITWISEANDASSIGN,
    k_BITWISEORASSIGN,
    k_NOTASSIGN,
    k_LSHIFTASSIGN,
    k_RSHIFTASSIGN,
    k_BITWISECLRASSIGN
};

struct opassignstmt
{
    enum opassignstmtkind kind;

    struct expr* name;
    struct expr* expr;
};


enum declstmtkind
{
    k_VARDECLSTMT = 1,
    k_TYPEDECLSTMT
};

struct declstmt
{
    enum declstmtkind kind;

    union
    {
        /* Valid AST program kind(s) => k_DECLprogram + k_VARDECL */
        struct decl* vardecl;
        /* Valid AST program kind(s) => k_DECLprogram + k_TYPEDECL */
        struct decl* typedecl;
    };
};


struct shortdeclstmt
{
    /* Valid AST program kind(s) => k_EXPRprogram + k_IDENTEXPR (for all programs) */
    struct expr* names;
    /* Valid AST program kind(s) => k_EXPRprogram (for all programs) */
    struct expr* exprs;
};


struct incdecstmt
{
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* expr;
};


struct returnstmt
{
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* expr;
};


struct ifstmt
{
    /* Valid AST program kind(s) => k_STMTprogram */
    struct stmt* init;
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* condition;
    /* Valid AST program kind(s) => k_STMTprogram (for all programs) */
    struct stmt* thenblock;
    /* Valid AST program kind(s) => k_STMTprogram (for all programs) */
    struct stmt* elseblock;
};


struct switchstmt
{
    /* Valid AST program kind(s) => k_STMTprogram */
    struct stmt* init;
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* condition;
    /* Valid AST program kind(s) => k_STMTprogram + k_SWITCHCASESTMT (for all programs) */
    struct stmt* cases;
};


struct switchcasestmt
{
    /* Valid AST program kind(s) => k_EXPRprogram (for all programs) */
    struct expr* exprs;
    /* Valid AST program kind(s) => k_STMTprogram (for all programs) */
    struct stmt* stmts;
};


struct forstmt
{
    /* Valid AST program kind(s) => k_STMTprogram */
    struct stmt* init;
    /* Valid AST program kind(s) => k_EXPRprogram */
    struct expr* condition;
    /* Valid AST program kind(s) => k_STMTprogram */
    struct stmt* post;
    /* Valid AST program kind(s) => k_STMTprogram (for all programs) */
    struct stmt* loopblock;
};

struct exprstmt{
    struct expr* expr;
};

struct seqstmt{
    struct stmt* stmt;
    struct stmt* next;
};

enum stmtkind
{
    k_EXPRSTMT = 1,
    k_BLOCKSTMT,
    k_ASSIGNSTMT,
    k_OPASSIGNSTMT,
    k_DECLSTMT,
    k_SHORTDECLSTMT,
    k_INCREMENTSTMT,
    k_DECREMENTSTMT,
    k_RETURNSTMT,
    k_IFSTMT,
    k_SWITCHSTMT,
    k_SWITCHCASESTMT,
    k_FORSTMT,
    k_BREAKSTMT,
    k_CONTINUESTMT,
    k_EMPTYSTMT,
    k_SEQSTMT
};

struct stmt
{
    enum stmtkind kind;
    int lineno;
    union
    {
        struct exprstmt exprstmt;
        struct blockstmt blockstmt;
        struct assignstmt assignstmt;
        struct opassignstmt opassignstmt;
        struct decl declstmt;
        struct shortdeclstmt shortdeclstmt;
        struct incdecstmt incrementstmt;
        struct incdecstmt decrementstmt;
        struct returnstmt returnstmt;
        struct ifstmt ifstmt;
        struct switchstmt switchstmt;
        struct switchcasestmt switchcasestmt;
        struct forstmt forstmt;
        struct seqstmt seqstmt;
    };
};


/*************************************************************************************************/
/* AST `struct type'-related data structures                                                     */
/*************************************************************************************************/

struct arrayindex {
    int index;
    struct arrayindex* next;
};

enum typekind
{
    k_SIMPLETYPE = 1,
    k_STRUCTTYPE
};

struct type
{
    struct symbol* symbol;
    enum typekind kind;
    int lineno;
    union {
        char* name;
        struct decl* structdef;
    };
    
    short array_type;
    struct arrayindex* dimensions;
};

/*************************************************************************************************/
/* AST types                                                                                     */
/*************************************************************************************************/

typedef struct arrayindex arrayindex_t;

typedef struct expr expr_t;
typedef struct decl decl_t;
typedef struct stmt stmt_t;
typedef struct seq seq_t;
typedef struct type type_t;


/*************************************************************************************************/
/* AST program constructor functions                                                                */
/*************************************************************************************************/

expr_t* make_identexpr(int lineno, char* value);
expr_t* make_literalexpr_intval(int lineno, int intval);
expr_t* make_literalexpr_fltval(int lineno, float fltval);
expr_t* make_literalexpr_strval(int lineno, char* strval);
expr_t* make_literalexpr_chrval(int lineno, char* chrval);
expr_t* make_unaryexpr(int lineno, enum unaryexprkind kind, expr_t* expr);
expr_t* make_binaryexpr(int lineno, enum binaryexprkind kind, expr_t* l_expr, expr_t* r_expr);
expr_t* make_funccallexpr(int lineno, expr_t* name, expr_t* args);
expr_t* make_builtincallexpr(int lineno, expr_t* name, expr_t* args);
expr_t* make_indexingexpr(int lineno, expr_t* array, expr_t* index);
expr_t* make_fieldselectorexpr(int lineno, expr_t* receiver, expr_t* selector);
expr_t* make_seqexpr(int lineno, expr_t* main, expr_t* next);
expr_t* make_parenthesesexpr(int lineno, expr_t* main);

decl_t* make_seqdecl(int lineno, decl_t* decl, decl_t* next);
decl_t* make_pkgdecl(int lineno, char* name);
decl_t* make_identvardecl(int lineno, expr_t* names, expr_t* exprs, type_t* type);
decl_t* make_funcvardecl(int lineno, expr_t* names, type_t* type);
decl_t* make_typedecl(int lineno, char* name, type_t* type);
decl_t* make_funcdecl(int lineno, char* name, type_t* returntype, decl_t* args, stmt_t* stmts);
void mark_struct_vardecl(decl_t* declnode);

stmt_t* make_seqstmt(int lineno, stmt_t* stmt, stmt_t* next);
stmt_t* make_exprstmt(int lineno, expr_t* expr);
stmt_t* make_blockstmt(int lineno, stmt_t* stmts);
stmt_t* make_assignstmt(int lineno, expr_t* names, expr_t* exprs);
stmt_t* make_opassignstmt(int lineno, enum opassignstmtkind kind, expr_t* name, expr_t* expr);
stmt_t* make_shortdeclstmt(int lineno, expr_t* names, expr_t* exprs);
stmt_t* make_incrementstmt(int lineno, expr_t* expr);
stmt_t* make_decrementstmt(int lineno, expr_t* expr);
stmt_t* make_returnstmt(int lineno, expr_t* expr);
stmt_t* make_ifstmt(int lineno, stmt_t* init, expr_t* condition, stmt_t* thenblock, stmt_t* elseblock);
stmt_t* make_switchstmt(int lineno, stmt_t* init, expr_t* condition, stmt_t* cases);
stmt_t* make_switchcasestmt(int lineno, expr_t* exprs, stmt_t* stmts);
stmt_t* make_forstmt(int lineno, stmt_t* init, expr_t* condition, stmt_t* post, stmt_t* loopblock);
stmt_t* make_breakstmt(int lineno);
stmt_t* make_continuestmt(int lineno);
stmt_t* make_emptystmt(int lineno);
stmt_t* make_declstmt(int lineno, decl_t* decl);

type_t* make_simpletype(int lineno, char* name);
type_t* make_structtype(int lineno, decl_t* structdef);
type_t* add_arraydimension(type_t* type, int size);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TREE_H */
