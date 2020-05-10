#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "symbol.h"
#include "tree.h"
#include "python_templates.h"

#define INDENT(count)  for(int i=0; i<count ; i++) {printf("    ");}

/*Func declaration */
static void _codegen_mainfunc();

static void _codegen_decl(decl_t* decl);
static void _codegen_vardecl(decl_t* decl);
static void _codegen_typedecl(decl_t* decl);
static void _codegen_funcdecl(decl_t* decl);
static void _codegen_print_globalvar(namelist_t* namelist);
static void _codegen_initialize_vardecl(expr_t* names);
static void _codegen_default_initialize(symboltype_t* symboltype);
static void _codegen_default_initialize_array(symboltype_t* symboltype, arrayindex_t* dimensions);
static void _codegen_default_initialize_struct(symboltype_t* symboltype);

static void _codegen_expr(expr_t* expr);
static void _codegen_identexpr(expr_t* expr);
static void _codegen_literalexpr(expr_t* expr);
static void _codegen_unaryexpr(expr_t* expr);
static void _codegen_binaryexpr(expr_t* expr);
static void _codegen_funccallexpr(expr_t* expr);
static void _codegen_funccallexpr_funccall(expr_t* expr);
static void _codegen_funccallexpr_typecast(expr_t* expr);
static void _codegen_builtincallexpr(expr_t* expr);
static void _codegen_builtincallexpr_print(expr_t* expr, int newline);
static void _codegen_builtincallexpr_cap(expr_t* expr);
static void _codegen_builtincallexpr_len(expr_t* expr);
static void _codegen_builtincallexpr_append(expr_t* expr);
static void _codegen_indexingexpr(expr_t* expr);
static void _codegen_fieldselectorexpr(expr_t* expr);
static void _codegen_seqexpr(expr_t* expr);
static void _print_assignment_rhs(expr_t* expr);
static int _codegen_is_slice_identifier(expr_t* expr);
static int _codegen_is_array_identifier(expr_t* expr);
static int _codegen_is_struct(expr_t* expr);

static void _codegen_stmt(stmt_t* stmt);
static void _codegen_exprstmt(stmt_t* stmt);
static void _codegen_blockstmt(stmt_t* stmt);
static void _codegen_assignstmt(stmt_t* stmt);
static void _codegen_opassignstmt(stmt_t* stmt);
static void _codegen_declstmt(stmt_t* stmt);
static void _codegen_shortdeclstmt(stmt_t* stmt);
static void _codegen_incrementstmt(stmt_t* stmt);
static void _codegen_decrementstmt(stmt_t* stmt);
static void _codegen_returnstmt(stmt_t* stmt);
static void _codegen_ifstmt(stmt_t* stmt);
static void _codegen_switchstmt(stmt_t* stmt);
static void _codegen_switchcasestmt(stmt_t* stmt, int condition_conatiner, int firstcase);
static void _codegen_forstmt(stmt_t* stmt);

static void _codegen_helpers();

/*Global variables*/
int codegen_indent = 0;
extern int init_count;
extern int contain_main;

stmt_t* loop_post = NULL;
int switch_condition_container_count = 0;
char* switch_condition_container_name = "__switch_condition_conatiner_";


/*************************************************************************************************/
/* Helper functions                                                                            */
/*************************************************************************************************/

static const symboltype_t* _codegen_resolve_basetype(const symboltype_t* type)
{
    while (type->type == k_TYPENOTBASE)
    {
        type = &type->parent->type;
    }
    return type;
}

static const symboltype_t* _codegen_resolve_type(const symboltype_t* type)
{
    while (type->type == k_TYPENOTBASE && !type->isarray)
    {
        type = &type->parent->type;
    }
    return type;
}

static int _codegen_is_slice(const symboltype_t* type)
{
    if (type->isarray)
    {
        for (const arrayindex_t* ai = type->dimensions; ai != NULL; ai = ai->next)
        {
            if (ai->index < 0)
            {
                return 1;
            }
        }
    }

    return 0;
}


/*************************************************************************************************/
/* Codegen function                                                                            */
/*************************************************************************************************/

void codegen(decl_t* root)
{
    _codegen_helpers();
    _codegen_decl(root);
    printf("\n\n");
    // Just to fix compiler errors, to be removed
    _codegen_mainfunc();
}

static void _codegen_mainfunc(){
    if(init_count == 0 && contain_main == 0)
        return;
    printf("if __name__ == \"__main__\":\n");
    for(int i = 0; i<init_count; i++){
        printf("    __init_%d()\n", i);
    }
    if(contain_main){
        printf("    __scope1_main()\n");
    }
}

/*************************************************************************************************/
/* Codegen for declarations                                                                      */
/*************************************************************************************************/

static void _codegen_helpers()
{
    printf("%s", imports);
    printf("%s", slice_class);
    printf("%s", fill_list_function);
    printf("%s", _get_bool_string_function);
}

static void _codegen_decl(decl_t* decl)
{
    if(decl == NULL)
        return;

    switch(decl->kind)
    {
        case k_PKGDECL:
            break;
        case k_VARDECL:
            _codegen_vardecl(decl);
            break;
        case k_TYPEDECL:
            _codegen_typedecl(decl);
            break;
        case k_FUNCDECL:
            _codegen_funcdecl(decl);
            break;
        case k_SEQDECL:
            _codegen_decl(decl->seqdecl.decl);
            _codegen_decl(decl->seqdecl.next);
            break;
        default:
            fprintf(stderr, "Undefined declaration kind. \n");
            exit(1);
    }
}

static void _codegen_vardecl(decl_t* decl)
{
    if(decl == NULL)
        return;

    switch(decl->vardecl.kind)
    {
        case k_FUNCVARDECL:
            _codegen_expr(decl->vardecl.names);
            printf(", ");
            break;
        case k_IDENTVARDECL:
            INDENT(codegen_indent);
            _codegen_expr(decl->vardecl.names);
            printf(" = ");
            if (decl->vardecl.exprs != NULL)
            {
                _print_assignment_rhs(decl->vardecl.exprs);
            }
            else
            {
                _codegen_initialize_vardecl(decl->vardecl.names);
            }
            printf("\n");

            break;
        case k_STRUCTVARDECL:
        default: ;
    }
}

static void _codegen_initialize_vardecl(expr_t* names)
{
    while(names != NULL)
    {
        symbol_t* var_symbol = names->seqexpr.main->identexpr.symbol;
        names = names->seqexpr.next;

        _codegen_default_initialize(&(var_symbol->type));
        if (names != NULL)
            printf(", ");
    }
}

static void _codegen_default_initialize(symboltype_t* symboltype)
{
    switch(symboltype->type)
    {
        case k_TYPEINT:
            printf("0");
            break;
        case k_TYPEFLOAT64:
            printf("0.0");
            break;
        case k_TYPERUNE:
            printf("0");
            break;
        case k_TYPEBOOL:
            printf("False");
            break;
        case k_TYPECSTRING:
            printf("''");
            break;
        case k_TYPESTRUCT:
            _codegen_default_initialize_struct(symboltype);

            break;
        case k_TYPENOTBASE:
            if (symboltype->isarray)
            {
                _codegen_default_initialize_array(symboltype, symboltype->dimensions);
            }
            else 
            {
                _codegen_default_initialize(&(symboltype->parent->type));
            }

            break;
        default: ;
    }
}

static void _codegen_default_initialize_array(symboltype_t* symboltype, arrayindex_t* dimensions)
{
    if (dimensions->index == -1)
    {
        if(symboltype->requiredslice){
            printf("%s()", slice_class_name);
        } else {
            printf("[]");
        }
        return;
    }

    printf("%s(", fill_list_function_name);

    if (dimensions->next == NULL)
    {
        _codegen_default_initialize(&(symboltype->parent->type));
    }
    else
    {
        _codegen_default_initialize_array(symboltype, dimensions->next);
    }

    printf(", %d)", dimensions->index);
}

static void _codegen_default_initialize_struct(symboltype_t* symboltype)
{
    symbol_t* structatr = symboltype->structsymbols;

    printf("{ ");
    while (structatr != NULL)
    {
        printf("'%s': ", structatr->name);
        _codegen_default_initialize(&(structatr->type));
        printf(", ");
        structatr = structatr->next;
    }
    printf("}");
}

static void _codegen_typedecl(decl_t* decl)
{
    if(decl == NULL)
        return;
}

static void _codegen_funcdecl(decl_t* decl)
{
    if(decl == NULL)
        return;

    printf("\ndef %s(", decl->funcdecl.symbol->codegen_name);
    _codegen_decl(decl->funcdecl.args);
    printf("):\n");
    codegen_indent++;
    if (decl->funcdecl.stmts)
    {
        _codegen_print_globalvar(decl->funcdecl.symbol->funcglobalvar);
        _codegen_stmt(decl->funcdecl.stmts);
    }
    else
    {
        INDENT(codegen_indent);
        printf("pass");
    }
    codegen_indent--;
    printf("\n");
}

static void _codegen_print_globalvar(namelist_t* namelist)
{
    while(namelist != NULL)
    {
        INDENT(codegen_indent);
        printf("global %s\n", namelist->codegen_name);
        namelist = namelist->next;
    }
}

/*************************************************************************************************/
/* Codegen for expressions                                                                            */
/*************************************************************************************************/

static void _codegen_expr(expr_t* expr)
{
    if(expr == NULL)
        return;
    switch(expr->kind)
    {
        case k_IDENTEXPR:
            _codegen_identexpr(expr);
            break;
        case k_LITERALEXPR:
            _codegen_literalexpr(expr);
            break;
        case k_UNARYEXPR:
            _codegen_unaryexpr(expr);
            break;
        case k_BINARYEXPR:
            _codegen_binaryexpr(expr);
            break;
        case k_FUNCCALLEXPR:
            _codegen_funccallexpr(expr);
            break;
        case k_BUILTINCALLEXPR:
            _codegen_builtincallexpr(expr);
            break;
        case k_INDEXINGEXPR:
            _codegen_indexingexpr(expr);
            break;
        case k_FIELDSELECTOREXPR:
            _codegen_fieldselectorexpr(expr);
            break;
        case k_SEQEXPR:
            _codegen_seqexpr(expr);
            break;
        case k_PARENTHESESEXPR:
            _codegen_expr(expr->parenthesesexpr.main);
            break;    
        default:    
            fprintf(stderr, "Undefined expression kind. \n");
            exit(1);
    }
}

static void _codegen_identexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    printf("%s", expr->identexpr.symbol->codegen_name);
}

static void _codegen_literalexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
        
    switch(expr->literalexpr.kind)
    {
        case k_INTLITERALEXPR:
            printf("%d", expr->literalexpr.intval);
            break;
        case k_FLTLITERALEXPR:
            printf("%lf", expr->literalexpr.fltval);
            break;
        case k_STRLITERALEXPR:
            printf("'%s'", expr->literalexpr.strval);
            break;
        case k_CHRLITERALEXPR:
            printf("ord('%s')", expr->literalexpr.chrval);
            break;
        default:
            fprintf(stderr, "Undefined literal kind.");
            exit(1);    
    }
}

static void _codegen_unaryexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    switch(expr->unaryexpr.kind)
    {
        case k_POSUNARYEXPR:
            // dont really need to print anything
            break;
        case k_NEGUNARYEXPR:
            printf("-");
            break;
        case k_NOTUNARYEXPR:
            printf("not");
            break;
        case k_BITNOTUNARYEXPR:
            printf("~");
            break;
        default:
            fprintf(stderr, "Undefined unary expression kind.");
            exit(1);  
    }
    printf("(");
    _codegen_expr(expr->unaryexpr.expr);
    printf(")");
}

static void _codegen_binaryexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    // TODO cast to int when dividing
    printf("(");
    _codegen_expr(expr->binaryexpr.l_expr);
    switch(expr->binaryexpr.kind)
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
            if (_codegen_resolve_basetype(expr->resolved_type)->type == k_TYPEFLOAT64)
            {
                printf(" / ");
            }
            else
            {
                printf(" // ");
            }
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
            printf(" & ~"); 
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
            printf(" and ");
            break;
        case k_LOGICALORBINARYEXPR:
            printf(" or ");
            break;
        default:
            fprintf(stderr, "Undefined binary expression kind.");
            exit(1);
    }
    _codegen_expr(expr->binaryexpr.r_expr);
    printf(")");
}

static void _codegen_funccallexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    switch (expr->funccallexpr.symbol->kind)
    {
        case k_FUNCTION:
            return _codegen_funccallexpr_funccall(expr);
        case k_TYPE:
            return _codegen_funccallexpr_typecast(expr);
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

static void _codegen_funccallexpr_funccall(expr_t* expr)
{
    if(expr == NULL)
        return;
    _codegen_expr(expr->funccallexpr.name);
    printf("(");
    if (expr->funccallexpr.args)
    {
        for (const expr_t* trav = expr->funccallexpr.args; trav != NULL; trav = trav->seqexpr.next)
        {
            const symboltype_t* type = _codegen_resolve_type(trav->seqexpr.main->resolved_type);
            if (type->type == k_TYPESTRUCT)
            {
                printf("dict(");
                _codegen_expr(trav->seqexpr.main);
                printf(")");
            } else if (type->isarray && _codegen_is_slice(type) && type->requiredslice)
            {
                printf("%s.dup(", slice_class_name);
                _codegen_expr(trav->seqexpr.main);
                printf(")");
            }
            else if(type->isarray && _codegen_is_slice(type)) // we copy the reference list
            {
                _codegen_expr(trav->seqexpr.main);
            }
            else if (type->isarray)
            {
                printf("list(");
                _codegen_expr(trav->seqexpr.main);
                printf(")");
            }
            else
            {
                _codegen_expr(trav->seqexpr.main);
            }
            printf(", ");
        }
    }
    printf(")");
}

static void _codegen_funccallexpr_typecast(expr_t* expr)
{
    symboltypekind_t target = _codegen_resolve_basetype(&expr->funccallexpr.symbol->type)->type;
    switch(target)
    {
        case k_TYPEINT:
            printf("int(");
            _codegen_expr(expr->funccallexpr.args);
            printf(")");
            break;
        case k_TYPEFLOAT64:
            printf("float(");
            _codegen_expr(expr->funccallexpr.args);
            printf(")");
            break;
        case k_TYPERUNE:
            printf("int(");
            _codegen_expr(expr->funccallexpr.args);
            printf(")");
            break;
        case k_TYPECSTRING:
            printf("str(");
            target = _codegen_resolve_basetype(expr->funccallexpr.args->seqexpr.main->resolved_type)->type;
            if (target == k_TYPEINT || target == k_TYPERUNE)
            {
                printf("chr(");
            }
            _codegen_expr(expr->funccallexpr.args);
            if (target == k_TYPEINT || target == k_TYPERUNE)
            {
                printf(")");
            }
            printf(")");
            break;
        case k_TYPEBOOL:
            printf("bool(");
            _codegen_expr(expr->funccallexpr.args);
            printf(")");
            break;
        default:
            printf("Error: [line %d] Unhandled target type while casting.", expr->lineno);
            exit(1);
    }
}

static void _codegen_builtincallexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    expr_t* name = expr->funccallexpr.name;
    // We are sure that the name will be identexpr, it is enforced in the grammar
    if(strcmp(name->identexpr.value, "print") == 0){
        _codegen_builtincallexpr_print(expr, 0);
    } else if(strcmp(name->identexpr.value, "println") == 0){
        _codegen_builtincallexpr_print(expr, 1);
    } else if(strcmp(name->identexpr.value, "cap") == 0){
        _codegen_builtincallexpr_cap(expr);
    } else if(strcmp(name->identexpr.value, "len") == 0){
        _codegen_builtincallexpr_len(expr);
    } else if(strcmp(name->identexpr.value, "append") == 0){
        _codegen_builtincallexpr_append(expr);
    }
}

static void _codegen_builtincallexpr_print(expr_t* expr, int newline)
{ 
    printf("print(");
    // agruments are seq expressions
    expr_t* sequence = expr->funccallexpr.args;
    int emptyargs = 1;
    while(sequence != NULL)
    {
        emptyargs = 0;
        expr_t* current = sequence->seqexpr.main;
        symboltypekind_t kind= _codegen_resolve_basetype(current->resolved_type)->type;
        switch(kind)
        {
            case k_TYPEFLOAT64:
                printf("\"{0:+e}\".format(");
                _codegen_expr(current);
                printf(")");
                break;
            case k_TYPEBOOL:
                printf("__codegen_helper_get_bool_str(");
                _codegen_expr(current);
                printf(")");
                break;
            default:
                _codegen_expr(current);
                break;
        }
        sequence = sequence->seqexpr.next;
        if(sequence != NULL)
            printf(",");
    }
    // Will loop through them

    if (emptyargs)
    {
        printf("''");
    }

    if(newline){
        printf(")");
    }else{
        printf(", sep = '', end = '')");
    }
}

static void _codegen_builtincallexpr_cap(expr_t* expr)
{
    // cap(element)
    expr_t* element = expr->funccallexpr.args->seqexpr.main;
    // slice
    if(element->resolved_type->dimensions->index == -1){
        _codegen_expr(element);
        printf(".cap");
    } else { // array, implemented as lists
        printf("len(");
        _codegen_expr(element);
        printf(")");
    }
}

static void _codegen_builtincallexpr_len(expr_t* expr)
{
    // len(element)
    expr_t* element = expr->funccallexpr.args->seqexpr.main;

    if(element->resolved_type->isarray && 
        element->resolved_type->dimensions->index == -1){// slice
        _codegen_expr(element);
        printf(".len");
    } else { // arrays or strings
        printf("len(");  
        _codegen_expr(element);
        printf(")");
    }
}

static void _codegen_builtincallexpr_append(expr_t* expr)
{
    // append(slice, element)
    expr_t* slice = expr->funccallexpr.args->seqexpr.main;
    expr_t* element = expr->funccallexpr.args->seqexpr.next->seqexpr.main;
    _codegen_expr(slice);
    printf(".append(");
    _codegen_expr(element);
    printf(")");
}

static void _codegen_indexingexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    expr_t* array = expr->indexingexpr.array;
    
    _codegen_expr(array);
    printf("[");
    _codegen_expr(expr->indexingexpr.index);
    printf("]");  
     
}

static void _codegen_fieldselectorexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    _codegen_expr(expr->fieldselectorexpr.receiver);
    // Grammar ensures it is an identifier and the selector expression does not get linked to a symbol
    printf("['%s']", expr->fieldselectorexpr.selector->identexpr.value);   
}

static void _codegen_seqexpr(expr_t* expr)
{
    if(expr == NULL)
        return;
    _codegen_expr(expr->seqexpr.main);
    if(expr->seqexpr.next != NULL){
        printf(", ");
        _codegen_expr(expr->seqexpr.next);
    }
}

static int _codegen_is_slice_identifier(expr_t* expr)
{
    while(expr->kind == k_PARENTHESESEXPR){
        expr = expr->parenthesesexpr.main;
    }
    return expr->resolved_type->isarray && expr->resolved_type->dimensions->index == -1
            && expr->kind == k_IDENTEXPR;
}

static int _codegen_is_array_identifier(expr_t* expr)
{
    while(expr->kind == k_PARENTHESESEXPR){
        expr = expr->parenthesesexpr.main;
    }
    return expr->resolved_type->isarray && expr->resolved_type->dimensions->index != -1
            && expr->kind == k_IDENTEXPR;
}

static int _codegen_is_struct(expr_t* expr)
{
    while(expr->kind == k_PARENTHESESEXPR){
        expr = expr->parenthesesexpr.main;
    }
    const symboltype_t* resolved_type = _codegen_resolve_type(expr->resolved_type);
    return !resolved_type->isarray && resolved_type->type == k_TYPESTRUCT 
            && expr->kind == k_IDENTEXPR;
}

static void _print_assignment_rhs(expr_t* expr)
{
    if(expr == NULL)
        return;
    // Need to do this to handle the case where slices need to be dupped
    expr_t* sequence = expr;
    while(sequence != NULL)
    {
        expr_t* current = sequence->seqexpr.main;
        if(_codegen_is_slice_identifier(current)){
            printf("%s.dup(", slice_class_name);
            _codegen_expr(current);
            printf(")");
        } else if(_codegen_is_array_identifier(current)){
            printf("deepcopy(");
            _codegen_expr(current);
            printf(")");
        } else if(_codegen_is_struct(current)){
            printf("copy(");
            _codegen_expr(current);
            printf(")");
        } else {
            _codegen_expr(current);
        }
        sequence = sequence->seqexpr.next;
        if(sequence != NULL)
            printf(", ");
    }

}

/*************************************************************************************************/
/* Codegen for statements                                                                            */
/*************************************************************************************************/

static void _codegen_stmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    
    switch(stmt->kind)
    {
        case k_EXPRSTMT: 
            _codegen_exprstmt(stmt);
            break;
        case k_BLOCKSTMT: 
            _codegen_blockstmt(stmt);
            break;
        case k_ASSIGNSTMT: 
            _codegen_assignstmt(stmt);
            break;
        case k_OPASSIGNSTMT: 
            _codegen_opassignstmt(stmt);
            break;
        case k_DECLSTMT: 
            _codegen_declstmt(stmt);
            break;
        case k_SHORTDECLSTMT: 
            _codegen_shortdeclstmt(stmt);
            break;
        case k_INCREMENTSTMT: 
            _codegen_incrementstmt(stmt);
            break;
        case k_DECREMENTSTMT: 
            _codegen_decrementstmt(stmt);
            break;
        case k_RETURNSTMT: 
            _codegen_returnstmt(stmt);
            break;
        case k_IFSTMT: 
            _codegen_ifstmt(stmt);
            break;
        case k_SWITCHSTMT: 
            _codegen_switchstmt(stmt);
            break;
        case k_FORSTMT: 
            _codegen_forstmt(stmt);
            break;
        case k_BREAKSTMT: 
            INDENT(codegen_indent);
            printf("break\n");
            break;
        case k_CONTINUESTMT: 
            _codegen_stmt(loop_post);
            INDENT(codegen_indent);
            printf("continue\n");
            break;
        case k_EMPTYSTMT: 
            break;
        case k_SEQSTMT: 
            _codegen_stmt(stmt->seqstmt.stmt);
            _codegen_stmt(stmt->seqstmt.next);
            break;
        default:
            fprintf(stderr, "Undefined statement kind. \n");
            exit(1);
    }
}

static void _codegen_exprstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    INDENT(codegen_indent);
    _codegen_expr(stmt->exprstmt.expr);
    printf("\n");
}

static void _codegen_blockstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    // Scoping is handled at the symbol level so no need to create a new scope 
    _codegen_stmt(stmt->blockstmt.stmts);
    
}

static expr_t* _codegen_pop_parentheses(expr_t* expr)
{
    while(expr->kind == k_PARENTHESESEXPR)
        expr = expr->parenthesesexpr.main;
    return expr;
}

static int _codegen_same_literal(expr_t* expr1, expr_t* expr2)
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

static int _codegen_same_expr(expr_t* expr1, expr_t* expr2)
{
    if(expr1->kind != expr2->kind)
        return 0;
    switch(expr1->kind)
    {
        case k_IDENTEXPR:   
            return strcmp(expr1->identexpr.value, expr2->identexpr.value) == 0;
        case k_LITERALEXPR:
            return _codegen_same_literal(expr1, expr2);
        case k_INDEXINGEXPR:    
            return _codegen_same_expr(expr1->indexingexpr.array, expr2->indexingexpr.array)
                   && _codegen_same_expr(expr1->indexingexpr.index, expr2->indexingexpr.index);
        case k_FIELDSELECTOREXPR:   
            return _codegen_same_expr(expr1->fieldselectorexpr.receiver, expr2->fieldselectorexpr.receiver)
                   && _codegen_same_expr(expr1->fieldselectorexpr.selector, expr2->fieldselectorexpr.selector);
        default: return 0;
    }
}

static int _codegen_fastappend(expr_t* names, expr_t* exprs)
{
    //make sure only one element is on each side
    if(names->seqexpr.next != NULL || exprs->seqexpr.next != NULL)
    {
        return 0;
    }
    expr_t* name = _codegen_pop_parentheses(names->seqexpr.main);
    expr_t* expr = _codegen_pop_parentheses(exprs->seqexpr.main);
    
    expr_t* funcname = expr->funccallexpr.name;
    if(expr->kind != k_BUILTINCALLEXPR || strcmp("append",funcname->identexpr.value) != 0)
    {
        return 0;
    }
    expr_t* first_funcarg = _codegen_pop_parentheses(expr->funccallexpr.args->seqexpr.main);
    expr_t* second_funcarg = _codegen_pop_parentheses(expr->funccallexpr.args->seqexpr.next->seqexpr.main);
    
    if( !expr->resolved_type->isarray || expr->resolved_type->dimensions->index != -1)
    {
        return 0;
    }
       
    if(!_codegen_same_expr(name, first_funcarg))
    {
        return 0;
    }    
    if(name->resolved_type->requiredslice){
        _codegen_expr(name);
        printf(".fast_append(");
        _codegen_expr(second_funcarg);
        printf(")\n");
    } else {
        _codegen_expr(name);
        printf(".append(copy(");
        _codegen_expr(second_funcarg);
        printf("))\n");
    }

    return 1;
}

static void _codegen_assignstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    INDENT(codegen_indent);
    if(!_codegen_fastappend(stmt->assignstmt.names, stmt->assignstmt.exprs))
    {
        _codegen_expr(stmt->assignstmt.names);
        printf(" = ");
        _print_assignment_rhs(stmt->assignstmt.exprs);
        printf("\n");
    }
}

static void _codegen_opassignstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    INDENT(codegen_indent);
    _codegen_expr(stmt->opassignstmt.name);

    switch(stmt->opassignstmt.kind)
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
            printf(" //= "); // avoid string cast to float
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
            printf(" &= ~ "); // gives the same result
            break; 
    }
    _codegen_expr(stmt->opassignstmt.expr);
    printf("\n");
}   

static void _codegen_declstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;

    _codegen_decl(&stmt->declstmt);
    
}

static void _codegen_shortdeclstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;

    INDENT(codegen_indent);
    _codegen_expr(stmt->shortdeclstmt.names);
    printf(" = ");
    // Need to do this to handle the case where slices need to be dupped
    _print_assignment_rhs(stmt->shortdeclstmt.exprs);
    printf("\n");
}

static void _codegen_incrementstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    INDENT(codegen_indent);
    _codegen_expr(stmt->incrementstmt.expr);
    printf(" += 1");
    printf("\n");
}

static void _codegen_decrementstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    INDENT(codegen_indent);
    _codegen_expr(stmt->decrementstmt.expr);
    printf(" -= 1");
    printf("\n");
}

static void _codegen_returnstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    INDENT(codegen_indent);
    printf("return ");
    if (stmt->returnstmt.expr)
    {
        const symboltype_t* type = _codegen_resolve_type(stmt->returnstmt.expr->resolved_type);
        if (type->type == k_TYPESTRUCT)
        {
            printf("dict(");
            _codegen_expr(stmt->returnstmt.expr);
            printf(")");
        }
        else if (type->isarray && !_codegen_is_slice(type))
        {
            printf("list(");
            _codegen_expr(stmt->returnstmt.expr);
            printf(")");
        }
        else
        {
            _codegen_expr(stmt->returnstmt.expr);
        }
    }
    printf("\n\n");

}

static void _codegen_ifstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    
    // doing it here is ok and will not interfer with other symbols
    // scope number is appended to the name
    _codegen_stmt(stmt->ifstmt.init);
    
    INDENT(codegen_indent);
    printf("if ");
    _codegen_expr(stmt->ifstmt.condition);
    printf(":\n");
    
    codegen_indent++;
    if(stmt->ifstmt.thenblock != NULL){
        _codegen_stmt(stmt->ifstmt.thenblock);
    } else {
        INDENT(codegen_indent);
        printf("pass \n");
    }
    
    
    if(stmt->ifstmt.elseblock != NULL){
        INDENT(codegen_indent-1);
        printf("else:\n");
        _codegen_stmt(stmt->ifstmt.elseblock);
    }
    codegen_indent--;
}

static void _codegen_switchstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;

    _codegen_stmt(stmt->switchstmt.init);

    int condition_container = switch_condition_container_count++;
    INDENT(codegen_indent);
    printf("%s%d = ", switch_condition_container_name, condition_container);
    if (stmt->switchstmt.condition)
    {
        _codegen_expr(stmt->switchstmt.condition);
    }
    else
    {
        printf("True");
    }
    printf("\n");
    

    INDENT(codegen_indent);
    printf("while(True):\n");

    codegen_indent++;

    stmt_t* switchcase = stmt->switchstmt.cases;
    stmt_t* defaultcase = NULL;
    int firstcase = 1;

    while(switchcase != NULL)
    {
        if (switchcase->seqstmt.stmt->switchcasestmt.exprs == NULL)
        {
            defaultcase = switchcase;
            switchcase = switchcase->seqstmt.next;
            continue;
        }
        
        _codegen_switchcasestmt(switchcase->seqstmt.stmt, condition_container, firstcase);
        switchcase = switchcase->seqstmt.next;
        firstcase = 0;
    }

    if (defaultcase != NULL)
    {
        _codegen_switchcasestmt(defaultcase->seqstmt.stmt, condition_container, firstcase);
    }

    INDENT(codegen_indent);
    printf("break\n");

    codegen_indent--;
}

static void _codegen_switchcasestmt(stmt_t* stmt, int condition_container, int firstcase)
{
    if(stmt == NULL)
        return;

    INDENT(codegen_indent);

    if (stmt->switchcasestmt.exprs != NULL)
    {
        if (firstcase)
        {
            printf("if ");
        }
        else
        {
            printf("elif ");
        }

        expr_t* caseexpr = stmt->switchcasestmt.exprs;
        while(caseexpr != NULL)
        {
            _codegen_expr(caseexpr->seqexpr.main);
            printf(" == %s%d", switch_condition_container_name, condition_container);

            if (caseexpr->seqexpr.next != NULL)
            {
                printf(" or ");
            }

            caseexpr = caseexpr->seqexpr.next;
        }

        printf(":\n");
    }
    else
    {
        if (firstcase)
        {
            printf("if True: \n");
        }
        else
        {
            printf("else: \n");
        }
    }
    

    codegen_indent++;
    _codegen_stmt(stmt->switchcasestmt.stmts);

    if (stmt->switchcasestmt.stmts == NULL)
    {
        INDENT(codegen_indent);
        printf("pass\n");
    }
    codegen_indent--;
}

static void _codegen_forstmt(stmt_t* stmt)
{
    if(stmt == NULL)
        return;

    stmt_t* prev_loop_post = loop_post;
    loop_post = stmt->forstmt.post;
    _codegen_stmt(stmt->forstmt.init);

    INDENT(codegen_indent);
    printf("while ");
    if (stmt->forstmt.condition != NULL)
    {
        _codegen_expr(stmt->forstmt.condition);
    }
    else
    {
        printf("True");
    }

    printf(":\n");

    codegen_indent++;
    _codegen_stmt(stmt->forstmt.loopblock);
    _codegen_stmt(stmt->forstmt.post);

    if (stmt->forstmt.loopblock == NULL && stmt->forstmt.post == NULL)
    {
        INDENT(codegen_indent);
        printf("pass\n");
    }
    codegen_indent--;

    loop_post = prev_loop_post;
}

