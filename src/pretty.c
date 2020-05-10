#include <stdio.h>
#include <stdlib.h>
#include "pretty.h"

#define INDENT(count) for(int i=0; i<count ; i++) {printf("    ");}


static void _pretty_expr_node(expr_t* expr);
static void _pretty_bin_expr_node(enum binaryexprkind kind, expr_t* l_expr, expr_t* r_expr);
static void _pretty_unary_expr_node(enum unaryexprkind kind, expr_t* expr);
static void _pretty_literal_expr_node(struct literalexpr expr);
static void _pretty_func_expr_node(expr_t* name, expr_t* args);
static void _pretty_indexing_expr_node(expr_t* array, expr_t* index);
static void _pretty_select_expr_node(expr_t* receiver, expr_t* selector);

static void _pretty_decl_node(decl_t* decl, int indentation);
static void _pretty_var_decl_node(struct vardecl* decl, int indentation);
static void _pretty_type_decl_node(struct typedecl* decl, int indentation);
static void _pretty_func_decl_node(struct funcdecl* decl, int indentation);

static void _pretty_stmt_node(stmt_t* stmt, int indentation);
static void _pretty_assign_stmt_node(struct assignstmt* stmt, int indentation);
static void _pretty_shortdecl_stmt_node(struct shortdeclstmt* decl, int indentation);
static void _pretty_return_stmt_node(struct returnstmt* stmt, int indentation);
static void _pretty_if_stmt_node(struct ifstmt* stmt, int indentation);
static void _pretty_switch_stmt_node(struct switchstmt* stmt, int indentation);
static void _pretty_switchcase_stmt_node(struct switchcasestmt* stmt, int indentation);
static void _pretty_for_stmt_node(struct forstmt* stmt, int indentation);
static void _pretty_block_stmt_node(struct blockstmt* stmt, int indentation);
static void _pretty_opassign_stmt_node(struct opassignstmt* stmt, int indentation);

static void _pretty_type(type_t* typenode, int indentation);

int simplestmt_newline = 1;

void pretty(decl_t* root)
{
    _pretty_decl_node(root,0);
    printf("\n");
}


/*************************************************************************************************/
/* Expression related functions                                                                  */
/*************************************************************************************************/

static void _pretty_expr_node(expr_t* expr)
{
    if(expr == NULL)
        return;

    switch(expr->kind)
    {
        case k_IDENTEXPR:
            printf("%s", expr->identexpr.value);
            break;
        case k_LITERALEXPR:
            _pretty_literal_expr_node(expr->literalexpr);
            break;
        case k_UNARYEXPR:
            _pretty_unary_expr_node(expr->unaryexpr.kind, expr->unaryexpr.expr);
            break;
        case k_BINARYEXPR:
            _pretty_bin_expr_node(expr->binaryexpr.kind, expr->binaryexpr.l_expr,expr->binaryexpr.r_expr);
            break;
        case k_FUNCCALLEXPR:
        case k_BUILTINCALLEXPR:
            _pretty_func_expr_node(expr->funccallexpr.name, expr->funccallexpr.args);
            break;
        case k_INDEXINGEXPR:
            _pretty_indexing_expr_node(expr->indexingexpr.array, expr->indexingexpr.index);
            break;
        case k_FIELDSELECTOREXPR:
            _pretty_select_expr_node(expr->fieldselectorexpr.receiver, expr->fieldselectorexpr.selector);
            break;
        case k_SEQEXPR:
            _pretty_expr_node(expr->seqexpr.main);
            if(expr->seqexpr.next != NULL)
            {
                printf(", ");
                _pretty_expr_node(expr->seqexpr.next);
            }
            break;
        case k_PARENTHESESEXPR:
            _pretty_expr_node(expr->parenthesesexpr.main);
            break;
        default:
            printf("Undefined expression type received \n");
            exit(1);
    }

}


static void _pretty_bin_expr_node(enum binaryexprkind kind, expr_t* l_expr, expr_t* r_expr)
{
    printf("(");
    _pretty_expr_node(l_expr);
    switch(kind)
    {
        case k_ADDBINARYEXPR:
            printf(" + ");
            break;
        case k_SUBBINARYEXPR:
            printf(" - ");
            break;
        case k_MULBINARYEXPR:
            printf(" * ");
            break;
        case k_DIVBINARYEXPR:
            printf(" / ");
            break;
        case k_MODBINARYEXPR:
            printf(" %% ");
            break;
        case k_LSHIFTBINARYEXPR:
            printf(" << ");
            break;
        case k_RSHIFTBINARYEXPR:
            printf(" >> ");
            break;
        case k_BITWISEANDBINARYEXPR:
            printf(" & ");
            break;
        case k_BITWISEORBINARYEXPR:
            printf(" | ");
            break;
        case k_BITWISEXORBINARYEXPR:
            printf(" ^ ");
            break;
        case k_BITWISECLRBINARYEXPR:
            printf(" &^ ");
            break;
        case k_EQBINARYEXPR:
            printf(" == ");
            break;
        case k_NEBINARYEXPR:
            printf(" != ");
            break;
        case k_LTBINARYEXPR:
            printf(" < ");
            break;
        case k_LEBINARYEXPR:
            printf(" <= ");
            break;
        case k_GTBINARYEXPR:
            printf(" > ");
            break;
        case k_GEBINARYEXPR:
            printf(" >= ");
            break;
        case k_LOGICALANDBINARYEXPR:
            printf(" && ");
            break;
        case k_LOGICALORBINARYEXPR:    
            printf(" || ");
            break;
        default:
            fprintf(stderr, "Undefined binary expression kind \n");
            exit(1);

    }
    _pretty_expr_node(r_expr);
    printf(")");
}

static void _pretty_unary_expr_node(enum unaryexprkind kind, expr_t* expr)
{
    if(expr == NULL)
        return;
    printf("(");
    switch(kind)
    {
        case k_POSUNARYEXPR:
            printf("+(");
            break;
        case k_NEGUNARYEXPR:
            printf("-(");
            break;
        case k_NOTUNARYEXPR:
            printf("!(");
            break;
        case k_BITNOTUNARYEXPR:
            printf("^(");
            break;
        
        default:
            fprintf(stderr, "Undefined unary expression kind \n");
            exit(1);
    }

    _pretty_expr_node(expr);
    printf(")");
}


static void _pretty_literal_expr_node(struct literalexpr expr)
{

    switch(expr.kind)
    {
        case k_INTLITERALEXPR:
            printf("%d", expr.intval);
            break;
        case k_FLTLITERALEXPR:
            printf("%lf", expr.fltval);
            break;
        case k_STRLITERALEXPR:
            printf("\"%s\"", expr.strval);
            break;
        case k_CHRLITERALEXPR:
            printf("'%s'", expr.chrval);
            break;       
        default:
            fprintf(stderr, "Undefined literal expression kind \n");
            exit(1);        

    }
}

static void _pretty_func_expr_node(expr_t* name, expr_t* args)
{
    _pretty_expr_node(name);
    printf("(");
    _pretty_expr_node(args);
    printf(")");
}

static void _pretty_indexing_expr_node(expr_t* array, expr_t* index)
{
    if(array == NULL || index ==NULL)
        return;

    _pretty_expr_node(array);
    printf("[");
    _pretty_expr_node(index);
    printf("]");

}

static void _pretty_select_expr_node(expr_t* receiver, expr_t* selector)
{
    if(receiver == NULL || selector ==NULL)
        return;

    _pretty_expr_node(receiver);
    printf(".");
    _pretty_expr_node(selector);

}

/*************************************************************************************************/
/* Declaration related functions                                                                 */
/*************************************************************************************************/



static void _pretty_decl_node(decl_t* decl, int indentation)
{
    if (decl == NULL)
        return;
    switch(decl->kind)
    {
        case k_PKGDECL:
            printf("package %s; \n\n", decl->pkgdecl.name);
            break;
        case k_VARDECL:
            _pretty_var_decl_node(&(decl->vardecl), indentation);
            break;
        case k_TYPEDECL:
            _pretty_type_decl_node(&(decl->typedecl), indentation);
            break;
        case k_FUNCDECL:
            _pretty_func_decl_node(&(decl->funcdecl), indentation);
            break;
        case k_SEQDECL:
            _pretty_decl_node(decl->seqdecl.decl, indentation);
            _pretty_decl_node(decl->seqdecl.next, indentation);
            break;
        default:
            fprintf(stderr, "Error: Unknown declaration type \n");
            exit(1);

    }

}

static void _pretty_var_decl_node(struct vardecl* decl, int indentation)
{
    if(decl == NULL)
        return;

    INDENT(indentation);
    if (decl->kind == k_IDENTVARDECL)
    {
        printf("var ");
    }
    _pretty_expr_node(decl->names);
    printf(" ");
    _pretty_type(decl->type, indentation);
    
    if(decl->kind == k_IDENTVARDECL && decl->exprs != NULL)
    {
        printf(" = ");
        _pretty_expr_node(decl->exprs);
    }

    if (decl->kind == k_IDENTVARDECL)
    {
        printf("; \n");
    }

    if (decl->kind == k_STRUCTVARDECL)
    {
        printf("\n");
    }
}


static void _pretty_type_decl_node(struct typedecl* decl, int indentation)
{
    if(decl == NULL)
        return;
    INDENT(indentation);
    printf("type %s ", decl->name);
    _pretty_type(decl->type, indentation);
    printf("; \n");
}

static void _pretty_func_decl_node(struct funcdecl* decl, int indentation)
{
    INDENT(indentation);

    printf("func %s (", decl->name);

    decl_t* func_args = decl->args;

    while(func_args != NULL)
    {
        _pretty_decl_node(func_args->seqdecl.decl, 0);
        if(func_args->seqdecl.next != NULL)
        {
            printf(", ");
        }
        func_args = func_args->seqdecl.next;
    }
    if(decl->returntype == NULL)
    {
        printf(") {\n");
    }
    else 
    {
        printf(" )");
        _pretty_type(decl->returntype, indentation);
        printf(" { \n");
    }
    
    _pretty_stmt_node(decl->stmts, indentation+1);

    INDENT(indentation);
    printf("} \n\n");
    
}

/*************************************************************************************************/
/* Statement related functions                                                                   */
/*************************************************************************************************/

static void _pretty_stmt_node(stmt_t* stmt, int indentation)
{
    if(stmt == NULL)
        return;

    switch(stmt->kind)
    {
        case k_EXPRSTMT:
            INDENT(indentation);
            _pretty_expr_node(stmt->exprstmt.expr);
            printf("; \n");
            break;
        case k_BLOCKSTMT:
            _pretty_block_stmt_node(&(stmt->blockstmt),indentation);
            break;
        case k_ASSIGNSTMT:
            _pretty_assign_stmt_node(&(stmt->assignstmt), indentation);
            break;
        case k_OPASSIGNSTMT:
            _pretty_opassign_stmt_node(&(stmt->opassignstmt), indentation);
            break;
        case k_DECLSTMT:
            _pretty_decl_node(&(stmt->declstmt), indentation);
            break;
        case k_SHORTDECLSTMT:
            _pretty_shortdecl_stmt_node(&(stmt->shortdeclstmt), indentation);
            break;
        case k_INCREMENTSTMT:
            INDENT(indentation);
            _pretty_expr_node(stmt->incrementstmt.expr);
            if(simplestmt_newline)
            {
                printf("++;\n");
            }
            else
            {
                printf("++");
            }
            break;
        case k_DECREMENTSTMT:
            INDENT(indentation);
            _pretty_expr_node(stmt->decrementstmt.expr);
            if(simplestmt_newline)
            {
                printf("--;\n");
            }
            else
            {
                printf("--");
            }
            break;
        case k_RETURNSTMT: 
            _pretty_return_stmt_node(&(stmt->returnstmt), indentation);
            break;
        case k_IFSTMT:
            _pretty_if_stmt_node(&(stmt->ifstmt), indentation);
            break;
        case k_SWITCHSTMT:
            _pretty_switch_stmt_node(&(stmt->switchstmt), indentation);
            break;
        case k_SWITCHCASESTMT:
            _pretty_switchcase_stmt_node(&(stmt->switchcasestmt), indentation);
            break;
        case k_FORSTMT:
            _pretty_for_stmt_node(&(stmt->forstmt), indentation);
            break;
        case k_BREAKSTMT:
            INDENT(indentation);
            printf("break; \n");
            break;
        case k_CONTINUESTMT:
            INDENT(indentation);
            printf("continue; \n");
            break;      
        case k_EMPTYSTMT:
            INDENT(indentation);
            printf("; \n");
            break;  
        case k_SEQSTMT:
            _pretty_stmt_node(stmt->seqstmt.stmt, indentation);
            _pretty_stmt_node(stmt->seqstmt.next, indentation);
            break;
        default:
            fprintf(stderr, "Error: Unknown statement type \n");
            exit(1);
    }
    
}

static void _pretty_assign_stmt_node(struct assignstmt* stmt, int indentation)
{
    if(stmt == NULL)
        return;
    INDENT(indentation);
    _pretty_expr_node(stmt->names);
    printf(" = ");
    _pretty_expr_node(stmt->exprs);
    if(simplestmt_newline)
        printf("; \n");    
}


static void _pretty_shortdecl_stmt_node(struct shortdeclstmt* decl, int indentation)
{
    if(decl == NULL)
        return;
    INDENT(indentation);
    _pretty_expr_node(decl->names);
    printf(" := ");
    _pretty_expr_node(decl->exprs);
    if(simplestmt_newline)
        printf("; \n");
    
}

static void _pretty_return_stmt_node(struct returnstmt* stmt, int indentation)
{
    if(stmt == NULL)
        return;
    INDENT(indentation);
    printf("return ");
    _pretty_expr_node(stmt->expr);
    printf("; \n");

}

static void _pretty_if_stmt_node(struct ifstmt* stmt, int indentation)
{
    if(stmt == NULL)
        return;

    INDENT(indentation);

    printf("if ");

    if(stmt->init != NULL)
    {
        simplestmt_newline = 0;
        _pretty_stmt_node(stmt->init, 0);
        simplestmt_newline = 1;
        printf(" ; ");
    }

    _pretty_expr_node(stmt->condition);

    printf(" {\n");

    _pretty_stmt_node(stmt->thenblock, indentation+1);

    if(stmt->elseblock != NULL)
    {   
        INDENT(indentation);
        printf("} else { \n");
        _pretty_stmt_node(stmt->elseblock, indentation + 1);
        
    } 
    
    INDENT(indentation);
    printf("}\n");
    
}

static void _pretty_switch_stmt_node(struct switchstmt* stmt, int indentation)
{
    if(stmt == NULL)
        return;

    INDENT(indentation);
    printf("switch ");
    if(stmt->init != NULL)
    {
        simplestmt_newline = 0;
        _pretty_stmt_node(stmt->init, 0);
        simplestmt_newline = 1;
        printf(" ; ");
    }
    _pretty_expr_node(stmt->condition);
    printf(" {\n");
    _pretty_stmt_node(stmt->cases, indentation + 1);
    INDENT(indentation);
    printf("}\n");
}

static void _pretty_switchcase_stmt_node(struct switchcasestmt* stmt, int indentation)
{
    if(stmt == NULL)
        return;

    INDENT(indentation);
    if(stmt->exprs != NULL)
    {
        printf("case ");
        _pretty_expr_node(stmt->exprs);
    }
    else 
    {
        printf("default ");
    }

    printf(" :\n");
    _pretty_stmt_node(stmt->stmts, indentation + 1);
}


static void _pretty_for_stmt_node(struct forstmt* stmt, int indentation)
{
    if(stmt == NULL)
        return;  

    INDENT(indentation);
    printf("for ");

    if(stmt->init == NULL && stmt->condition == NULL && stmt->post == NULL )
    {
        printf(" {\n");
    }
    else if (stmt->init == NULL && stmt->condition != NULL && stmt->post == NULL)
    {
        _pretty_expr_node(stmt->condition);
        printf(" {\n");

    }
    else 
    {
        simplestmt_newline = 0;
        _pretty_stmt_node(stmt->init, 0);
        printf(" ; ");
        _pretty_expr_node(stmt->condition);
        printf(" ; ");
        _pretty_stmt_node(stmt->post, 0);
        simplestmt_newline = 1;
        printf(" {\n");
    }


    _pretty_stmt_node(stmt->loopblock, indentation + 1);
    
    INDENT(indentation);
    printf("}\n");

}

static void _pretty_block_stmt_node(struct blockstmt* stmt, int indentation)
{
    if(stmt == NULL)
        return;
    INDENT(indentation);
    printf("{\n");
    _pretty_stmt_node(stmt->stmts, indentation + 1);
    INDENT(indentation);
    printf("}\n");

}


static void _pretty_opassign_stmt_node(struct opassignstmt* stmt, int indentation)
{
    if(stmt == NULL)
        return;
    
    INDENT(indentation);
    _pretty_expr_node(stmt->name);

    switch(stmt->kind)
    {
        case k_ADDASSIGN:
            printf(" += ");
            break;
        case k_SUBASSIGN:
            printf(" -= ");
            break;
        case k_MULASSIGN:
            printf(" *= ");
            break;
        case k_DIVASSIGN:
            printf(" /= ");
            break;
        case k_MODASSIGN:
            printf(" %%= ");
            break;
        case k_BITWISEANDASSIGN:
            printf(" &= ");
            break;
        case k_BITWISEORASSIGN:
            printf(" |= ");
            break;
        case k_NOTASSIGN:
            printf(" ^= ");
            break;
        case k_LSHIFTASSIGN:
            printf(" <<= ");
            break;
        case k_RSHIFTASSIGN:
            printf(" >>= ");
            break;
        case k_BITWISECLRASSIGN:
            printf(" &^= ");
            break;
        default:
            fprintf(stderr, "Error: Undefined operator assignment kind. \n");
            break;
    }

    _pretty_expr_node(stmt->expr);
    if(simplestmt_newline)
        printf(";\n");
}

/*************************************************************************************************/
/* Statement related functions                                                                   */
/*************************************************************************************************/

static void _pretty_type(type_t* typenode, int indentation)
{

    if (typenode != NULL) {
        if (typenode->array_type)
        {
            arrayindex_t* dimension = typenode->dimensions;
            while (dimension != NULL)
            {

                if (dimension->index != -1) printf("[%d]", dimension->index);
                else printf("[]"); 
                dimension = dimension->next;
            }
        }

        switch (typenode->kind)
        {
            case k_SIMPLETYPE:
                printf("%s", typenode->name);
                break;
            case k_STRUCTTYPE:
                printf("struct {\n");
                _pretty_decl_node(typenode->structdef, indentation + 1);
                INDENT(indentation);
                printf("}");
                break;
            default: ;
        }
    }
}
