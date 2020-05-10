#include "tree.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*************************************************************************************************/
/* Static helper functions                                                                       */
/*************************************************************************************************/

static void* _xalloc(size_t size)
{
    void* p = malloc(size);
    if (!p)
    {
        fprintf(stderr, "Error: malloc(): %s\n", strerror(errno));
        exit(1);
    }

    return p;
}



static expr_t* _make_expr(int lineno)
{
    expr_t* expr = (expr_t*) _xalloc(sizeof(expr_t));
    expr->lineno = lineno;
    return expr;
}

static decl_t* _make_decl(int lineno)
{
    decl_t* decl = (decl_t*) _xalloc(sizeof(decl_t));
    decl->lineno = lineno;
    return decl;
}

static stmt_t* _make_stmt(int lineno)
{
    stmt_t* stmt = (stmt_t*) _xalloc(sizeof(stmt_t));
    stmt->lineno = lineno;
    return stmt;
}

static type_t* _make_type(int lineno)
{
    type_t* type = (type_t*) _xalloc(sizeof(type_t));
    type->lineno = lineno;
    return type;
}

static arrayindex_t* _make_arrayindex()
{
    struct arrayindex* index = _xalloc(sizeof(struct arrayindex));
    return index;
}

static expr_t* _make_literalexpr(int lineno)
{
    expr_t* expr = (expr_t*) _xalloc(sizeof(expr_t));
    expr->lineno = lineno;
    expr->kind = k_LITERALEXPR;
    return expr;
}


/*************************************************************************************************/
/* AST node constructor functions (kind = k_EXPRNODE)                                            */
/*************************************************************************************************/

expr_t* make_identexpr(int lineno, char* value)
{
    expr_t* expr = _make_expr(lineno);

    expr->kind = k_IDENTEXPR;
    expr->identexpr.value = value;

    return expr;
}

expr_t* make_literalexpr_intval(int lineno, int intval)
{
    expr_t* expr = _make_literalexpr(lineno);

    expr->literalexpr.kind = k_INTLITERALEXPR;
    expr->literalexpr.intval = intval;

    return expr;
}

expr_t* make_literalexpr_fltval(int lineno, float fltval)
{
    expr_t* expr = _make_literalexpr(lineno);

    expr->literalexpr.kind = k_FLTLITERALEXPR;
    expr->literalexpr.fltval = fltval;

    return expr;
}

expr_t* make_literalexpr_strval(int lineno, char* strval)
{
    expr_t* expr = _make_literalexpr(lineno);

    expr->literalexpr.kind = k_STRLITERALEXPR;
    expr->literalexpr.strval = strval;

    return expr;
}

expr_t* make_literalexpr_chrval(int lineno, char* chrval)
{
    expr_t* expr = _make_literalexpr(lineno);

    expr->literalexpr.kind = k_CHRLITERALEXPR;
    expr->literalexpr.chrval = chrval;

    return expr;
}

expr_t* make_unaryexpr(int lineno, enum unaryexprkind kind, expr_t* expr)
{
    expr_t* uexpr = _make_expr(lineno);

    uexpr->kind = k_UNARYEXPR;
    uexpr->unaryexpr.kind = kind;
    uexpr->unaryexpr.expr = expr;

    return uexpr;
}

expr_t* make_binaryexpr(int lineno, enum binaryexprkind kind, expr_t* l_expr, expr_t* r_expr)
{
    expr_t* expr = _make_expr(lineno);

    expr->kind = k_BINARYEXPR;
    expr->binaryexpr.kind = kind;
    expr->binaryexpr.l_expr = l_expr;
    expr->binaryexpr.r_expr = r_expr;

    return expr;
}

expr_t* make_funccallexpr(int lineno, expr_t* name, expr_t* args)
{
    expr_t* expr = _make_expr(lineno);

    expr->kind = k_FUNCCALLEXPR;
    expr->funccallexpr.name = name;
    expr->funccallexpr.args = args;

    return expr;
}

expr_t* make_builtincallexpr(int lineno, expr_t* name, expr_t* args)
{
    expr_t* expr = _make_expr(lineno);

    expr->kind = k_BUILTINCALLEXPR;
    expr->funccallexpr.name = name;
    expr->funccallexpr.args = args;

    return expr;
}

expr_t* make_indexingexpr(int lineno, expr_t* array, expr_t* index)
{
    expr_t* expr = _make_expr(lineno);

    expr->kind = k_INDEXINGEXPR;
    expr->indexingexpr.array = array;
    expr->indexingexpr.index = index;

    return expr;
}

expr_t* make_fieldselectorexpr(int lineno, expr_t* receiver, expr_t* selector)
{
    expr_t* expr = _make_expr(lineno);

    expr->kind = k_FIELDSELECTOREXPR;
    expr->fieldselectorexpr.receiver = receiver;
    expr->fieldselectorexpr.selector = selector;

    return expr;
}

expr_t* make_seqexpr(int lineno, expr_t* main, expr_t* next)
{
    expr_t* expr = _make_expr(lineno);

    expr->kind = k_SEQEXPR;
    expr->seqexpr.main = main;
    expr->seqexpr.next = next;

    return expr;
}

expr_t* make_parenthesesexpr(int lineno, expr_t* main)
{
    expr_t* expr = _make_expr(lineno);

    expr->kind = k_PARENTHESESEXPR;
    expr->parenthesesexpr.main = main;

    return expr;
}


/*************************************************************************************************/
/* AST node constructor functions (kind = k_DECLNODE)                                            */
/*************************************************************************************************/

decl_t* make_seqdecl(int lineno, decl_t* decl, decl_t* next)
{
    decl_t* seqdecl = _make_decl(lineno);

    seqdecl->kind = k_SEQDECL;
    seqdecl->seqdecl.decl = decl;
    seqdecl->seqdecl.next = next;

    return seqdecl;
}
decl_t* make_pkgdecl(int lineno, char* name)
{
    decl_t* decl = _make_decl(lineno);

    decl->kind = k_PKGDECL;
    decl->pkgdecl.name = name;

    return decl;
}

decl_t* make_identvardecl(int lineno, expr_t* names, expr_t* exprs, type_t* type)
{
    decl_t* decl = _make_decl(lineno);
    
    decl->kind = k_VARDECL;
    decl->vardecl.kind = k_IDENTVARDECL;
    decl->vardecl.names = names;
    decl->vardecl.exprs = exprs;
    decl->vardecl.type = type;

    return decl;
}

decl_t* make_funcvardecl(int lineno, expr_t* names, type_t* type)
{
    decl_t* decl = _make_decl(lineno);

    decl->kind = k_VARDECL;
    decl->vardecl.kind = k_FUNCVARDECL;
    decl->vardecl.names = names;
    decl->vardecl.type = type;

    return decl;
}

decl_t* make_typedecl(int lineno, char* name, type_t* type)
{
    decl_t* decl = _make_decl(lineno);

    decl->kind = k_TYPEDECL;
    decl->typedecl.name = name;
    decl->typedecl.type = type;

    return decl;
}

decl_t* make_funcdecl(int lineno, char* name, type_t* returntype, decl_t* args, stmt_t* stmts)
{
    decl_t* decl = _make_decl(lineno);

    decl->kind = k_FUNCDECL;
    decl->funcdecl.name = name;
    decl->funcdecl.returntype = returntype;
    decl->funcdecl.args = args;
    decl->funcdecl.stmts = stmts;

    return decl;
}

void mark_struct_vardecl(decl_t* declnode)
{
    if (declnode == NULL) return;

    decl_t* node = declnode;
    while (node != NULL) 
    {
        decl_t* decl = node->seqdecl.decl;
        if (decl->kind == k_VARDECL) 
        {
            decl->vardecl.kind = k_STRUCTVARDECL;
        }
        else
        {
            fprintf(stderr, "Type declaration inside struct has the wrong type.\n");
            exit(1);
        }
        node = node->seqdecl.next;
    }
}

/*************************************************************************************************/
/* AST node constructor functions (kind = k_STMTNODE)                                            */
/*************************************************************************************************/

stmt_t* make_seqstmt(int lineno, stmt_t* stmt, stmt_t* next)
{
    stmt_t* seqstmt = _make_stmt(lineno);

    seqstmt->kind = k_SEQSTMT;
    seqstmt->seqstmt.stmt = stmt;
    seqstmt->seqstmt.next = next;

    return seqstmt;    
}

stmt_t* make_exprstmt(int lineno, expr_t* expr)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_EXPRSTMT;
    stmt->exprstmt.expr = expr;

    return stmt;
}

stmt_t* make_blockstmt(int lineno, stmt_t* stmts)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_BLOCKSTMT;
    stmt->blockstmt.stmts = stmts;

    return stmt;
}

stmt_t* make_assignstmt(int lineno, expr_t* names, expr_t* exprs)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_ASSIGNSTMT;
    stmt->assignstmt.names = names;
    stmt->assignstmt.exprs = exprs;

    return stmt;
}

stmt_t* make_opassignstmt(int lineno, enum opassignstmtkind kind, expr_t* name, expr_t* expr)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_OPASSIGNSTMT;
    stmt->opassignstmt.kind = kind;
    stmt->opassignstmt.name = name;
    stmt->opassignstmt.expr = expr;

    return stmt;
}

stmt_t* make_declstmt(int lineno, decl_t* decl)
{
    if(decl == NULL)
        return NULL;
    stmt_t* stmt = _make_stmt(lineno);
    stmt->kind = k_DECLSTMT;
    stmt->declstmt = *decl;
    
    return stmt;
}

stmt_t* make_shortdeclstmt(int lineno, expr_t* names, expr_t* exprs)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_SHORTDECLSTMT;
    stmt->shortdeclstmt.names = names;
    stmt->shortdeclstmt.exprs = exprs;

    return stmt;
}

stmt_t* make_incrementstmt(int lineno, expr_t* expr)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_INCREMENTSTMT;
    stmt->incrementstmt.expr = expr;

    return stmt;
}

stmt_t* make_decrementstmt(int lineno, expr_t* expr)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_DECREMENTSTMT;
    stmt->decrementstmt.expr = expr;

    return stmt;
}

stmt_t* make_returnstmt(int lineno, expr_t* expr)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_RETURNSTMT;
    stmt->returnstmt.expr = expr;

    return stmt;
}

stmt_t* make_ifstmt(int lineno, stmt_t* init, expr_t* condition, stmt_t* thenblock, stmt_t* elseblock)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_IFSTMT;
    stmt->ifstmt.init = init;
    stmt->ifstmt.condition = condition;
    stmt->ifstmt.thenblock = thenblock;
    stmt->ifstmt.elseblock = elseblock;

    return stmt;
}

stmt_t* make_switchstmt(int lineno, stmt_t* init, expr_t* condition, stmt_t* cases)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_SWITCHSTMT;
    stmt->switchstmt.init = init;
    stmt->switchstmt.condition = condition;
    stmt->switchstmt.cases = cases;

    return stmt;
}

stmt_t* make_switchcasestmt(int lineno, expr_t* exprs, stmt_t* stmts)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_SWITCHCASESTMT;
    stmt->switchcasestmt.exprs = exprs;
    stmt->switchcasestmt.stmts = stmts;

    return stmt;
}

stmt_t* make_forstmt(int lineno, stmt_t* init, expr_t* condition, stmt_t* post, stmt_t* loopblock)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_FORSTMT;
    stmt->forstmt.init = init;
    stmt->forstmt.condition = condition;
    stmt->forstmt.post = post;
    stmt->forstmt.loopblock = loopblock;

    return stmt;
}

stmt_t* make_breakstmt(int lineno)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_BREAKSTMT;

    return stmt;
}

stmt_t* make_continuestmt(int lineno)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_CONTINUESTMT;

    return stmt;
}

stmt_t* make_emptystmt(int lineno)
{
    stmt_t* stmt = _make_stmt(lineno);

    stmt->kind = k_EMPTYSTMT;

    return stmt;
}

/*************************************************************************************************/
/* AST node constructor functions (kind = k_EXPRNODE)                                            */
/*************************************************************************************************/

type_t* make_simpletype(int lineno, char* name)
{
    type_t* type = _make_type(lineno);

    type->kind = k_SIMPLETYPE;
    type->name = name;
    type->array_type = 0;
    type->dimensions = NULL;

    return type;
}

type_t* make_structtype(int lineno, decl_t* structdef)
{
    type_t* type = _make_type(lineno);

    type->kind = k_STRUCTTYPE;
    type->structdef = structdef;
    type->array_type = 0;
    type->dimensions = NULL;

    return type;
}

type_t* add_arraydimension(type_t* type, int size)
{
    struct arrayindex* dimension = _make_arrayindex();
    dimension->index = size;
    dimension->next = type->dimensions;
    type->dimensions = dimension;
    type->array_type = 1;
    return type;
}
