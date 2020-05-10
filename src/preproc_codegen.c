#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "preproc_codegen.h"
#include "symbol.h"
#include "tree.h"

/*Func declaration */

static void _preproc_decl(decl_t* decl);
static void _preproc_funcdecl(decl_t* decl);

static void _preproc_expr(expr_t* expr);
static void _preproc_funccallexpr(expr_t* expr);
static void _preproc_funccallexpr_funccall(expr_t* expr);
static void _preproc_builtincallexpr(expr_t* expr);
static void _preproc_builtincallexpr_cap(expr_t* expr);
static void _preproc_builtincallexpr_len(expr_t* expr);
static void _preproc_indexingexpr(expr_t* expr);
static void _preproc_seqexpr(expr_t* expr);
static int _preproc_is_slice_identifier(expr_t* expr);


static void _preproc_stmt(stmt_t* stmt);
static void _preproc_exprstmt(stmt_t* stmt);
static void _preproc_blockstmt(stmt_t* stmt);
static void _preproc_assignstmt(stmt_t* stmt);
static void _preproc_opassignstmt(stmt_t* stmt);
static void _preproc_declstmt(stmt_t* stmt);
static void _preproc_shortdeclstmt(stmt_t* stmt);
static void _preproc_returnstmt(stmt_t* stmt);
static void _preproc_ifstmt(stmt_t* stmt);
static void _preproc_switchstmt(stmt_t* stmt);
static void _preproc_switchcasestmt(stmt_t* stmt);
static void _preproc_forstmt(stmt_t* stmt);

// global variables
symbol_t* currentFunction = NULL;

/*************************************************************************************************/
/* preproc function                                                                            */
/*************************************************************************************************/

void preproc(decl_t* root)
{
    _preproc_decl(root);
    _preproc_decl(root);
}

/*************************************************************************************************/
/* preproc helper functions                                                                            */
/*************************************************************************************************/

static expr_t* _preproc_pop_parentheses(expr_t* expr)
{
    while(expr->kind == k_PARENTHESESEXPR)
        expr = expr->parenthesesexpr.main;
    return expr;
}

static int _preproc_is_slice_identifier(expr_t* expr)
{
    expr = _preproc_pop_parentheses(expr);
    return expr->resolved_type->isarray && expr->resolved_type->dimensions->index == -1
            && expr->kind == k_IDENTEXPR;
}

/*************************************************************************************************/
/* preproc for declarations                                                                      */
/*************************************************************************************************/

static void _preproc_decl(decl_t* decl)
{
    if(decl == NULL)
        return;

    switch(decl->kind)
    {
        case k_PKGDECL:
            break;
        case k_VARDECL:
            break;
        case k_TYPEDECL:
            break;
        case k_FUNCDECL:
            _preproc_funcdecl(decl);
            break;
        case k_SEQDECL:
            _preproc_decl(decl->seqdecl.decl);
            _preproc_decl(decl->seqdecl.next);
            break;
        default:
            fprintf(stderr, "Undefined declaration kind. \n");
            exit(1);
    }
}

static void _preproc_funcdecl(decl_t* decl)
{
    if(decl == NULL)
        return;
    currentFunction = decl->funcdecl.symbol;
    _preproc_stmt(decl->funcdecl.stmts);
    currentFunction = NULL;
}


/*************************************************************************************************/
/* preproc for expressions                                                                            */
/*************************************************************************************************/

static void _preproc_expr(expr_t* expr)
{
    if(expr == NULL)
        return;
    switch(expr->kind)
    {
        case k_IDENTEXPR:
            break;
        case k_LITERALEXPR:
            break;
        case k_UNARYEXPR:
            _preproc_expr(expr->unaryexpr.expr);
            break;
        case k_BINARYEXPR:
            _preproc_expr(expr->binaryexpr.l_expr);
            _preproc_expr(expr->binaryexpr.r_expr);
            break;
        case k_FUNCCALLEXPR:
            _preproc_funccallexpr(expr);
            break;
        case k_BUILTINCALLEXPR:
            _preproc_builtincallexpr(expr);
            break;
        case k_INDEXINGEXPR:
            _preproc_indexingexpr(expr);
            break;
        case k_FIELDSELECTOREXPR:
            break;
        case k_SEQEXPR:
            _preproc_seqexpr(expr);
            break;
        case k_PARENTHESESEXPR:
            _preproc_expr(expr->parenthesesexpr.main);
            break;    
        default:    
            fprintf(stderr, "Undefined expression kind. \n");
            exit(1);
    }
}

static void _preproc_funccallexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    switch (expr->funccallexpr.symbol->kind)
    {
        case k_FUNCTION:
            _preproc_funccallexpr_funccall(expr);
            break;
        case k_TYPE:
            break;
        case k_VARIABLE:
            fprintf(stderr, "Error: [line %d] Function call expression on non-function type.\n", expr->lineno);
            exit(1);
        case k_CONSTANT:
            fprintf(stderr, "Error: [line %d] Function call expression on non-function type.\n", expr->lineno);
            exit(1);
        default:
            fprintf(stderr, "Error: [line %d] Corrupted symbol type.\n", expr->lineno);
            exit(1);
    }
}

static void _preproc_funccallexpr_funccall(expr_t* expr)
{
    if(expr == NULL)
        return;
    /* If the called function requires that the argument be a slice
       we set the element properly
    */
    typelist_t* funcarg = expr->funccallexpr.symbol->funcinfo;
    expr_t* args = expr->funccallexpr.args; 
    // The length of args and funcsym should match

    while(args != NULL)
    {
        expr_t* arg = _preproc_pop_parentheses(args->seqexpr.main);
        if(arg->kind == k_IDENTEXPR && funcarg->current->requiredslice)
        {
            arg->identexpr.symbol->type.requiredslice = true;
        }
        args = args->seqexpr.next;
        funcarg = funcarg->next;
    }
}

static void _preproc_builtincallexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    expr_t* name = expr->funccallexpr.name;
    
    if(strcmp(name->identexpr.value, "cap") == 0){
        _preproc_builtincallexpr_cap(expr);
    } else if(strcmp(name->identexpr.value, "len") == 0){
        _preproc_builtincallexpr_len(expr);
    } else {
        _preproc_expr(expr->funccallexpr.args);
    }
}


static void _preproc_builtincallexpr_cap(expr_t* expr){
    
    expr_t* element = _preproc_pop_parentheses(expr->funccallexpr.args->seqexpr.main);
    if(_preproc_is_slice_identifier(element)){
        element->identexpr.symbol->type.requiredslice = true;
    }
    
}

static void _preproc_builtincallexpr_len(expr_t* expr)
{
    expr_t* element = _preproc_pop_parentheses(expr->funccallexpr.args->seqexpr.main);
    if(_preproc_is_slice_identifier(element)){
        element->identexpr.symbol->type.requiredslice = true;
    }
}

static void _preproc_indexingexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    expr_t* array = _preproc_pop_parentheses(expr->indexingexpr.array);
    expr_t* index = _preproc_pop_parentheses(expr->indexingexpr.index);
    _preproc_expr(array);
    _preproc_expr(index);
    if(array->resolved_type->requiredslice &&
       expr->resolved_type->isarray &&
       expr->resolved_type->dimensions->index == -1){
        
        expr->resolved_type->requiredslice = true;
    }
}

static void _preproc_seqexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    _preproc_expr(expr->seqexpr.main);
    _preproc_expr(expr->seqexpr.next);
}

/*************************************************************************************************/
/* preproc for statements                                                                            */
/*************************************************************************************************/

static void _preproc_stmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    
    switch(stmt->kind)
    {
        case k_EXPRSTMT: 
            _preproc_exprstmt(stmt);
            break;
        case k_BLOCKSTMT: 
            _preproc_blockstmt(stmt);
            break;
        case k_ASSIGNSTMT: 
            _preproc_assignstmt(stmt);
            break;
        case k_OPASSIGNSTMT: 
            _preproc_opassignstmt(stmt);
            break;
        case k_DECLSTMT: 
            _preproc_declstmt(stmt);
            break;
        case k_SHORTDECLSTMT: 
            _preproc_shortdeclstmt(stmt);
            break;
        case k_INCREMENTSTMT: 
            break;
        case k_DECREMENTSTMT: 
            break;
        case k_RETURNSTMT: 
            _preproc_returnstmt(stmt);
            break;
        case k_IFSTMT: 
            _preproc_ifstmt(stmt);
            break;
        case k_SWITCHSTMT: 
            _preproc_switchstmt(stmt);
            break;
        case k_SWITCHCASESTMT:
            _preproc_switchcasestmt(stmt);
            break;
        case k_FORSTMT: 
            _preproc_forstmt(stmt);
            break;
        case k_BREAKSTMT: 
            break;
        case k_CONTINUESTMT: 
            break;
        case k_EMPTYSTMT: 
            break;
        case k_SEQSTMT: 
            _preproc_stmt(stmt->seqstmt.stmt);
            _preproc_stmt(stmt->seqstmt.next);
            break;
        default:
            fprintf(stderr, "Undefined statement kind. \n");
            exit(1);
    }
}

static void _preproc_exprstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    // TODO: deal with func calls.
    expr_t* expr = stmt->exprstmt.expr;
    if(expr->kind == k_FUNCCALLEXPR)
    {
        typelist_t* typelist = expr->funccallexpr.symbol->funcinfo;
        expr_t* arglist = expr->funccallexpr.args;

        while(typelist != NULL)
        {
            expr_t* arg = _preproc_pop_parentheses(arglist->seqexpr.main);

            if(arg->kind == k_IDENTEXPR && typelist->current->requiredslice){
                arg->identexpr.symbol->type.requiredslice = true;
            }
            arglist = arglist->seqexpr.next;
            typelist = typelist->next;
        }
    }

    _preproc_expr(expr->funccallexpr.args);
}

static void _preproc_blockstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    _preproc_stmt(stmt->blockstmt.stmts);
}

static bool _preproc_is_function_argument(expr_t* expr)
{
    symboltype_t* symtype = &(expr->identexpr.symbol->type);
    typelist_t* funcargs = currentFunction->funcinfo;
    while(funcargs != NULL)
    {
        if(symtype == funcargs->current)
            return true;
        funcargs = funcargs->next;
    }
    return false;
}

static int _preproc_same_literal(expr_t* expr1, expr_t* expr2)
{
    if(expr1 == NULL || expr2 == NULL || expr1->literalexpr.kind != expr2->literalexpr.kind)
        return 0;

    switch(expr1->literalexpr.kind)
    {
        case k_INTLITERALEXPR:
            return expr1->literalexpr.intval == expr2->literalexpr.intval;
        case k_FLTLITERALEXPR:
            return expr1->literalexpr.fltval == expr2->literalexpr.fltval;
        case k_STRLITERALEXPR:
            return strcmp(expr1->literalexpr.strval, expr2->literalexpr.strval);
        case k_CHRLITERALEXPR:
            return strcmp(expr1->literalexpr.chrval, expr2->literalexpr.chrval);
        default:    
            return 0;
    }
}
static int _preproc_same_expr(expr_t* expr1, expr_t* expr2)
{
    if(expr1->kind != expr2->kind)
        return 0;
    switch(expr1->kind)
    {
        case k_IDENTEXPR:   
            return strcmp(expr1->identexpr.value, expr2->identexpr.value) == 0;
        case k_LITERALEXPR:
            return _preproc_same_literal(expr1, expr2);
        case k_INDEXINGEXPR:    
            return _preproc_same_expr(expr1->indexingexpr.array, expr2->indexingexpr.array)
                   && _preproc_same_expr(expr1->indexingexpr.index, expr2->indexingexpr.index);
        case k_FIELDSELECTOREXPR:   
            return _preproc_same_expr(expr1->fieldselectorexpr.receiver, expr2->fieldselectorexpr.receiver)
                   && _preproc_same_expr(expr1->fieldselectorexpr.selector, expr2->fieldselectorexpr.selector);
        default: return 0;
    }
}

static void _preproc_assignstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    expr_t* names = stmt->assignstmt.names;
    expr_t* exprs = stmt->assignstmt.exprs;

    while(names != NULL)
    {
        expr_t* name = _preproc_pop_parentheses(names->seqexpr.main);
        expr_t* expr = _preproc_pop_parentheses(exprs->seqexpr.main);

        expr_t* funcname = expr->funccallexpr.name;
        /*
        *   A slice needs to be used as a slice when the following happens:
        *   x = append(x, smt) when x is one of the arguments in the current function
        *   y = append(x ,smt) 
        *   y = x 
        */
        if( expr->resolved_type->isarray 
            && expr->resolved_type->dimensions->index == -1 && expr->kind == k_BUILTINCALLEXPR
            && strcmp("append",funcname->identexpr.value) == 0 )
        {
            expr_t* first_funcarg = _preproc_pop_parentheses(expr->funccallexpr.args->seqexpr.main);
            expr_t* second_funcarg = _preproc_pop_parentheses(expr->funccallexpr.args->seqexpr.next->seqexpr.main);
            if( !_preproc_same_expr(name, first_funcarg)  
                || _preproc_is_function_argument(first_funcarg))
            {
                first_funcarg->resolved_type->requiredslice = true;
                name->resolved_type->requiredslice = true;
            }
            if(first_funcarg->resolved_type->requiredslice)
                second_funcarg->resolved_type->requiredslice = true;         
        } else if (name->kind == k_IDENTEXPR && name->resolved_type->isarray 
            && name->resolved_type->dimensions->index == -1 && expr->kind == k_IDENTEXPR ){
                expr->identexpr.symbol->type.requiredslice = true;
                name->identexpr.symbol->type.requiredslice = true;
        } else {
            
            _preproc_expr(name);
            _preproc_expr(expr);
        }
        names = names->seqexpr.next;
        exprs = exprs->seqexpr.next;
    }
}

static void _preproc_opassignstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    expr_t* name = _preproc_pop_parentheses(stmt->opassignstmt.name);
    expr_t* expr = _preproc_pop_parentheses(stmt->opassignstmt.expr);
    _preproc_expr(name);
    _preproc_expr(expr);
}   

static void _preproc_declstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    _preproc_decl(&stmt->declstmt);
    
}

static void _preproc_shortdeclstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    // should be treated the same as assignments
    stmt_t* temp = make_assignstmt(0, stmt->shortdeclstmt.names, stmt->shortdeclstmt.exprs);
    _preproc_assignstmt(temp);
    free(temp);
}

static void _preproc_returnstmt(stmt_t* stmt)
{
    if(stmt == NULL || stmt->returnstmt.expr == NULL)
        return;
    expr_t* expr = _preproc_pop_parentheses(stmt->returnstmt.expr);
    if(_preproc_is_slice_identifier(expr)){
        expr->identexpr.symbol->type.requiredslice = true;
    }
}

static void _preproc_ifstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    _preproc_stmt(stmt->ifstmt.init);
    _preproc_stmt(stmt->ifstmt.thenblock);
    _preproc_stmt(stmt->ifstmt.elseblock);
}

static void _preproc_switchstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;

    _preproc_stmt(stmt->switchstmt.init);
    _preproc_stmt(stmt->switchstmt.cases);
}

static void _preproc_switchcasestmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    _preproc_stmt(stmt->switchcasestmt.stmts);
}

static void _preproc_forstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    _preproc_stmt(stmt->forstmt.init);
    _preproc_expr(stmt->forstmt.condition);
    _preproc_stmt(stmt->forstmt.post);
    _preproc_stmt(stmt->forstmt.loopblock);
}

