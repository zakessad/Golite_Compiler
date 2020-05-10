#include<stdio.h>

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typecheck.h"
#include "symbol.h"
#include "tree.h"


/*************************************************************************************************/
/* External variables and functions                                                              */
/*************************************************************************************************/

extern symbol_t* basetype_bool;
extern symbol_t* basetype_int;
extern symbol_t* basetype_float64;
extern symbol_t* basetype_rune;
extern symbol_t* basetype_string;


/*************************************************************************************************/
/* Global context variables                                                                      */
/*************************************************************************************************/

static symboltype_t* _context_return_type = NULL;


/*************************************************************************************************/
/* Function prototypes                                                                           */
/*************************************************************************************************/
static symboltype_t* _make_symbol_type(symbol_t* parent);

static symboltype_t* resolve_type(const symboltype_t* symboltype);

static bool _is_comparable_type(const symboltype_t* symboltype);
static bool _is_ordered_type(const symboltype_t* symboltype);
static bool _is_numeric_type(const symboltype_t* symboltype);
static bool _is_integer_type(const symboltype_t* symboltype);

static bool _types_are_identical(const symboltype_t* t1, const symboltype_t* t2);

static bool _type_is_convertible(const symboltype_t* dst, const symboltype_t* src);

static symboltype_t* _typecheck_expr(expr_t* expr);
static void _typecheck_stmt(stmt_t* stmt);
static void _typecheck_decl(decl_t* decl);

static void _error_incompatible_type(int lineno, char* error, const symboltype_t* t1, const symboltype_t* t2);
static void _print_type(const symboltype_t* t);
static void _print_array(arrayindex_t* dimension);
static void _print_struct(symbol_t* structelements);
/*************************************************************************************************/
/* Miscellaneous helper functions                                                                */
/*************************************************************************************************/
static symboltype_t* _make_symbol_type(symbol_t* parent)
{
    symboltype_t* new_type = (symboltype_t*) malloc(sizeof(symboltype_t));
    new_type->name = parent->name;
    new_type->parent = parent;
    new_type->type = k_TYPENOTBASE;
    new_type->isarray = 0;
    new_type->dimensions = NULL;
    return new_type;
}

/*
 * Resolve a type to it's base type, stopping when an underlying array/slice type is encountered.
 *
 * e.g.
 *
 * type A int // A -> int
 * type B A   // B -> int
 * type C []A // C -> []A
 */
static symboltype_t* resolve_type(const symboltype_t* symboltype)
{
    if (symboltype->type == k_TYPENOTBASE && !symboltype->isarray)
    {
        return resolve_type(&symboltype->parent->type);
    }
    else
    {
        return (symboltype_t*)symboltype;
    }
}

/*
 * Determine whether an expression is an lvalue (i.e. addressable).
 * 
 * Valid lvalues in Golite consist of:
 * - Variables (non-constant)
 * - Slice indexing
 * - Array indexing of an addressable array
 * - Field selection of an addressable struct
 */
static int _is_lvalue(const expr_t* expr)
{
    symboltype_t* array_symtype = NULL;
    switch(expr->kind)
    {
        case k_IDENTEXPR:
            if(expr->identexpr.symbol->kind == k_CONSTANT)
                return 0;
            return 1;
        case k_INDEXINGEXPR:
            array_symtype = expr->indexingexpr.array->resolved_type;
            return (array_symtype->isarray && array_symtype->dimensions->index == -1) 
                    ||_is_lvalue(expr->indexingexpr.array);
        case k_FIELDSELECTOREXPR:
            return _is_lvalue(expr->fieldselectorexpr.receiver);
        case k_PARENTHESESEXPR:
            return _is_lvalue(expr->parenthesesexpr.main);
        case k_LITERALEXPR:
        case k_UNARYEXPR:
        case k_BINARYEXPR:
        case k_FUNCCALLEXPR:
        case k_BUILTINCALLEXPR:
        case k_SEQEXPR:
        default: ;
            return 0;
    }
}

static void _error_incompatible_type(int lineno, char* error, const symboltype_t* t1, const symboltype_t* t2)
{
    fprintf(stderr, "Error: [line %d] %s [ ", lineno, error);
    _print_type(t1);
    fprintf(stderr, " != ");
    _print_type(t2);
    fprintf(stderr, " ]\n");
    exit(1);
}

static void _print_type(const symboltype_t* type)
{
    
    switch(type->type)
    {
        case k_TYPEINT:
            fprintf(stderr,"int");
            break;
        case k_TYPEFLOAT64:
            fprintf(stderr,"float64");
            break;
        case k_TYPERUNE:
            fprintf(stderr,"rune");
            break;
        case k_TYPEBOOL:
            fprintf(stderr,"bool");
            break;
        case k_TYPECSTRING:
            fprintf(stderr,"string");
            break;
        case k_TYPEVOID:
            fprintf(stderr,"void");
            break;
        case k_TYPESTRUCT:
            if(type->isarray) _print_array(type->dimensions);
            fprintf(stderr,"struct { ");
            _print_struct(type->structsymbols);
            fprintf(stderr," }");
            break;
        case k_TYPENOTBASE:
            fprintf(stderr,"%s", type->name);
            break;
        case k_TYPEINFERED:
            fprintf(stderr,"infered");
            break;
        default:
            fprintf(stderr, "Error: Unknown type kind \n");
            exit(1);        
    }           
}

static void _print_array(arrayindex_t* dimension)
{
    while (dimension != NULL)
    {
        if (dimension->index != -1) fprintf(stderr,"[%d]", dimension->index);
        else fprintf(stderr,"[]"); 
        dimension = dimension->next;
    }
}

static void _print_struct(symbol_t* structelements)
{
    while(structelements != NULL){
        fprintf(stderr,"%s ", structelements->name);
        _print_type(&(structelements->type));
        fprintf(stderr,"; ");
        structelements = structelements->next;
    }
}

/*************************************************************************************************/
/* Type-category functions (i.e. _is_*_type() functions)                                         */
/*************************************************************************************************/

/* Returns true if the type is 'comparable', and false otherwise. */
/* See: https://golang.org/ref/spec#Comparison_operators */
static bool _is_comparable_type(const symboltype_t* symboltype)
{
    const symbol_t* symbol = NULL;

    while (1)
    {
        if (symboltype->isarray)
        {
            for (const arrayindex_t* arrayindex = symboltype->dimensions; arrayindex; arrayindex = arrayindex->next)
            {
                if (arrayindex->index == -1)
                {
                    return false;
                }
            }
        }

        if (symboltype->type != k_TYPENOTBASE)
        {
            break;
        }

        symboltype = &symboltype->parent->type;
    }

    switch (symboltype->type)
    {
        case k_TYPEVOID:
            return false;

        case k_TYPEBOOL:
            return true;
        case k_TYPEINT:
            return true;
        case k_TYPEFLOAT64:
            return true;
        case k_TYPERUNE:
            return true;
        case k_TYPECSTRING:
            return true;

        case k_TYPESTRUCT:
            for (symbol = symboltype->structsymbols; symbol; symbol = symbol->next)
            {
                if (!_is_comparable_type(&symbol->type))
                {
                    return false;
                }
            }

            return true;

        case k_TYPENOTBASE:
            assert("Unresolved base type returned after base type resolution" && 0);
            abort();

        case k_TYPEINFERED:
            assert("Infered type encountered before its resolution" && 0);
            abort();

        default:
            assert("Unhandled symbol type kind" && 0);
            abort();
    }

    /* Unreachable */

    return false;
}

/* Returns true if the type is 'ordered', and false otherwise. */
/* See: https://golang.org/ref/spec#Comparison_operators */
static bool _is_ordered_type(const symboltype_t* symboltype)
{
    symboltype = resolve_type(symboltype);
    switch (symboltype->type)
    {
        case k_TYPEVOID:
            return false;

        case k_TYPEBOOL:
            return false;
        case k_TYPEINT:
            return !symboltype->isarray;
        case k_TYPEFLOAT64:
            return !symboltype->isarray;
        case k_TYPERUNE:
            return !symboltype->isarray;
        case k_TYPECSTRING:
            return !symboltype->isarray;

        case k_TYPESTRUCT:
            return false;

        case k_TYPENOTBASE:
            return false;
            // assert("Unresolved base type returned after base type resolution" && 0);
            // abort();

        case k_TYPEINFERED:
            return false;
            // assert("Infered type encountered before its resolution" && 0);
            // abort();

        default:
            assert("Unhandled symbol type kind" && 0);
            abort();
    }

    /* Unreachable */

    return false;
}

/* Returns true if the type is 'numeric', and false otherwise. */
/* See: https://golang.org/ref/spec#Numeric_types */
static bool _is_numeric_type(const symboltype_t* symboltype)
{
    symboltype = resolve_type(symboltype);
    switch (symboltype->type)
    {
        case k_TYPEVOID:
            return false;

        case k_TYPEBOOL:
            return false;
        case k_TYPEINT:
            return !symboltype->isarray;
        case k_TYPEFLOAT64:
            return !symboltype->isarray;
        case k_TYPERUNE:
            return !symboltype->isarray;
        case k_TYPECSTRING:
            return false;

        case k_TYPESTRUCT:
            return false;

        case k_TYPENOTBASE:
            return false;

        case k_TYPEINFERED:
            assert("Infered type encountered before its resolution" && 0);
            abort();

        default:
            assert("Unhandled symbol type kind" && 0);
            abort();
    }

    /* Unreachable */

    return false;
}

/* Returns true if the type is 'integer', and false otherwise. */
/* See: https://golang.org/ref/spec#Numeric_types */
static bool _is_integer_type(const symboltype_t* symboltype)
{
    symboltype = resolve_type(symboltype);
    switch (symboltype->type)
    {
        case k_TYPEVOID:
            return false;

        case k_TYPEBOOL:
            return false;
        case k_TYPEINT:
            return !symboltype->isarray;
        case k_TYPEFLOAT64:
            return false;
        case k_TYPERUNE:
            return !symboltype->isarray;
        case k_TYPECSTRING:
            return false;

        case k_TYPESTRUCT:
            return false;

        case k_TYPENOTBASE:
            return false;

        case k_TYPEINFERED:
            assert("Infered type encountered before its resolution" && 0);
            abort();

        default:
            assert("Unhandled symbol type kind" && 0);
            abort();
    }

    /* Unreachable */

    return false;
}


/*************************************************************************************************/
/* Type-equality functions                                                                       */
/*************************************************************************************************/

static bool _types_have_same_array_dimensions(const symboltype_t* t1, const symboltype_t* t2)
{
    arrayindex_t* ai_1 = t1->dimensions;
    arrayindex_t* ai_2 = t2->dimensions;

    while (1)
    {
        if (!(ai_1 && ai_2))
        {
            return ai_1 == ai_2;
        }
        if (ai_1->index != ai_2->index)
        {
            return false;
        }

        ai_1 = ai_1->next;
        ai_2 = ai_2->next;
    }

    return true;
}

static bool _types_have_same_struct_elements(const symboltype_t* t1, const symboltype_t* t2)
{
    symbol_t* s_1 = t1->structsymbols;
    symbol_t* s_2 = t2->structsymbols;

    while (1)
    {
        if (!(s_1 && s_2))
        {
            return s_1 == s_2;
        }
        if (strcmp(s_1->name, s_2->name) != 0)
        {
            return false;
        }
        if (!_types_are_identical(&s_1->type, &s_2->type))
        {
            return false;
        }

        s_1 = s_1->next;
        s_2 = s_2->next;
    }

    return true;
}


/*
 * Returns true if the types are identical, and false otherwise.
 *
 * e.g.
 *
 * type A int                       // RT(A) == int; A != int
 * type B int                       // A != B; RT(A) == RT(B)
 * type C A                         // A != C; RT(C) == RT(A); RT(C) == RT(B); RT(C) != A
 * type D B                         // B != D; RT(D) == RT(A); RT(D) == RT(B); RT(D) != B
 * type E []A                       // A != E; RT(E) != []int; RT(E) == []A
 * type F []int                     // E != F; RT(F) == []int; RT(F) != RT(E)
 * type G struct { x, y, z int; }   // G != struct { x, y, z int; }; G != struct { z, y, x int; }
 */
static bool _types_are_identical(const symboltype_t* t1, const symboltype_t* t2)
{
    if (t1->type != t2->type)
    {
        // printf("TYPE DIF \n");
        fflush(stdout);
        return false;
    }

    if (t1->type == k_TYPENOTBASE && t1->parent != t2->parent)
    {
        // printf("PARENT DIF \n");
        fflush(stdout);
        return false;
    }

    if (t1->isarray != t2->isarray || (t1->isarray && t2->isarray && !_types_have_same_array_dimensions(t1, t2)))
    {
        // printf(" ARRAY DIF \n");
        fflush(stdout);
        return false;
    }

    if (t1->type == k_TYPESTRUCT && !_types_have_same_struct_elements(t1, t2))
    {
        // printf("STRUCTDIF \n");
        fflush(stdout);
        return false;
    }

    return true;
}


/*************************************************************************************************/
/* Type-convertibility functions                                                                 */
/*************************************************************************************************/

/*
 * Returns true if type `src' is convertible to type `dst', and false otherwise.
 *
 * e.g.
 *
 * type A int                       // int <- A [OK]; A <- int [OK]
 * type B int                       // A <- B [OK]; B <- A [OK]
 * type C A                         // int <- C [OK]; C <- int [OK]; A <- C [OK]; C <- B [OK]
 * type D B                         // C <- D [OK]; D <- C [OK]; B <- D [OK]; D <- A [OK]
 * type E []A                       // not convertible
 * type F []int                     // not convertible
 * type G struct { x, y, z int; }   // not convertible
 */
static bool _type_is_convertible(const symboltype_t* dst, const symboltype_t* src)
{
    dst = resolve_type(dst);
    src = resolve_type(src);

    if (dst->isarray || src->isarray)
    {
        return false;
    }
    if (dst->type != k_TYPEBOOL && dst->type != k_TYPEINT && dst->type != k_TYPEFLOAT64
            && dst->type != k_TYPERUNE && dst->type != k_TYPECSTRING)
    {
        return false;
    }

    if (_types_are_identical(dst, src))
    {
        return true;
    }
    if (_is_numeric_type(dst) && _is_numeric_type(src))
    {
        return true;
    }
    if (dst->type == k_TYPECSTRING && _is_integer_type(src))
    {
        return true;
    }

    return false;
}


/*************************************************************************************************/
/* Expression typecheck functions                                                                */
/*************************************************************************************************/

static symboltype_t* _typecheck_expr_identexpr(expr_t* expr)
{
    expr->resolved_type = &expr->identexpr.symbol->type;
    
    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_literalexpr(expr_t* expr)
{
    switch (expr->literalexpr.kind)
    {
        case k_INTLITERALEXPR:
            expr->resolved_type = _make_symbol_type(basetype_int);
            break;
        case k_FLTLITERALEXPR:
            expr->resolved_type = _make_symbol_type(basetype_float64);
            break;
        case k_STRLITERALEXPR:
            expr->resolved_type = _make_symbol_type(basetype_string);
            break;
        case k_CHRLITERALEXPR:
            expr->resolved_type = _make_symbol_type(basetype_rune);
            break;

        default:
            assert("Unhandled literal expression kind" && 0);
            abort();
    }

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_unaryexpr(expr_t* expr)
{
    symboltype_t* symboltype = _typecheck_expr(expr->unaryexpr.expr);

    switch (expr->unaryexpr.kind)
    {
        case k_POSUNARYEXPR:
            if (!_is_numeric_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Unary '+' operation on expression with non-numeric type.\n", expr->lineno);
                exit(1);
            }
            break;
        case k_NEGUNARYEXPR:
            if (!_is_numeric_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Unary '-' operation on expression with non-numeric type.\n", expr->lineno);
                exit(1);
            }
            break;
        case k_NOTUNARYEXPR:
            if (resolve_type(symboltype)->type != k_TYPEBOOL)
            {
                fprintf(stderr, "Error: [line %d] Unary '!' operation on expression with non-boolean type.\n", expr->lineno);
                exit(1);
            }
            break;
        case k_BITNOTUNARYEXPR:
            if (!_is_integer_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Unary '^' operation on expression with non-integer type.\n", expr->lineno);
                exit(1);
            }
            break;

        default:
            assert("Unhandled unary expression kind" && 0);
            abort();
    }

    expr->resolved_type = symboltype;

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_binaryexpr(expr_t* expr)
{
    symboltype_t* l_symboltype = _typecheck_expr(expr->binaryexpr.l_expr);
    symboltype_t* r_symboltype = _typecheck_expr(expr->binaryexpr.r_expr);

    symboltype_t* symboltype = resolve_type(l_symboltype);

    if (!_types_are_identical(l_symboltype, r_symboltype))
    {
        _error_incompatible_type(expr->lineno, "Incompatible operand types in binary expression", l_symboltype, r_symboltype);
        exit(1);
    }

    switch (expr->binaryexpr.kind)
    {
        case k_ADDBINARYEXPR:
            if (!_is_numeric_type(symboltype) && (symboltype->isarray || symboltype->type != k_TYPECSTRING))
            {
                fprintf(stderr, "Error: [line %d] Binary '+' operation on expression with non-numeric/non-string type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_SUBBINARYEXPR:
            if (!_is_numeric_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '-' operation on expression with non-numeric type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_MULBINARYEXPR:
            if (!_is_numeric_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '*' operation on expression with non-numeric type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_DIVBINARYEXPR:
            if (!_is_numeric_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '/' operation on expression with non-numeric type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_MODBINARYEXPR:
            if (!_is_integer_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '%%' operation on expression with non-integer type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_LSHIFTBINARYEXPR:
            if (!_is_integer_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '<<' operation on expression with non-integer type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_RSHIFTBINARYEXPR:
            if (!_is_integer_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '>>' operation on expression with non-integer type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_BITWISEANDBINARYEXPR:
            if (!_is_integer_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '&' operation on expression with non-integer type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_BITWISEORBINARYEXPR:
            if (!_is_integer_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '|' operation on expression with non-integer type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_BITWISEXORBINARYEXPR:
            if (!_is_integer_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '^' operation on expression with non-integer type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_BITWISECLRBINARYEXPR:
            if (!_is_integer_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '&^' operation on expression with non-integer type.\n", expr->lineno);
                exit(1);
            }
            symboltype = l_symboltype;
            break;
        case k_EQBINARYEXPR:
            if (!_is_comparable_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '==' operation on expression with non-comparable type.\n", expr->lineno);
                exit(1);
            }
            // symboltype = &basetype_bool->type;
            symboltype = _make_symbol_type(basetype_bool);
            break;
        case k_NEBINARYEXPR:
            if (!_is_comparable_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '!=' operation on expression with non-comparable type.\n", expr->lineno);
                exit(1);
            }
            // symboltype = &basetype_bool->type;
            symboltype = _make_symbol_type(basetype_bool);
            break;
        case k_LTBINARYEXPR:
            if (!_is_ordered_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '<' operation on expression with non-ordered type.\n", expr->lineno);
                exit(1);
            }
            // symboltype = &basetype_bool->type;
            symboltype = _make_symbol_type(basetype_bool);
            break;
        case k_LEBINARYEXPR:
            if (!_is_ordered_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '<=' operation on expression with non-ordered type.\n", expr->lineno);
                exit(1);
            }
            // symboltype = &basetype_bool->type;
            symboltype = _make_symbol_type(basetype_bool);
            break;
        case k_GTBINARYEXPR:
            if (!_is_ordered_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '>' operation on expression with non-ordered type.\n", expr->lineno);
                exit(1);
            }
            // symboltype = &basetype_bool->type;
            symboltype = _make_symbol_type(basetype_bool);
            break;
        case k_GEBINARYEXPR:
            if (!_is_ordered_type(symboltype))
            {
                fprintf(stderr, "Error: [line %d] Binary '>=' operation on expression with non-ordered type.\n", expr->lineno);
                exit(1);
            }
            // symboltype = &basetype_bool->type;
            symboltype = _make_symbol_type(basetype_bool);
            break;
        case k_LOGICALANDBINARYEXPR:
            if (symboltype->type != k_TYPEBOOL)
            {
                fprintf(stderr, "Error: [line %d] Binary '&&' operation on expression with non-boolean type.\n", expr->lineno);
                exit(1);
            }
            // symboltype = &basetype_bool->type;
            // symboltype = _make_symbol_type(basetype_bool);
            symboltype = l_symboltype;
            break;
        case k_LOGICALORBINARYEXPR:
            if (symboltype->type != k_TYPEBOOL)
            {
                fprintf(stderr, "Error: [line %d] Binary '||' operation on expression with non-boolean type.\n", expr->lineno);
                exit(1);
            }
            // symboltype = &basetype_bool->type;
            // symboltype = _make_symbol_type(basetype_bool);
            symboltype = l_symboltype;
            break;

        default:
            assert("Unhandled binary expression kind" && 0);
            abort();
    }

    expr->resolved_type = symboltype;

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_funccallexpr_funccall(expr_t* expr)
{
    symbol_t* symbol = expr->funccallexpr.symbol;

    if (strncmp(symbol->name, "init", 5) == 0)
    {
        fprintf(stderr, "Error: [line %d] Special function 'init' may not be called.\n", expr->lineno);
        exit(1);
    }

    const expr_t* explist = expr->funccallexpr.args;

    const typelist_t* arglist = symbol->funcinfo;

    const symboltype_t* arg_symboltype = NULL;
    const symboltype_t* exp_symboltype = NULL;

    for (int i = 0; explist && arglist; arglist = arglist->next, explist = explist->seqexpr.next, ++i)
    {
        exp_symboltype = _typecheck_expr(explist->seqexpr.main);
        arg_symboltype = arglist->current;
        if (!_types_are_identical(exp_symboltype, arg_symboltype))
        {
            char* error_msg = (char*) malloc(sizeof(char)*(15 + strlen("Function call argument has incompatible type")));
            sprintf(error_msg, "Function call argument %d has incompatible type", i);
            _error_incompatible_type(expr->lineno, error_msg, exp_symboltype, arg_symboltype);
            exit(1);
        }
    }

    if (explist && !arglist)
    {
        fprintf(stderr, "Error: [line %d] Function call with too few arguments.\n", expr->lineno);
        exit(1);
    }
    if (arglist && !explist)
    {
        fprintf(stderr, "Error: [line %d] Function call with too many arguments.\n", expr->lineno);
        exit(1);
    }

    expr->resolved_type = &symbol->type;

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_funccallexpr_typecast(expr_t* expr)
{
    if (expr == NULL || expr->funccallexpr.args == NULL || expr->funccallexpr.args->seqexpr.next)
    {
        fprintf(stderr, "Error: [line %d] Typecast expects 1 argument.\n", expr->lineno);
        exit(1);
    }

    symboltype_t* expr_symboltype = _typecheck_expr(expr->funccallexpr.args->seqexpr.main);
    //symboltype_t* cast_symboltype = &expr->funccallexpr.symbol->type;
    symboltype_t* cast_symboltype = _make_symbol_type(expr->funccallexpr.symbol);
    if (!_type_is_convertible(cast_symboltype, expr_symboltype))
    {
        _error_incompatible_type(expr->lineno, "Typecast between incompatible types", cast_symboltype, expr_symboltype);
    }

    expr->resolved_type = cast_symboltype;

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_funccallexpr(expr_t* expr)
{
    _typecheck_expr(expr->funccallexpr.name);

    switch (expr->funccallexpr.symbol->kind)
    {
        case k_FUNCTION:
            return _typecheck_expr_funccallexpr_funccall(expr);
        case k_TYPE:
            return _typecheck_expr_funccallexpr_typecast(expr);

        case k_VARIABLE:
            fprintf(stderr, "Error: [line %d] Function call expression on non-function type.\n", expr->lineno);
            exit(1);
        case k_CONSTANT:
            fprintf(stderr, "Error: [line %d] Function call expression on non-function type.\n", expr->lineno);
            exit(1);

        default:
            assert("Unhandled symboltype kind" && 0);
            abort();
    }

    /* Unreachable */

    return NULL;
}

static symboltype_t* _typecheck_expr_builtinfunccallexpr_println(expr_t* expr)
{
    const expr_t* explist = expr->funccallexpr.args;
    // if (!explist)
    // {
    //     fprintf(stderr, "Error: [line %d] Builtin function 'println' expects at least 1 argument.\n", expr->lineno);
    //     exit(1);
    // }

    for (int i = 0; explist; explist = explist->seqexpr.next, ++i)
    {
        const symboltype_t* symboltype = resolve_type(_typecheck_expr(explist->seqexpr.main));
        if (symboltype->type != k_TYPEBOOL && symboltype->type != k_TYPEINT && symboltype->type != k_TYPEFLOAT64
                && symboltype->type != k_TYPERUNE && symboltype->type != k_TYPECSTRING)
        {
            fprintf(stderr, "Error: [line %d] Builtin function 'println' expects base types.\n", expr->lineno);
            exit(1);
        }
    }

    expr->resolved_type = &expr->funccallexpr.symbol->type;

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_builtinfunccallexpr_print(expr_t* expr)
{
    const expr_t* explist = expr->funccallexpr.args;
    // if (!explist)
    // {
    //     fprintf(stderr, "Error: [line %d] Builtin function 'print' expects at least 1 argument.\n", expr->lineno);
    //     exit(1);
    // }

    for (int i = 0; explist; explist = explist->seqexpr.next, ++i)
    {
        const symboltype_t* symboltype = resolve_type(_typecheck_expr(explist->seqexpr.main));
        if (symboltype->type != k_TYPEBOOL && symboltype->type != k_TYPEINT && symboltype->type != k_TYPEFLOAT64
                && symboltype->type != k_TYPERUNE && symboltype->type != k_TYPECSTRING)
        {
            fprintf(stderr, "Error: [line %d] Builtin function 'print' expects base types.\n", expr->lineno);
            exit(1);
        }
    }

    expr->resolved_type = &expr->funccallexpr.symbol->type;

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_builtinfunccallexpr_append(expr_t* expr)
{
    if (!expr->funccallexpr.args->seqexpr.next)
    {
        fprintf(stderr, "Error: [line %d] Builtin function 'append' expects 2 arguments.\n", expr->lineno);
        exit(1);
    }
    if (expr->funccallexpr.args->seqexpr.next->seqexpr.next)
    {
        fprintf(stderr, "Error: [line %d] Builtin function 'append' expects 2 arguments.\n", expr->lineno);
        exit(1);
    }

    expr_t* e1 = expr->funccallexpr.args->seqexpr.main;
    expr_t* e2 = expr->funccallexpr.args->seqexpr.next->seqexpr.main;
    symboltype_t* e1_symboltype = _typecheck_expr(e1);
    symboltype_t* e2_symboltype = _typecheck_expr(e2);

    symboltype_t temp_symboltype = *resolve_type(e1_symboltype);
    if(temp_symboltype.isarray){
        if(temp_symboltype.dimensions->index != -1){
            fprintf(stderr, "Error: [line %d] Append expected a slice type as first argument. \n", expr->lineno );
            exit(1);            
        }
        temp_symboltype.dimensions = temp_symboltype.dimensions->next;
        if(temp_symboltype.dimensions == NULL){
            temp_symboltype.isarray = 0;
        }
    } else {
        fprintf(stderr, "Error: [line %d] Append expected a slice type as first argument. \n", expr->lineno );
        exit(1);
    }

    if (!_types_are_identical(&temp_symboltype, e2_symboltype))
    {
        _error_incompatible_type(expr->lineno, "Builtin function 'append' slice type is incompatible with element type", &temp_symboltype, e2_symboltype);
    }

    expr->resolved_type = e1_symboltype;

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_builtinfunccallexpr_len(expr_t* expr)
{
    if (expr->funccallexpr.args->seqexpr.next)
    {
        fprintf(stderr, "Error: [line %d] Builtin function 'len' expects 1 argument.\n", expr->lineno);
        exit(1);
    }

    const symboltype_t* symboltype = _typecheck_expr(expr->funccallexpr.args->seqexpr.main);
    symboltype = resolve_type(symboltype);
    if (!symboltype->isarray && symboltype->type != k_TYPECSTRING)
    {
        fprintf(stderr, "Error: [line %d] Expression of non-array/slice/string type passed to builtin function 'len'.\n", expr->lineno);
        exit(1);
    }

    // expr->resolved_type = &basetype_int->type;
    expr->resolved_type = _make_symbol_type(basetype_int);
    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_builtinfunccallexpr_cap(expr_t* expr)
{
    if (expr->funccallexpr.args->seqexpr.next)
    {
        fprintf(stderr, "Error: [line %d] Builtin function 'cap' expects 1 argument.\n", expr->lineno);
        exit(1);
    }

    const symboltype_t* symboltype = _typecheck_expr(expr->funccallexpr.args->seqexpr.main);
    symboltype = resolve_type(symboltype);
    if (!symboltype->isarray)
    {
        fprintf(stderr, "Error: [line %d] Expression of non-array/slice type passed to builtin function 'cap'.\n", expr->lineno);
        exit(1);
    }

    // expr->resolved_type = &basetype_int->type;
    expr->resolved_type = _make_symbol_type(basetype_int);
    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_builtinfunccallexpr(expr_t* expr)
{
    if (strncmp(expr->funccallexpr.name->identexpr.value, "println", 8) == 0)
    {
        return _typecheck_expr_builtinfunccallexpr_println(expr);
    }
    if (strncmp(expr->funccallexpr.name->identexpr.value, "print", 6) == 0)
    {
        return _typecheck_expr_builtinfunccallexpr_print(expr);
    }
    if (strncmp(expr->funccallexpr.name->identexpr.value, "len", 4) == 0)
    {
        return _typecheck_expr_builtinfunccallexpr_len(expr);
    }
    if (strncmp(expr->funccallexpr.name->identexpr.value, "cap", 4) == 0)
    {
        return _typecheck_expr_builtinfunccallexpr_cap(expr);
    }
    if (strncmp(expr->funccallexpr.name->identexpr.value, "append", 7) == 0)
    {
        return _typecheck_expr_builtinfunccallexpr_append(expr);
    }

    assert("Unhandled builtin function call" && 0);
    abort();

    /* Unreachable */

    return NULL;
}

static symboltype_t* _typecheck_expr_indexingexpr(expr_t* expr)
{
    const symboltype_t* array_symboltype = resolve_type(_typecheck_expr(expr->indexingexpr.array));
    const symboltype_t* index_symboltype = resolve_type(_typecheck_expr(expr->indexingexpr.index));

    if (!array_symboltype->isarray)
    {
        fprintf(stderr, "Error: [line %d] Target of indexing expression is of non-array/slice type.\n", expr->lineno);
        exit(1);
    }
    if (index_symboltype->type != k_TYPEINT)
    {
        fprintf(stderr, "Error: [line %d] Index of indexing expression must be of type int.\n", expr->lineno);
        exit(1);
    }

    symboltype_t* symboltype = malloc(sizeof(symboltype_t));
    if (!symboltype)
    {
        fprintf(stderr, "Error: malloc(): %s\n", strerror(errno));
        exit(1);
    }

    *symboltype = *array_symboltype;
    symboltype->dimensions = symboltype->dimensions->next;
    if (!symboltype->dimensions)
    {
        symboltype->isarray = 0;
    }

    expr->resolved_type = symboltype;

    return expr->resolved_type;
}

static symboltype_t* _typecheck_expr_fieldselectorexpr(expr_t* expr)
{
    symboltype_t* symboltype = resolve_type(_typecheck_expr(expr->fieldselectorexpr.receiver));
    if (symboltype->type != k_TYPESTRUCT)
    {
        fprintf(stderr, "Error: [line %d] Receiver of field selection expression must be a struct type.\n", expr->lineno);
        exit(1);
    }

    symbol_t* symbol = symboltype->structsymbols;
    for ( ; symbol; symbol = symbol->next)
    {
        if (strcmp(expr->fieldselectorexpr.selector->identexpr.value, symbol->name) == 0)
        {
            break;
        }
    }

    if (!symbol)
    {
        fprintf(stderr, "Error: [line %d] Receiver of field selection expression has no field '%s'.\n",
            expr->lineno, expr->fieldselectorexpr.selector->identexpr.value
            );
        exit(1);
    }

    expr->resolved_type = &symbol->type;

    return expr->resolved_type;
}


static symboltype_t* _typecheck_expr(expr_t* expr)
{
    if (expr == NULL) return NULL;
    switch (expr->kind)
    {
        case k_PARENTHESESEXPR:
            return expr->resolved_type = _typecheck_expr(expr->parenthesesexpr.main);

        case k_IDENTEXPR:
            return _typecheck_expr_identexpr(expr);
        case k_LITERALEXPR:
            return _typecheck_expr_literalexpr(expr);
        case k_UNARYEXPR:
            return _typecheck_expr_unaryexpr(expr);
        case k_BINARYEXPR:
            return _typecheck_expr_binaryexpr(expr);
        case k_FUNCCALLEXPR:
            return _typecheck_expr_funccallexpr(expr);
        case k_BUILTINCALLEXPR:
            return _typecheck_expr_builtinfunccallexpr(expr);
        case k_INDEXINGEXPR:
            return _typecheck_expr_indexingexpr(expr);
        case k_FIELDSELECTOREXPR:
            return _typecheck_expr_fieldselectorexpr(expr);

        default:
            assert("Unhandled expression kind" && 0);
            abort();
    }

    /* Unreachable */

    return NULL;
}


/*************************************************************************************************/
/* Statement typecheck functions                                                                 */
/*************************************************************************************************/

static void _typecheck_vardecl(const expr_t* name, expr_t* expr)
{
    if (expr == NULL) return;

    symboltype_t* expr_type = _typecheck_expr(expr);
    symboltype_t* name_type = (&name->identexpr.symbol->type);
    if (name_type->type != k_TYPEINFERED && !_types_are_identical(name_type, expr_type))
    {
        _error_incompatible_type(name->lineno, "Incompatible assignment", name_type, expr_type);
    } else if (expr_type->type == k_TYPEVOID) 
    {
        fprintf(stderr, "Error: [line %d] void functions cannot be used in assignments\n", 
            name->lineno
        );
        exit(1);
    }

    if (name_type->type == k_TYPEINFERED)
    {
        name->identexpr.symbol->type = *expr_type;
    }
}

static void _typecheck_funcdecl(const decl_t* decl)
{
    _context_return_type = &(decl->funcdecl.symbol->type);
    _typecheck_stmt(decl->funcdecl.stmts);
    _context_return_type = NULL;
}

static void _typecheck_shortdecl(expr_t* name, expr_t* expr)
{
    if (expr == NULL) return;

    symboltype_t* name_type = _typecheck_expr(name);
    symboltype_t* expr_type = _typecheck_expr(expr);
    if (name_type->type != k_TYPEINFERED && !_types_are_identical(name_type, expr_type))
    {
        _error_incompatible_type(name->lineno, "Incompatible assignment", name_type, expr_type);
    } else if (expr_type->type == k_TYPEVOID) {
        fprintf(stderr, "Error: [line %d] void functions cannot be used in assignments\n", 
            name->lineno
        );
        exit(1);
    }

    if (name_type->type == k_TYPEINFERED)
    {
        name->identexpr.symbol->type = *expr_type;
    }
}

static void _typecheck_shortdeclstmt(const stmt_t* stmt)
{
    expr_t *names = stmt->shortdeclstmt.names;
    expr_t *exprs = stmt->shortdeclstmt.exprs;
    while (names != NULL && exprs != NULL)
    {
        _typecheck_shortdecl(names->seqexpr.main, exprs->seqexpr.main);
        names = names->seqexpr.next;
        exprs = exprs->seqexpr.next;
    }
}

static void _typecheck_assignstmt(const stmt_t* stmt)
{
    expr_t *names = stmt->assignstmt.names;
    expr_t *exprs = stmt->assignstmt.exprs;
    while (names != NULL && exprs != NULL)
    {
        expr_t* curr_name = names->seqexpr.main;
        expr_t* curr_expr = exprs->seqexpr.main;
        
        symboltype_t* curr_name_type = _typecheck_expr(curr_name);
        symboltype_t* curr_expr_type = _typecheck_expr(curr_expr);
        if(!_is_lvalue(curr_name)){
            fprintf(stderr, "Error: [line %d] Assignment expects lvalue on lhs.\n",
            curr_name->lineno
            );
            exit(1);
        }if (curr_expr_type->type == k_TYPEVOID) {
            fprintf(stderr, "Error: [line %d] void functions cannot be used in assignments\n", 
                stmt->lineno
            );
            exit(1);
        } else if(curr_name_type->type == k_TYPEINFERED){
            curr_name->resolved_type = curr_expr_type;
        } else if (!_types_are_identical(curr_name_type, curr_expr_type)){
            _error_incompatible_type(stmt->lineno, "Incompatible assignment", curr_name_type, curr_expr_type);
        } 

        names = names->seqexpr.next;
        exprs = exprs->seqexpr.next;
    }
}

static void _typecheck_opassignstmt(const stmt_t* stmt)
{
    symboltype_t* name_type = _typecheck_expr(stmt->opassignstmt.name);
    symboltype_t* expr_type = _typecheck_expr(stmt->opassignstmt.expr);
    if (!_types_are_identical(name_type, expr_type))
    {
        _error_incompatible_type(stmt->lineno, "Incompatible assignment", name_type, expr_type);
    }

    switch(stmt->opassignstmt.kind)
    {
        case k_ADDASSIGN:
            if ((!_is_numeric_type(name_type) || !_is_numeric_type(expr_type)) &&
                (resolve_type(name_type)->type != k_TYPECSTRING || resolve_type(expr_type)->type != k_TYPECSTRING))
            {
                fprintf(stderr, "Error: [line %d] Incompatible operand types in op assignment.\n",
                    stmt->lineno
                );
                exit(1);
            }
            break;
        case k_SUBASSIGN:
        case k_MULASSIGN:
        case k_DIVASSIGN:
            if (!_is_numeric_type(name_type) || !_is_numeric_type(expr_type))
            {
                fprintf(stderr, "Error: [line %d] Incompatible operand types in op assignment.\n",
                    stmt->lineno
                );
                exit(1);
            }
            break;
        case k_MODASSIGN:
        case k_BITWISEANDASSIGN:
        case k_BITWISEORASSIGN:
        case k_NOTASSIGN:
        case k_LSHIFTASSIGN:
        case k_RSHIFTASSIGN:
        case k_BITWISECLRASSIGN:
            if (!_is_integer_type(name_type) || !_is_integer_type(expr_type))
            {
                fprintf(stderr, "Error: [line %d] Incompatible operand types in op assignment.\n",
                    stmt->lineno
                );
                exit(1);
            }
            break;
        default: ;
    }
}

static void _typecheck_incdecstmt(expr_t* expr)
{
    symboltype_t *expr_type = _typecheck_expr(expr);

    if (!_is_lvalue(expr))
    {
        fprintf(stderr, "Error: [line %d] Unary increment/decrement expects lvalue.\n",
            expr->lineno
        );
        exit(1);
    }

    if (!_is_numeric_type(expr_type))
    {
        fprintf(stderr, "Error: [line %d] Unary increment/decrement statement on non-numeric type.\n ",
            expr->lineno
        );
        exit(1);
    }
}

static void _typecheck_returnstmt(const stmt_t* stmt)
{
    symboltype_t* expr_type = _typecheck_expr(stmt->returnstmt.expr);
    if (_context_return_type == NULL)
    {
        fprintf(stderr, "Error: [line %d] Return statement outside of a function definition.\n", stmt->lineno);
        exit(1);
    }
    else if (_context_return_type->type == k_TYPEVOID && stmt->returnstmt.expr != NULL)
    {
        fprintf(stderr, "Error: [line %d] Non-void return statement in void function.\n", stmt->lineno);
        exit(1);
    }
    else if (_context_return_type->type != k_TYPEVOID && stmt->returnstmt.expr == NULL)
    {
        fprintf(stderr, "Error: [line %d] Void return statement in non-void function.\n", stmt->lineno);
        exit(1);
    }
    else if(stmt->returnstmt.expr != NULL && !_types_are_identical(_context_return_type, expr_type))
    {
        _error_incompatible_type(stmt->lineno, "Incompatible return type", _context_return_type, expr_type);
    }
}

static void _typecheck_ifstmt(const stmt_t* stmt)
{
    _typecheck_stmt(stmt->ifstmt.init);
    symboltype_t* condition_type = _typecheck_expr(stmt->ifstmt.condition);
    if (resolve_type(condition_type)->type != k_TYPEBOOL)
    {
        fprintf(stderr, "Error: [line %d] Non-boolean condition in if statement.\n", stmt->lineno);
        exit(1);
    }

    _typecheck_stmt(stmt->ifstmt.thenblock);
    _typecheck_stmt(stmt->ifstmt.elseblock);
}

static void _typecheck_switch_cases(const stmt_t* stmt, const symboltype_t* condition_type)
{
    if (stmt == NULL) return;

    expr_t* case_expr = NULL;
    switch (stmt->kind)
    {
        case k_SEQSTMT:
            _typecheck_switch_cases(stmt->seqstmt.stmt, condition_type);
            _typecheck_switch_cases(stmt->seqstmt.next, condition_type);
            break;
        case k_SWITCHCASESTMT:
            case_expr = stmt->switchcasestmt.exprs;
            while(case_expr != NULL)
            {
                expr_t* expr = case_expr->seqexpr.main;
                symboltype_t* expr_type = _typecheck_expr(expr);
                if (condition_type == NULL)
                {
                    if (expr_type->type != k_TYPENOTBASE || expr_type->parent != basetype_bool)
                    {
                        _error_incompatible_type(stmt->lineno, "Incompatible case expression", &(basetype_bool->type), expr_type);
                    }
                } 
                else 
                {
                    if (!_types_are_identical(expr_type, condition_type))
                    {
                        _error_incompatible_type(stmt->lineno, "Incompatible case expression", condition_type, expr_type);
                    }
                }
                case_expr = case_expr->seqexpr.next;
            }
            _typecheck_stmt(stmt->switchcasestmt.stmts);
            break;
        default: ;
    }
}

static void _typecheck_switchstmt(const stmt_t* stmt)
{
    _typecheck_stmt(stmt->switchstmt.init);

    symboltype_t *condition_type = NULL;
    if (stmt->switchstmt.condition != NULL)
    {
        condition_type = _typecheck_expr(stmt->switchstmt.condition);
        if (!_is_comparable_type(resolve_type(condition_type)))
        {
            fprintf(stderr, "Error: [line %d] Non-comparable condition in switch statement.\n",
                stmt->lineno
            );
            exit(1);
        }
    }

    _typecheck_switch_cases(stmt->switchstmt.cases, condition_type);
}

static void _typecheck_forstmt(const stmt_t* stmt)
{
    _typecheck_stmt(stmt->forstmt.init);
    symboltype_t* condition_type = _typecheck_expr(stmt->forstmt.condition);
    _typecheck_stmt(stmt->forstmt.post);
    if (stmt->forstmt.condition != NULL && resolve_type(condition_type)->type != k_TYPEBOOL)
    {
        fprintf(stderr, "Error: [line %d] Non-boolean condition in for statement.\n", stmt->lineno);
        exit(1);
    }
    _typecheck_stmt(stmt->forstmt.loopblock);
}

static void _typecheck_stmt(stmt_t* stmt)
{
    if (stmt == NULL) return;
    switch(stmt->kind)
    {
        case k_EXPRSTMT:
            _typecheck_expr(stmt->exprstmt.expr);
            break;
        case k_BLOCKSTMT:
            _typecheck_stmt(stmt->blockstmt.stmts);
            break;
        case k_ASSIGNSTMT:
            _typecheck_assignstmt(stmt);
            break;
        case k_OPASSIGNSTMT:
            _typecheck_opassignstmt(stmt);
            break;
        case k_DECLSTMT:
            _typecheck_decl(&(stmt->declstmt));
            break;
        case k_SHORTDECLSTMT:
            _typecheck_shortdeclstmt(stmt);
            break;
        case k_INCREMENTSTMT:
            _typecheck_incdecstmt(stmt->incrementstmt.expr);
            break;
        case k_DECREMENTSTMT:
            _typecheck_incdecstmt(stmt->decrementstmt.expr);
            break;
        case k_RETURNSTMT:
            _typecheck_returnstmt(stmt);
            break;
        case k_IFSTMT:
            _typecheck_ifstmt(stmt);
            break;
        case k_SWITCHSTMT:
            _typecheck_switchstmt(stmt);
            break;
        case k_FORSTMT:
            _typecheck_forstmt(stmt);
            break;
        case k_SEQSTMT:
            _typecheck_stmt(stmt->seqstmt.stmt);
            _typecheck_stmt(stmt->seqstmt.next);
            break;
        default: ;
    }
}


/*************************************************************************************************/
/* Declaration typecheck functions                                                               */
/*************************************************************************************************/

static void _typecheck_decl(decl_t* decl)
{
    if (decl == NULL) return;

    expr_t* names = NULL;
    expr_t* exprs = NULL;
    switch(decl->kind)
    {
        case k_VARDECL:
            names = decl->vardecl.names;
            exprs = decl->vardecl.exprs;
            while(names != NULL && exprs != NULL)
            {
                _typecheck_vardecl(names->seqexpr.main, exprs->seqexpr.main);
                names = names->seqexpr.next;
                exprs = exprs->seqexpr.next;
            }
            break;
        case k_TYPEDECL:
            break;
        case k_FUNCDECL:
            _typecheck_funcdecl(decl);
            break;
        case k_SEQDECL:
            _typecheck_decl(decl->seqdecl.decl);
            _typecheck_decl(decl->seqdecl.next);
            break;
        default: ;
    }
}


/*************************************************************************************************/
/* Typecheck function                                                                            */
/*************************************************************************************************/

void typecheck(decl_t* root)
{
    _typecheck_decl(root);
}
