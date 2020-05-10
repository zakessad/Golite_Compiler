#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "weeder.h"

#define BLANK_ID_CHECK(str) (str!=NULL && (strcmp(str,"_")==0))

static void _weed_expr_node(expr_t* expr);
static void _weed_stmt_node(stmt_t* stmt);
static void _weed_decl_node(decl_t* decl);
static void _weed_package_decl_node(char* name, int lineno);
static void _weed_type_node(type_t* type);

static void _weed_var_decl_node(struct vardecl* decl, int lineno);
static void _weed_type_decl_node(struct typedecl* decl);
static void _weed_func_decl_node(struct funcdecl* decl, int lineno);
static void _weed_check_decl_struct(decl_t* decls);

static int _weed_terminating_statement(char* name, stmt_t* stmt);
static int _weed_contains_break(stmt_t* stmt);
static void _weed_error_func_return(int lineno, char* name, const char* error_type);

static void _weed_expr_stmt_node(expr_t* expr);
static void _weed_builtin_func(expr_t* expr);
static void _weed_assign_stmt_node(struct assignstmt* stmt);
static void _weed_opassign_stmt_node(struct opassignstmt* stmt);
static void _weed_shortdecl_stmt_node(struct shortdeclstmt* stmt);
static void _weed_if_stmt_node(struct ifstmt* stmt);
static void _weed_switch_stmt_node(struct switchstmt* stmt);
static void _weed_switchcase_stmt_node(struct switchcasestmt* stmt);
static void _weed_for_stmt_node(struct forstmt* stmt);


static void _weed_simple_stmt_node(stmt_t* stmt);

static int _weed_expr_list_length(expr_t* node);
static int _weed_valid_ident_list(expr_t* node);

static void _weed_check_lr_balance(expr_t* lhs, expr_t* rhs);
static int _weed_valid_lhs_list(expr_t* node);

static void _weed_blank_id_check(expr_t* node);


int allow_break = 0;
int allow_continue = 0;

void weed(decl_t* root)
{
    if(root == NULL)
        return;

    _weed_decl_node(root);

}

/*************************************************************************************************/
/* Declaration related functions                                                                 */
/*************************************************************************************************/

static void _weed_decl_node(decl_t* decl)
{
    if(decl == NULL)
        return;
    switch(decl->kind)
    {
        case k_PKGDECL:
            _weed_package_decl_node(decl->pkgdecl.name, decl->lineno);
            break;
        case k_VARDECL:
            _weed_var_decl_node(&(decl->vardecl), decl->lineno);
            break;
        case k_TYPEDECL:
            _weed_type_decl_node(&(decl->typedecl));
            break;
        case k_FUNCDECL:
            _weed_func_decl_node(&(decl->funcdecl), decl->lineno);
            break;
        case k_SEQDECL:
            _weed_decl_node(decl->seqdecl.decl);
            _weed_decl_node(decl->seqdecl.next);
            break;
        default:
            fprintf(stderr, "Error:[line %d] Unknown declaration type \n", decl->lineno);
            exit(1);            
    }
}

static void _weed_package_decl_node(char* name, int lineno)
{
    if(strcmp(name, "_") == 0 )
    {
        fprintf(stderr, "Error:[line %d] package can't be a blank identifier. \n", lineno);
        exit(1);
    }
}

static void _weed_var_decl_node(struct vardecl* decl, int lineno)
{
    if(decl == NULL)
        return;
                
    switch(decl->kind)
    {
        case k_IDENTVARDECL:
            if(decl->exprs != NULL)
            {
                _weed_blank_id_check(decl->exprs);
                _weed_check_lr_balance(decl->names, decl->exprs);
            }
            _weed_valid_ident_list(decl->names);
            _weed_type_node(decl->type);
            break;
        case k_FUNCVARDECL:
            _weed_valid_ident_list(decl->names);
            _weed_type_node(decl->type);
            break;
        default:                                                                                                                   
            fprintf(stderr,"Error: [line %d] Unknown var declaration type \n",lineno);
            exit(1);
    }
}

static void _weed_type_decl_node(struct typedecl* decl)
{
    if(decl == NULL) return;

    _weed_type_node(decl->type);
}

static void _weed_check_decl_struct(decl_t* decls)
{
    if (decls == NULL) return;

    decl_t* node = decls;
    while (node != NULL && node->seqdecl.decl != NULL)
    {
        decl_t* decl = node->seqdecl.decl;
        if (decl->kind == k_VARDECL)
        {
            if(decl->vardecl.exprs != NULL)
            {
                fprintf(stderr,"Error:[line %d] struct fields cannot be assigned inside definition. \n", decl->lineno);
                exit(1);  
            }
            if(decl->vardecl.type == NULL)
            {
                fprintf(stderr,"Error:[line %d] struct fields must have a type. \n", decl->lineno);
                exit(1);  
            }
            _weed_type_node(decl->vardecl.type);
        }
        node = node->seqdecl.next;
    }

}

static void _weed_func_decl_node(struct funcdecl* decl, int lineno)
{
    if(decl == NULL)
        return;
    _weed_type_node(decl->returntype);

    _weed_decl_node(decl->args);
    _weed_stmt_node(decl->stmts);
    if(decl->returntype != NULL && _weed_terminating_statement(decl->name, decl->stmts) == 0){
        _weed_error_func_return(lineno, decl->name, "final statement not terminating");
    }
}

static void _weed_expr_node(expr_t* expr)
{
    if(expr == NULL)
        return;

    switch (expr->kind)
    {
        case k_SEQEXPR:
            _weed_expr_node(expr->seqexpr.main);
            _weed_expr_node(expr->seqexpr.next);
            return;
        case k_BUILTINCALLEXPR:
            _weed_builtin_func(expr);
        default: 
            return;
    }
}


static void _weed_builtin_func(expr_t* expr)
{
    if(expr == NULL)
        return;
    if(expr->kind == k_BUILTINCALLEXPR)
    {
        if(expr->funccallexpr.name->kind == k_IDENTEXPR && strcmp(expr->funccallexpr.name->identexpr.value, "append") == 0)
        {   
            if(_weed_expr_list_length(expr->funccallexpr.args) != 2)
            {
                fprintf(stderr, "Error:[line %d] Appends needs to be used with two arguments \n", expr->lineno);
                exit(1);                
            }
        }
        else if(expr->funccallexpr.name->kind == k_IDENTEXPR && strcmp(expr->funccallexpr.name->identexpr.value, "len") == 0)
        {
            if(_weed_expr_list_length(expr->funccallexpr.args) != 1)
            {
                fprintf(stderr, "Error:[line %d] Len needs to be used with two arguments \n", expr->lineno);
                exit(1);                
            }
        }
        else if(expr->funccallexpr.name->kind == k_IDENTEXPR && strcmp(expr->funccallexpr.name->identexpr.value, "cap") == 0)
        {
            if(_weed_expr_list_length(expr->funccallexpr.args) != 1)
            {
                fprintf(stderr, "Error:[line %d] Cap needs to be used with two arguments \n", expr->lineno);
                exit(1);                
            }
        } 
    }    
}

/*************************************************************************************************/
/* Statements related functions                                                                 */
/*************************************************************************************************/


static void _weed_stmt_node(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    switch(stmt->kind)
    {
        case k_EXPRSTMT:
            _weed_expr_stmt_node(stmt->exprstmt.expr);
            break;
        case k_BLOCKSTMT:
            _weed_stmt_node(stmt->blockstmt.stmts);
            break;
        case k_ASSIGNSTMT:
            _weed_assign_stmt_node(&(stmt->assignstmt));
            break;
        case k_OPASSIGNSTMT:
            _weed_opassign_stmt_node(&(stmt->opassignstmt));
            break;
        case k_DECLSTMT:
            _weed_decl_node(&(stmt->declstmt));
            break;
        case k_SHORTDECLSTMT:
            _weed_shortdecl_stmt_node(&(stmt->shortdeclstmt));
            break;
        case k_INCREMENTSTMT:
            _weed_expr_node(stmt->incrementstmt.expr);
            _weed_blank_id_check(stmt->incrementstmt.expr);
            break;
        case k_DECREMENTSTMT:
            _weed_expr_node(stmt->decrementstmt.expr);
            _weed_blank_id_check(stmt->incrementstmt.expr);
            break;
        case k_IFSTMT:
            _weed_if_stmt_node(&(stmt->ifstmt));
            break;
        case k_SWITCHSTMT:
            _weed_switch_stmt_node(&(stmt->switchstmt));
            break;
        case k_SWITCHCASESTMT:
            _weed_switchcase_stmt_node(&(stmt->switchcasestmt));
            break;
        case k_FORSTMT:
            _weed_for_stmt_node(&(stmt->forstmt));
            break;
        case k_BREAKSTMT:
            if(!allow_break)
            {
                fprintf(stderr, "Error:[line %d] Unexpected break statement \n", stmt->lineno);
                exit(1);
            }
            break;
        case k_CONTINUESTMT:
            if(!allow_continue)
            {
                fprintf(stderr, "Error:[line %d] Unexpected continue statement \n", stmt->lineno);
                exit(1);
            }
            break;
        case k_SEQSTMT:
            _weed_stmt_node(stmt->seqstmt.stmt);
            _weed_stmt_node(stmt->seqstmt.next);
        case k_RETURNSTMT:
        case k_EMPTYSTMT:     
            return;
        default:
            fprintf(stderr, "Error:[line %d] Unknown statement type \n", stmt->lineno);
            exit(1);           
    }

}

static void _weed_error_func_return(int lineno, char* name, const char* error_type)
{
    fprintf(stderr, "Error:[line %d] Function %s does not have a terminating statement [%s]. \n",
            lineno, name, error_type);
    exit(1);
}

static int _weed_terminating_statement(char* name, stmt_t* stmt)
{
    if(stmt == NULL)
        return 0;
    while(stmt->kind == k_SEQSTMT && stmt->seqstmt.next != NULL)
    {
        stmt = stmt->seqstmt.next;
    }
    if(stmt->kind == k_SEQSTMT)
        stmt = stmt->seqstmt.stmt;
    switch (stmt->kind)
    {
        case k_RETURNSTMT:
            return 1;
        case k_BLOCKSTMT:
            return _weed_terminating_statement(name, stmt->blockstmt.stmts);
        case k_IFSTMT:
            if(stmt->ifstmt.elseblock == NULL){
                _weed_error_func_return(stmt->lineno, name, "missing else branch");
            }
            if((_weed_terminating_statement(name, stmt->ifstmt.thenblock) &&
                _weed_terminating_statement(name, stmt->ifstmt.elseblock)) == 0){
                
                _weed_error_func_return(stmt->lineno, name, "Not returned on all paths");
                
            }
            return 1;
        case k_FORSTMT:
            if(stmt->forstmt.condition != NULL){
                _weed_error_func_return(stmt->lineno, name, "loop condition not empty");
            }
            if(_weed_contains_break(stmt->forstmt.loopblock)){
                _weed_error_func_return(stmt->lineno, name, "loop cannot break");
            }
            return 1;
        case k_SWITCHSTMT:
            if(_weed_contains_break(stmt->switchstmt.cases)){
                _weed_error_func_return(stmt->lineno, name, "switch cannot break");
            }
            stmt_t* case_seq = stmt->switchstmt.cases;
            bool default_found = false;
            while(case_seq!= NULL){
                stmt_t* current_case = case_seq->seqstmt.stmt;
                if(_weed_terminating_statement(name, current_case) == 0){
                    _weed_error_func_return(current_case->lineno, name, "switch case does not terminate");
                }
                if(current_case->switchcasestmt.exprs == NULL)
                    default_found = true;
                case_seq = case_seq->seqstmt.next;
            }
            if(!default_found){
                _weed_error_func_return(stmt->lineno, name, "no default case");
            }
            return 1;
        case k_SWITCHCASESTMT:        
            return _weed_terminating_statement(name, stmt->switchcasestmt.stmts);
        case k_BREAKSTMT:
        case k_ASSIGNSTMT:
        case k_OPASSIGNSTMT:
        case k_DECLSTMT:
        case k_SHORTDECLSTMT:
        case k_INCREMENTSTMT:
        case k_DECREMENTSTMT:
        case k_EXPRSTMT:
        case k_CONTINUESTMT:
        case k_EMPTYSTMT:
            return 0;
        default:
            fprintf(stderr, "Error: [line %d] Unknown statement type. \n", stmt->lineno);
            exit(1);
    }
}

static int _weed_contains_break(stmt_t* stmt)
{
    if(stmt == NULL)
        return 0;
    switch(stmt->kind)
    {
        case k_SEQSTMT:
            return _weed_contains_break(stmt->seqstmt.stmt) ||
                  _weed_contains_break(stmt->seqstmt.next);
        case k_BLOCKSTMT:
            return _weed_contains_break(stmt->blockstmt.stmts);
        case k_BREAKSTMT:
            return 1;
        case k_IFSTMT:
            return _weed_contains_break(stmt->ifstmt.thenblock) ||
                  _weed_contains_break(stmt->ifstmt.elseblock);
        case k_SWITCHSTMT:
        case k_SWITCHCASESTMT:
        case k_RETURNSTMT:
        case k_FORSTMT:
        case k_ASSIGNSTMT:
        case k_OPASSIGNSTMT:
        case k_DECLSTMT:
        case k_SHORTDECLSTMT:
        case k_INCREMENTSTMT:
        case k_DECREMENTSTMT:
        case k_EXPRSTMT:
        case k_CONTINUESTMT:
        case k_EMPTYSTMT:
            return 0;
        default:
            fprintf(stderr,"Error: [line %d] Unknown statements type", stmt->lineno);
            exit(1);
    }

}


static void _weed_expr_stmt_node(expr_t* expr)
{
    if(expr == NULL)
        return;
    
    if(expr->kind != k_FUNCCALLEXPR && expr->kind != k_BUILTINCALLEXPR)
    {
        fprintf(stderr, "Error: [line %d] Expression Statements can only be function calls. \n", 
                        expr->lineno);
        exit(1);
    }

    _weed_blank_id_check(expr->funccallexpr.args);
    _weed_expr_node(expr);

}

static void _weed_assign_stmt_node(struct assignstmt* stmt)
{
    if(stmt == NULL)
        return;
    _weed_valid_lhs_list(stmt->names);
    _weed_blank_id_check(stmt->exprs);
    _weed_check_lr_balance(stmt->names, stmt->exprs);
    _weed_expr_node(stmt->exprs);
}

static void _weed_opassign_stmt_node(struct opassignstmt* stmt)
{
    if(stmt == NULL)
        return;
    _weed_valid_lhs_list(stmt->name);
    _weed_blank_id_check(stmt->name);
    _weed_blank_id_check(stmt->expr);
    _weed_expr_node(stmt->expr);
}

static void _weed_shortdecl_stmt_node(struct shortdeclstmt* stmt)
{
    if(stmt == NULL)
        return;
    _weed_valid_ident_list(stmt->names);
    _weed_blank_id_check(stmt->exprs);
    _weed_check_lr_balance(stmt->names, stmt->exprs);    
}

static void _weed_if_stmt_node(struct ifstmt* stmt)
{
    if(stmt == NULL)
        return;
    

    _weed_stmt_node(stmt->init);
    _weed_simple_stmt_node(stmt->init);
    
    _weed_expr_node(stmt->condition);
    _weed_blank_id_check(stmt->condition);

    _weed_stmt_node(stmt->thenblock);
    _weed_stmt_node(stmt->elseblock);

}

static void _weed_switch_stmt_node(struct switchstmt* stmt)
{
    if(stmt == NULL)
        return;

    int disable_break = !allow_break;

    allow_break = 1;
    stmt_t* current_case = stmt->cases;

    int default_count = 0;
    _weed_stmt_node(stmt->init);
    _weed_simple_stmt_node(stmt->init);

    _weed_blank_id_check(stmt->condition);
    while(current_case != NULL)
    {
        _weed_stmt_node(current_case->seqstmt.stmt);
        stmt_t* case_val = current_case->seqstmt.stmt;
        if(case_val->switchcasestmt.exprs == NULL)
            default_count++;
        
        if(default_count>1)
        {
            fprintf(stderr, "Error: [line %d] Duplicate default case in switch statement. \n"
                    , case_val->lineno);
            exit(1);
        }
        _weed_blank_id_check(case_val->switchcasestmt.exprs);
        current_case = current_case->seqstmt.next;
    }
    
    if(disable_break)
        allow_break = 0;

}

static void _weed_switchcase_stmt_node(struct switchcasestmt* stmt)
{
    if(stmt == NULL)
        return;
    
    _weed_blank_id_check(stmt->exprs);
    _weed_stmt_node(stmt->stmts);
}


static void _weed_for_stmt_node(struct forstmt* stmt)
{
    if(stmt == NULL)
        return;

    int disable_break = !allow_break;
    int disable_continue = !allow_continue;

    allow_break = 1;
    allow_continue = 1;
    _weed_stmt_node(stmt->init);
    _weed_simple_stmt_node(stmt->init);
    _weed_expr_node(stmt->condition);
    _weed_blank_id_check(stmt->condition);
    if(stmt->post != NULL && stmt->post->kind == k_SHORTDECLSTMT)
    {
        fprintf(stderr, "Error: [line %d] post loop statement can't be a short declaration \n",
                stmt->post->lineno);
        exit(1);
    }
    _weed_stmt_node(stmt->post);
    _weed_simple_stmt_node(stmt->post);
    _weed_stmt_node(stmt->loopblock);
    
    if(disable_break)
        allow_break = 0;

    if(disable_continue)
        allow_continue = 0;    
}

static void _weed_simple_stmt_node(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    
    if(stmt->kind == k_EXPRSTMT && stmt->exprstmt.expr->kind != k_FUNCCALLEXPR)
    {
        fprintf(stderr, "Error: [line %d] expression statements can only be function calls. \n",
                stmt->lineno);
        exit(1);
    }
}

/*************************************************************************************************/
/* Expression related functions                                                                 */
/*************************************************************************************************/


static int _weed_expr_list_length(expr_t* expr)
{
    if(expr == NULL)
        return 0;

    switch(expr->kind)
    {
        case k_SEQEXPR:
            return _weed_expr_list_length(expr->seqexpr.main) +
                   _weed_expr_list_length(expr->seqexpr.next);
        case k_INDEXINGEXPR:
            if(expr->indexingexpr.index == NULL)
            {
                fprintf(stderr,"Error:[line %d] the index cannot be empty \n",
                expr->lineno);
                exit(1);
            }
            return 1;
        case k_PARENTHESESEXPR:
        case k_FIELDSELECTOREXPR:
        case k_IDENTEXPR:
        case k_LITERALEXPR:
        case k_UNARYEXPR:
        case k_BINARYEXPR:
        case k_FUNCCALLEXPR:
        case k_BUILTINCALLEXPR:
            return 1;
        default:
            fprintf(stderr,"Error:[line %d] Undefined expression type.",
                    expr->lineno);
            exit(1);
    }
}

static int _weed_valid_ident_list(expr_t* expr)
{
    if(expr == NULL)
        return 1;
    switch(expr->kind)
    {
        case k_IDENTEXPR:
            return 1;
        case k_SEQEXPR:
            return _weed_valid_ident_list(expr->seqexpr.main) &&
                   _weed_valid_ident_list(expr->seqexpr.next);
        case k_LITERALEXPR:
        case k_UNARYEXPR:
        case k_BINARYEXPR:
        case k_FUNCCALLEXPR:
        case k_PARENTHESESEXPR:
        case k_FIELDSELECTOREXPR:
        case k_INDEXINGEXPR:
            fprintf(stderr,"Error:[line %d] Expected a list of identifiers. \n",
                    expr->lineno);
            exit(1);
        default:
            fprintf(stderr,"Error:[line %d] Undefined expression type. \n", 
                    expr->lineno);
            exit(1);
    }
}



static int _weed_valid_lhs_list(expr_t* expr)
{
    if(expr == NULL)
        return 0;
    
    switch(expr->kind)
    {
        case k_IDENTEXPR:
        case k_PARENTHESESEXPR:
            return 1;
        case k_FIELDSELECTOREXPR:
            _weed_blank_id_check(expr->fieldselectorexpr.selector);
            _weed_blank_id_check(expr->fieldselectorexpr.receiver);
            return 0;
        case k_INDEXINGEXPR:
            if(expr->indexingexpr.index == NULL)
            {
                fprintf(stderr, "Error:[line %d] array index cannot be empty. \n",
                    expr->lineno);
                exit(1);
            }
            _weed_blank_id_check(expr->indexingexpr.index);
            _weed_blank_id_check(expr->indexingexpr.array);
            return 1;
        case k_SEQEXPR:
            return _weed_valid_lhs_list(expr->seqexpr.main) &&
                   _weed_valid_lhs_list(expr->seqexpr.next);
        case k_LITERALEXPR:
        case k_UNARYEXPR:
        case k_BINARYEXPR:
            fprintf(stderr, "Error: [line %d] Invalid arguments on the lhs. \n",
                    expr->lineno);
            exit(1);
        case k_FUNCCALLEXPR:
        case k_BUILTINCALLEXPR:
            return 1;
        default:
            fprintf(stderr, "Error:[line %d] Undefined expression type. \n", 
                    expr->lineno);
            exit(1);
    }
}


static void _weed_check_lr_balance(expr_t* lhs, expr_t* rhs)
{
    int lhs_length = _weed_expr_list_length(lhs);
    int rhs_length = _weed_expr_list_length(rhs);

    if(rhs_length != lhs_length)
    {
        fprintf(stderr,"Error: [line %d] mismatch assignment arguments lhs(%d)!= rhs(%d) \n",
                lhs->lineno, lhs_length, rhs_length);
        exit(1);
    }
}


static void _weed_blank_id_check(expr_t* expr)
{
    if(expr == NULL)
        return;

    switch(expr->kind)
    {
        case k_IDENTEXPR:
            if(BLANK_ID_CHECK(expr->identexpr.value))
            {
                fprintf(stderr,"Error: [line %d] Unexpected blank identifiers.\n", expr->lineno);
                exit(1);
            }
        case k_LITERALEXPR:
            break;
        case k_UNARYEXPR:
            _weed_blank_id_check(expr->unaryexpr.expr);
            break;
        case k_BINARYEXPR:
            _weed_blank_id_check(expr->binaryexpr.l_expr);
            _weed_blank_id_check(expr->binaryexpr.r_expr);
            break;
        case k_FUNCCALLEXPR:
        case k_BUILTINCALLEXPR:
            _weed_blank_id_check(expr->funccallexpr.args);
            break;
        case k_INDEXINGEXPR:
            _weed_blank_id_check(expr->indexingexpr.array);
            _weed_blank_id_check(expr->indexingexpr.index);
            break;
        case k_FIELDSELECTOREXPR:
            _weed_blank_id_check(expr->fieldselectorexpr.selector);
            _weed_blank_id_check(expr->fieldselectorexpr.receiver);
            break;
        case k_SEQEXPR:
            _weed_blank_id_check(expr->seqexpr.main);
            _weed_blank_id_check(expr->seqexpr.next);
            break;
        case k_PARENTHESESEXPR:
            _weed_blank_id_check(expr->parenthesesexpr.main);
            break;
        default:
            fprintf(stderr, "Error:[line %d] Unknown statement type \n",expr->lineno);
            exit(1);  
    }
  
}

/*************************************************************************************************/
/* Type related functions                                                                 */
/*************************************************************************************************/

static void _weed_type_node(type_t* type)
{
    if(type == NULL)
        return;
    switch (type->kind)
    {
        case k_SIMPLETYPE:
            if (BLANK_ID_CHECK(type->name))
            {
                fprintf(stderr, "Error:[line %d] type cannot contain the blank identifier.\n", type->lineno);
                exit(1);
            }
            break;
        case k_STRUCTTYPE:
            _weed_check_decl_struct(type->structdef);
            break;
        default: ;
    }
}
