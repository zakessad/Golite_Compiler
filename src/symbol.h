#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define HASH_SIZE 321

#include "tree.h"
#include <stdbool.h>

#define MASKFUNCTION (1<<0)
#define MASKTYPE (1<<1)
#define MASKVARIABLE (1<<2)
#define MASKCONSTANT (1<<3)

struct symboltable;
struct symboltype;
struct symbol;
// Stores all the possible resolved types
// Need to "k_TYPENOTBASE" to know when recurse
typedef enum symboltypekind{
    k_TYPEINT,
    k_TYPEFLOAT64,
    k_TYPERUNE,
    k_TYPEBOOL,
    k_TYPECSTRING,
    k_TYPEVOID,
    k_TYPESTRUCT,
    k_TYPENOTBASE, //define when to recurse
    k_TYPEINFERED
}symboltypekind_t;

typedef enum symmbolkind{
    k_VARIABLE,
    k_FUNCTION,
    k_TYPE,
    k_CONSTANT
}symbolkind_t;


typedef struct symboltype{
    symboltypekind_t type;
    char* name;
    union{
        struct symbol* structsymbols;
        struct symbol* parent;
    };
    short isarray;
    arrayindex_t* dimensions;
    bool requiredslice;
}symboltype_t;

typedef struct typelist{
    symboltype_t* current;
    struct typelist* next;
}typelist_t;

typedef struct namelist{
    char* codegen_name;
    struct namelist* next;
}namelist_t;

typedef struct symbol{
    char* name;
    struct symboltype type;
    struct typelist* funcinfo;
    struct namelist* funcglobalvar;
    symbolkind_t kind;
    char* codegen_name;
    struct symbol* next;
}symbol_t;

typedef struct symboltable{
    symbol_t* table [HASH_SIZE];
    int scope;
    struct symboltable *parent;
} symboltable_t;

symboltable_t* make_table(decl_t* root);

void print_symtype(symboltype_t type, symbolkind_t symkind);
symbol_t* get_symbol(symboltable_t* table, char* name, bool recurse, int typemask);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif