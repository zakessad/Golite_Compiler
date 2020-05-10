#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "symbol.h"

#define MALLOC(type) (type*) malloc(sizeof(type))
#define INDENT(count) do { printf("%*s", (4 * (count)), ""); } while (0)

static symboltable_t* _init_sym_table();
static symboltable_t* _scope_sym_table(symboltable_t* table);
static symboltable_t* _unscope_sym_table(symboltable_t* table);
static symbol_t* _put_symbol(symboltable_t* table, symbol_t* symbol);
static uint32_t _get_hash(char* name);
static int _get_mask(symbolkind_t kind);
static char* _get_next_blankid_name();

static symbol_t* _make_base_type(char* name, symboltypekind_t type);
static symbol_t* _make_constant(char* name, symbol_t* parent);
static symbol_t* _make_linked_type(char* name, symbol_t* parent_type); 
static symbol_t* _make_struct_type(symboltable_t* table,char* name, decl_t* structdef); 

static void _print_symbol(symbol_t* sym);
static void _print_symtype(symboltype_t type, symbolkind_t symkind, bool stop);
static void _print_array(arrayindex_t* dimension);
static void _print_struct(symbol_t* structelements);
static void _print_funcargs(typelist_t* funcinfo);

static void _make_symtab_stmt(symboltable_t* table, stmt_t* stmt);
static void _make_symtab_ifstmt(symboltable_t* table, struct ifstmt ifstmt);
static void _make_symtab_forstmt(symboltable_t* table, struct forstmt forstmt);
static void _make_symtab_casestmt(symboltable_t* table, struct switchcasestmt switchcasestmt);
static void _make_symtab_switchstmt(symboltable_t* table, struct switchstmt switchstmt);
static void _make_symtab_shortdecl(symboltable_t* table, stmt_t* decl);

static void _make_symtab_decl(symboltable_t* table, decl_t* decl);
static symbol_t* _make_symtab_vardecl(symboltable_t* table, struct vardecl decl);
static void _make_symtab_funcdecl(symboltable_t* table, struct funcdecl* decl, int lineno);
static void _make_symtab_typedecl(symboltable_t* table, struct typedecl* decl);
static symbol_t* _append_symbol(int lineno, symbol_t* list, symbol_t* new_symbol);

static bool _find_type_struct(symbol_t* table, char* type_name);
static void _recursive_type_error(int lineno, char* type);
static bool _contains_slice(symbol_t* sym);

static void _make_symtab_expr(symboltable_t* table, expr_t* expr);
static symbol_t* _funccall_symbol(symboltable_t* table, expr_t* expr);

static symboltype_t* _convert_type(symboltable_t* table, type_t* type);
static void _special_func_check(int lineno, char* name);
static void _append_globalvar(char* codegen_name);

extern int print_symbols;

int indentation = 0;
int print_newline = 1;
int print_struct = 0;
int expr_stmt_scope = 0;
int scope_count = 0;

// Codegen vars
int init_count = 0;
int contain_main = 0;
int blank_id_count = 0;
symbol_t* current_func_sym = NULL;


symbol_t* basetype_bool;
symbol_t* basetype_int;
symbol_t* basetype_float64;
symbol_t* basetype_rune;
symbol_t* basetype_string;

symboltable_t* make_table(decl_t* root)
{
    symboltable_t* table = _scope_sym_table(NULL);

    symbol_t* base = NULL;
    
    // Add basic types and constants
    base = basetype_int = _make_base_type("int", k_TYPEINT);
    _put_symbol(table, base);
    base = basetype_float64 = _make_base_type("float64", k_TYPEFLOAT64);
    _put_symbol(table, base);
    base = basetype_rune = _make_base_type("rune", k_TYPERUNE);
    _put_symbol(table, base);
    base = basetype_string = _make_base_type("string", k_TYPECSTRING);
    _put_symbol(table, base);
    base = basetype_bool = _make_base_type("bool", k_TYPEBOOL);
    _put_symbol(table, base);
    // bool constants
    symbol_t* cst = _make_constant("true", basetype_bool);
    _put_symbol(table, cst);
    free(cst->codegen_name);
    cst->codegen_name = "True";

    cst = _make_constant("false", basetype_bool);
    _put_symbol(table, cst);
    free(cst->codegen_name);
    cst->codegen_name = "False";
    // scope again
    table = _scope_sym_table(table);
    _make_symtab_decl(table, root);
    table = _unscope_sym_table(table);
    if (print_symbols)
    {
        printf("}\n");
    }
    return table;
}

static symboltable_t* _init_sym_table()
{
    symboltable_t* sym_table = MALLOC(symboltable_t);
    for(int i =0; i<HASH_SIZE; i++)
    {
        sym_table->table[i] = NULL;
    }
    sym_table->parent = NULL;
    return sym_table;
}

static symboltable_t* _scope_sym_table(symboltable_t* parent)
{
    if(print_symbols)
    {
        INDENT(indentation);
        indentation++;
        printf("{\n");
    }
    symboltable_t* new_tab = _init_sym_table();
    new_tab->parent = parent;
    new_tab->scope = scope_count;
    scope_count++;
    return new_tab;
}

static symboltable_t* _unscope_sym_table(symboltable_t* table)
{
    if(print_symbols)
    {
        indentation--;
        INDENT(indentation);
        printf("}\n");
    }
    return table->parent;
}

static symbol_t* _put_symbol (symboltable_t* sym_tab, symbol_t* symbol)
{
    if(symbol->name != NULL && strcmp(symbol->name, "_") == 0){
        symbol->codegen_name = _get_next_blankid_name();
        return symbol;
    }

    uint32_t hash_code = _get_hash(symbol->name);
    for(symbol_t* sym = sym_tab->table[hash_code]; sym!=NULL; sym = sym->next)
    {
        if(strcmp(sym->name, symbol->name)==0)
        {
            fprintf(stderr, "Error: symbol %s is already defined in the same scope\n", symbol->name);
            exit(1);
        }
    }
    symbol->next = sym_tab->table[hash_code];
    sym_tab->table[hash_code] = symbol;
    if(print_symbols){
        _print_symbol(symbol);
    }
    // Add codegen names to the variable
    // Length of name is: length of symbol name + 9 char for the scope number + 8 for "__scope_"
    char* codegen_name = (char*) malloc(sizeof(char)*(strlen(symbol->name)+17));
    sprintf(codegen_name,"__scope%d_%s", sym_tab->scope, symbol->name);
    symbol->codegen_name = codegen_name;
    symbol->type.requiredslice = false;
    return symbol;
}

symbol_t* get_symbol(symboltable_t* sym_tab, char* name, bool recurse, int typemask)
{
    uint32_t hash_code = _get_hash(name);

    for(symbol_t* sym = sym_tab->table[hash_code]; sym!=NULL; sym = sym->next)
    {
        // check for name and type
        if(strcmp(sym->name, name)==0 && (_get_mask(sym->kind) & typemask))
        {
            // Add the global variable to function's global var list
            if(sym->kind == k_VARIABLE && sym_tab->scope == 1 &&
                current_func_sym != NULL)
            {
                _append_globalvar(sym->codegen_name);
            }
            return sym;
        }
    }

    if(sym_tab->parent == NULL || recurse == false) return NULL;
    return get_symbol(sym_tab->parent, name, recurse, typemask);
}

static int _get_mask(symbolkind_t kind)
{
    switch(kind)
    {
        case k_VARIABLE: return MASKVARIABLE;
        case k_FUNCTION: return MASKFUNCTION;
        case k_TYPE: return MASKTYPE;
        case k_CONSTANT: return MASKCONSTANT;
        default:
            fprintf(stderr, "Error: unknown symbol kind");
            exit(1);
    }
    return 0;
}

static uint32_t _get_hash(char* name)
{
    uint32_t hash = 0;
    while(*name)
    {
        hash = (hash << 1) + *name++;
    }
    return (hash%HASH_SIZE);
}

static symbol_t* _make_base_type(char* name, symboltypekind_t type)
{
    symbol_t* new_sym = MALLOC(symbol_t);
    new_sym->name = name;
    new_sym->kind = k_TYPE;
    new_sym->type.name = name;
    new_sym->type.type = type;
    new_sym->type.isarray = 0;
    new_sym->type.dimensions = NULL;
    return new_sym;
}

static symbol_t* _make_constant(char* name, symbol_t* parent)
{
    symbol_t* new_sym = MALLOC(symbol_t);
    new_sym->name = name;
    new_sym->kind = k_CONSTANT;
    new_sym->type.name = parent->name;
    new_sym->type.parent = parent;
    new_sym->type.type = k_TYPENOTBASE;
    new_sym->type.isarray = 0;
    new_sym->type.dimensions = NULL;
    return new_sym;
}

static symbol_t* _make_linked_type(char* name, symbol_t* parent_type)
{
    symbol_t* new_sym = MALLOC(symbol_t);
    new_sym->name = name;
    new_sym->kind = k_TYPE;
    new_sym->type.name = parent_type->name;
    new_sym->type.type = k_TYPENOTBASE;
    new_sym->type.parent = parent_type;
    new_sym->type.isarray = 0;
    new_sym->type.dimensions = NULL;
    return new_sym;
}

static char* _get_next_blankid_name()
{
    char* codegen_name = (char*) malloc(sizeof(char)*(17));
    sprintf(codegen_name,"__blankid_%d", blank_id_count);
    blank_id_count++;
    return codegen_name;
}

static void _print_symbol(symbol_t* sym)
{
    if(sym == NULL)
        return;
    INDENT(indentation);
    printf("%s ", sym->name);

    bool stop = false; // controls recursion of the type
    switch(sym->kind)
    {
        case k_VARIABLE:
            stop = true;
            if(!print_struct) //eliminate useless info inside strucs
                printf("[variable] = ");
            break;
        case k_FUNCTION:
            printf("[function] = ");
            if(strcmp(sym->name, "init") == 0){
                printf("<unmapped> \n");
                return;
            }
            _print_funcargs(sym->funcinfo);
            stop = true;
            break;
        case k_TYPE:
            printf("[type] = ");
            if( sym->type.type == k_TYPENOTBASE || sym->type.type == k_TYPESTRUCT)
                printf("%s", sym->name);
            break;
        case k_CONSTANT:
            printf("[constant] = ");
            break;
        default:
            fprintf(stderr, "Error: Undefined symbol type. \n");
            exit(1);
    }
    _print_symtype(sym->type, sym->kind, stop);
    fflush(stdout);
}

static void _print_funcargs(typelist_t* funcinfo)
{
    printf("(");
    typelist_t* lst = funcinfo;
    print_newline = 0;
    while(lst!=NULL){
        _print_symtype(*(lst->current), k_FUNCTION, true);
        lst = lst->next;
        if(lst != NULL){
            printf(", ");
        }
    }
    printf(") -> ");
    print_newline = 1;
}

void _print_symtype(symboltype_t type, symbolkind_t symkind, bool stop)
{
    if(!print_symbols)
        return;

    switch(type.type)
    {
        case k_TYPEINT:
            printf("int");
            break;
        case k_TYPEFLOAT64:
            printf("float64");
            break;
        case k_TYPERUNE:
            printf("rune");
            break;
        case k_TYPEBOOL:
            printf("bool");
            break;
        case k_TYPECSTRING:
            printf("string");
            break;
        case k_TYPEVOID:
            printf("void");
            break;
        case k_TYPESTRUCT:
            if(symkind == k_TYPE){
                printf(" -> ");
            }
            if(type.isarray) _print_array(type.dimensions);
            printf("struct { ");
            _print_struct(type.structsymbols);
            printf(" }");
            break;
        case k_TYPENOTBASE:
            if(symkind == k_TYPE){
                printf(" -> ");
            }
            if(type.isarray) _print_array(type.dimensions);
            
            if(stop){
                printf("%s", type.name);
                break;
            } 
            
            printf("%s", type.parent->name);
            // Stop when hitting an array
            if(type.isarray){
                break;
            }
            // Continue printing the types
            if(type.parent->type.type == k_TYPENOTBASE || type.parent->type.type == k_TYPESTRUCT){
                _print_symtype(type.parent->type, k_TYPE, false);
                return;
            }
            break;
        case k_TYPEINFERED:
            printf("<infered>");
            break;
        default:
            fprintf(stderr, "Error: Unknown type kind \n");
            exit(1);        
    }
    if(print_newline)
        printf("\n");
}

static void _print_array(arrayindex_t* dimension)
{
    while (dimension != NULL)
    {
        if (dimension->index != -1) printf("[%d]", dimension->index);
        else printf("[]"); 
        dimension = dimension->next;
    }
}

static void _print_struct(symbol_t* structelements)
{
    int enable_newline = print_newline;
    int disable_printstruct = !print_struct;
    int old_indentation = indentation;
    print_newline = 0;
    indentation = 0;
    print_struct = 1;
    while(structelements != NULL){
        _print_symbol(structelements);
        printf("; ");
        structelements = structelements->next;
    }
    indentation = old_indentation;
    if(enable_newline)
        print_newline = 1;
    if(disable_printstruct)
        print_struct = 0;
}

static void _make_symtab_stmt(symboltable_t* table, stmt_t* stmt)
{
    if(stmt == NULL)
        return;
    switch(stmt->kind)
    {
        case k_SEQSTMT:
            _make_symtab_stmt(table, stmt->seqstmt.stmt);
            _make_symtab_stmt(table, stmt->seqstmt.next);
            break;
        case k_EXPRSTMT:
            expr_stmt_scope = 1;
            _make_symtab_expr(table, stmt->exprstmt.expr);
            expr_stmt_scope = 0;
            break;
        case k_BLOCKSTMT:
            table = _scope_sym_table(table);
            _make_symtab_stmt(table, stmt->blockstmt.stmts);
            table = _unscope_sym_table(table);
            break;
        case k_ASSIGNSTMT:
            _make_symtab_expr(table, stmt->assignstmt.names);
            _make_symtab_expr(table, stmt->assignstmt.exprs);
            break;
        case k_OPASSIGNSTMT:
            _make_symtab_expr(table, stmt->opassignstmt.name);
            _make_symtab_expr(table, stmt->opassignstmt.expr);
            break;
        case k_DECLSTMT:
            _make_symtab_decl(table, &(stmt->declstmt));
            break;
        case k_SHORTDECLSTMT:
            _make_symtab_shortdecl(table,stmt);
            break;
        case k_IFSTMT:
            _make_symtab_ifstmt(table, stmt->ifstmt);
            break;
        case k_SWITCHSTMT:
            _make_symtab_switchstmt(table, stmt->switchstmt);
            break;
        case k_SWITCHCASESTMT:
            _make_symtab_casestmt(table, stmt->switchcasestmt);
            break;
        case k_FORSTMT:
            _make_symtab_forstmt(table, stmt->forstmt);
            break;
        case k_INCREMENTSTMT:
            _make_symtab_expr(table, stmt->incrementstmt.expr);
            break;
        case k_DECREMENTSTMT:
            _make_symtab_expr(table, stmt->decrementstmt.expr);
            break;
        case k_RETURNSTMT:
            _make_symtab_expr(table, stmt->returnstmt.expr);
            break;
        case k_BREAKSTMT:
        case k_CONTINUESTMT:
        case k_EMPTYSTMT:
            break;
        default:
            fprintf(stderr, "Error: Unknown statement type\n.");
            exit(1);
    }
}

static void _check_duplicate_ident(char* name, expr_t* rest)
{
    while(rest!= NULL)
    {
        char* next_name = rest->seqexpr.main->identexpr.value;
        if(strcmp(name, next_name) == 0){
            fprintf(stderr,"Error: [line %d] Repeated identifier %s on lhs of a short declaration.\n" ,rest->lineno, name);
            exit(1);
        } 
        rest =rest->seqexpr.next;
    }
}

static void _make_symtab_shortdecl(symboltable_t* table, stmt_t* decl)
{
    struct shortdeclstmt stmt = decl->shortdeclstmt;

    bool undeclared_var = false;

    expr_t* name_seq = stmt.names;
    expr_t* expr_seq = stmt.exprs;
    // Check that at least one element was not declared
    while(name_seq != NULL)
    {
        expr_t* name = name_seq->seqexpr.main;
        expr_t* expr = NULL;
        if(expr_seq != NULL)
            expr = expr_seq->seqexpr.main;
        _check_duplicate_ident(name->identexpr.value, name_seq->seqexpr.next);
        symbol_t* sym = get_symbol(table, name->identexpr.value, false, 0b1111);
        // Store individual expressions in an expr sequence
        expr_t* new_name = make_seqexpr(decl->lineno, name, NULL);
        expr_t* new_expr = make_seqexpr(decl->lineno, expr, NULL);
            
        if(sym == NULL){
            // Blank ids are not considered "new variables"
            if(strcmp(name->identexpr.value, "_") != 0)
                undeclared_var = true;
            struct vardecl var = {k_IDENTVARDECL, new_name, new_expr, NULL};
            _make_symtab_vardecl(table, var);
        } else {
            if(sym->kind != k_VARIABLE){
                fprintf(stderr, "Error: [line %d] %s is not a variable. \n", decl->lineno, sym->name);
                exit(1);
            }
            stmt_t* assignment = make_assignstmt(decl->lineno, new_name, new_expr);
            _make_symtab_stmt(table, assignment);
        }

        _make_symtab_expr(table, expr);
        name_seq = name_seq->seqexpr.next;
        if(expr_seq != NULL)
            expr_seq = expr_seq->seqexpr.next;
    }

    if (undeclared_var == false){
        fprintf(stderr, "Error: [line %d] expected at least one undeclared variable in short declaration \n", decl->lineno);
        exit(1);
    }
    
}

static void _make_symtab_switchstmt(symboltable_t* table, struct switchstmt switchstmt)
{
    table = _scope_sym_table(table);
    _make_symtab_stmt(table, switchstmt.init);
    _make_symtab_expr(table, switchstmt.condition);
    _make_symtab_stmt(table, switchstmt.cases);
    table = _unscope_sym_table(table);
}

static void _make_symtab_casestmt(symboltable_t* table, struct switchcasestmt switchcasestmt)
{
    table = _scope_sym_table(table);
    _make_symtab_expr(table, switchcasestmt.exprs);
    _make_symtab_stmt(table, switchcasestmt.stmts);
    table = _unscope_sym_table(table);
}

static void _make_symtab_forstmt(symboltable_t* table, struct forstmt forstmt)
{
    table = _scope_sym_table(table);
    _make_symtab_stmt(table, forstmt.init);
    _make_symtab_expr(table, forstmt.condition);
    _make_symtab_stmt(table, forstmt.post);
    table = _scope_sym_table(table);
    _make_symtab_stmt(table, forstmt.loopblock);
    table = _unscope_sym_table(table);
    table = _unscope_sym_table(table);
}

static void _make_symtab_ifstmt(symboltable_t* table, struct ifstmt ifstmt)
{
    table = _scope_sym_table(table);
    _make_symtab_stmt(table, ifstmt.init);
    _make_symtab_expr(table, ifstmt.condition);
    table = _scope_sym_table(table);
    _make_symtab_stmt(table, ifstmt.thenblock);
    table = _unscope_sym_table(table);
    if(ifstmt.elseblock != NULL)
    {
        table = _scope_sym_table(table);
        _make_symtab_stmt(table, ifstmt.elseblock);
        table = _unscope_sym_table(table);
    }
    table = _unscope_sym_table(table);
}


static void _make_symtab_decl(symboltable_t* table, decl_t* decl)
{
    if(decl == NULL)
        return;
    switch(decl->kind)
    {
        case k_PKGDECL: // nothing to do
            return;
        case k_VARDECL:
            _make_symtab_vardecl(table, decl->vardecl);
            return;
        case k_TYPEDECL:
            _make_symtab_typedecl(table, &(decl->typedecl));
            return;
        case k_FUNCDECL:
            _make_symtab_funcdecl(table, &(decl->funcdecl), decl->lineno);
            return;
        case k_SEQDECL:
            _make_symtab_decl(table, decl->seqdecl.decl);
            _make_symtab_decl(table, decl->seqdecl.next);
            return;
        default:    
            fprintf(stderr, "Error: Unknown declaration type\n.");
            exit(1);        
    }
}

static symbol_t* _make_symtab_vardecl(symboltable_t* table, struct vardecl decl)
{
    type_t* decl_type = decl.type;
    symbol_t* new_sym = NULL;
    symbol_t* sym = NULL;

    expr_t* names = decl.names;
    expr_t* exprs = decl.exprs;

    symbol_t* symbol_list = NULL;
    // TODO: this is ineficient, it should be change to only fetch type once.
    // Here assume ballance between lhs and rhs, weeder check
    while(names != NULL)
    {
        expr_t* name = names->seqexpr.main;
        if(exprs != NULL){
            expr_t* expr = exprs->seqexpr.main;
            // Make sure the expression is defined
            _make_symtab_expr(table, expr);
        }
        _special_func_check(name->lineno, name->identexpr.value);
        if(decl_type != NULL){
            switch(decl_type->kind)
            {
                case k_SIMPLETYPE:
                    sym = get_symbol(table, decl_type->name, true, 0b1111);
                    if(sym == NULL){
                        fprintf(stderr, "Error: line[%d] Undefined type %s. \n", decl_type->lineno, decl_type->name);
                        exit(1);
                    }
                    if(sym->kind != k_TYPE){
                        fprintf(stderr, "Error: [line %d] %s is not a type . \n", decl_type->lineno, decl_type->name);
                        exit(1);
                    }
                    decl_type->symbol = sym;
                    new_sym = _make_linked_type(name->identexpr.value, sym);
                    
                    break;
                case k_STRUCTTYPE:
                    new_sym = _make_struct_type(table, name->identexpr.value, decl_type->structdef);
                    break;
                default:
                    fprintf(stderr, "Error: Unknown declaration type.\n");
                    exit(1);
            }

            new_sym->type.isarray = decl_type->array_type;
            new_sym->type.dimensions = decl_type->dimensions;
            
        } else {
            new_sym = MALLOC(symbol_t);
            new_sym->name = name->identexpr.value;
            new_sym->type.type = k_TYPEINFERED;
        }
        new_sym->kind = k_VARIABLE;
      
        if(decl.kind == k_STRUCTVARDECL){
            symbol_list = _append_symbol(name->lineno, symbol_list, new_sym);
        } else {
            _put_symbol(table, new_sym);
        }
            

        name->identexpr.symbol = new_sym;

        names = names->seqexpr.next;

        if(exprs != NULL)
            exprs = exprs->seqexpr.next;
        
    }

    return symbol_list;
}

static symbol_t* _append_symbol(int lineno, symbol_t* list, symbol_t* new_symbol)
{
    if(list == NULL )
        return new_symbol;
    
    symbol_t* list_head = list;
    while(list != new_symbol){
        if(strcmp(new_symbol->name, "_") != 0 && strcmp(list->name, new_symbol->name) == 0){
            fprintf(stderr, "Error: [line %d] Redeclared symbol %s | %s\n", lineno, list->name, new_symbol->name);
            exit(1);
        }
        if(list->next == NULL)
            list->next = new_symbol;
        list = list->next;        
    }
    return list_head;
}

static void _recursive_type_error(int lineno, char* type)
{
    fprintf(stderr, "Error: [line %d] invalid recursive type %s.\n", lineno, type);
    exit(1);
}

static bool _contains_slice(symbol_t* sym)
{
    if(sym->type.isarray)
    {
        arrayindex_t* dim = sym->type.dimensions;
        while(dim != NULL){
            if(dim->index == -1){
                return true;
            }
            dim = dim->next;
        }
    }
    return false;
}

static bool _find_type_struct(symbol_t* struct_symbols, char* type_name)
{
    while(struct_symbols != NULL){

        if(struct_symbols->type.type == k_TYPESTRUCT){
            if(_find_type_struct(struct_symbols->type.structsymbols, type_name))
                return true;
        } else {
            char* name = struct_symbols->type.name;
            if(!_contains_slice(struct_symbols) && strcmp(type_name, name)==0)
                return true;           
        }
        struct_symbols = struct_symbols->next;
    }
    return false;
}

static void _make_symtab_typedecl(symboltable_t* table, struct typedecl* decl)
{
    type_t* decl_type = decl->type;
    symbol_t* sym = NULL;
    symbol_t* new_type = MALLOC(symbol_t);
    int print = print_symbols;
    print_symbols = 0;
    _special_func_check(decl_type->lineno, decl->name);
    
    new_type->name = decl->name;
    new_type->kind = k_TYPE;
    new_type->funcinfo = NULL;
    new_type->type.isarray = decl_type->array_type;
    new_type->type.dimensions = decl_type->dimensions;

    _put_symbol(table, new_type);
    switch(decl_type->kind)
    {
        case k_SIMPLETYPE:
            if(!_contains_slice(new_type) && strcmp(decl->name, decl_type->name) == 0)
                _recursive_type_error(decl_type->lineno, decl->name);
            sym = get_symbol(table, decl_type->name, true, MASKTYPE);
            if(sym == NULL){
                fprintf(stderr, "Error: [line %d] Unknown type %s.\n",decl_type->lineno, decl->name);
                exit(1);
            }
            new_type->type.parent = sym;
            new_type->type.type = k_TYPENOTBASE;
            new_type->type.name = decl->name;
            
            break;
        case k_STRUCTTYPE:
            sym = _make_struct_type(table, decl->name,decl_type->structdef);
            new_type->type.type = k_TYPESTRUCT;
            new_type->type.structsymbols = sym->type.structsymbols;
            
            if(_find_type_struct(new_type->type.structsymbols, decl->name))
                _recursive_type_error(decl_type->lineno, decl->name);
            break;
        default:
            fprintf(stderr, "Error: Unknown declaration type.\n");
            exit(1);
    }

    decl->symbol = new_type;
    print_symbols = print;
    if(print_symbols )
        _print_symbol(new_type);
    
}

static symbol_t* _make_struct_type(symboltable_t* table, char* name, decl_t* structdef)
{
    int enable_print = print_symbols;
    print_symbols = 0;
    symbol_t* symbol = MALLOC(symbol_t);
    symbol->kind = k_TYPE;
    symbol->name = name;
    symbol->type.name = name;
    symbol->type.type = k_TYPESTRUCT;
    symbol->type.structsymbols = NULL;

    // Struct def is a bunch of variable declaration lists
    while(structdef != NULL){
        symbol_t* new_list = _make_symtab_vardecl(table, structdef->seqdecl.decl->vardecl);
        new_list->codegen_name = new_list->name;
        symbol->type.structsymbols = _append_symbol(structdef->lineno, symbol->type.structsymbols, new_list);
        structdef = structdef->seqdecl.next;
    }

    if(enable_print)
        print_symbols = 1;
    return symbol;
}

static void _check_valid_special_func(int lineno, struct funcdecl* decl)
{
    if(decl->args != NULL || decl->returntype != NULL){
        fprintf(stderr, "Error: [line %d] %s must have no parameters and no return type. \n", lineno, decl->name);
        exit(1);
    }
}

static void _make_symtab_funcdecl(symboltable_t* table, struct funcdecl* decl, int lineno)
{
    symbol_t* symbol = MALLOC(symbol_t);
    symbol->name = decl->name;
    symbol->kind = k_FUNCTION;
    // Set function context to allow global var processing
    symbol->funcglobalvar= NULL;
    current_func_sym = symbol;

    if(strcmp(decl->name , "init") == 0){
        _check_valid_special_func(lineno, decl);
        // codegen stuff
        char* codegen_name = (char*) malloc(sizeof(char)*(17));
        sprintf(codegen_name,"__init_%d", init_count);
        symbol->codegen_name = codegen_name;
    
        init_count++;    
    }else if (strcmp(decl->name , "main") == 0){
        _check_valid_special_func(lineno, decl);
        contain_main = 1;
    }
    if(strcmp(decl->name, "init") != 0){
        int enable_print = print_symbols;
        print_symbols = 0;
        _put_symbol(table, symbol);
        print_symbols = enable_print;
    } 

    // Get the types for the arguments 
    decl_t* args = decl->args;
    symbol->funcinfo = NULL;
    
    symbol_t* arg_list = NULL; 
    
    if(args != NULL){
        symbol->funcinfo = MALLOC(typelist_t);
        arg_list = MALLOC(symbol_t);
        typelist_t* lst = symbol->funcinfo;
        symbol_t* arg_list_tail = arg_list;
        lst->next = NULL;
        arg_list_tail->next = NULL;
        while(args != NULL){
            // arg is a declaration
            decl_t* arg = args->seqdecl.decl;
            symboltype_t* arg_type = _convert_type(table, arg->vardecl.type);
            
            expr_t* id_seq = arg->vardecl.names;
            while(id_seq != NULL){
                // creates the new symbol for the variable
                arg_list_tail->type = *arg_type; 
                arg_list_tail->name = id_seq->seqexpr.main->identexpr.value;
                arg_list_tail->kind = k_VARIABLE;
                // Set the type in the type list
                lst->current = &(arg_list_tail->type);
                // link the symbol with the expression 
                id_seq->seqexpr.main->identexpr.symbol = arg_list_tail;
                // iterate
                id_seq = id_seq->seqexpr.next;
                if(id_seq!= NULL){
                    lst->next =MALLOC(typelist_t);
                    lst = lst->next;
                    lst->next = NULL;

                    arg_list_tail->next = MALLOC(symbol_t);
                    arg_list_tail = arg_list_tail->next;
                    arg_list_tail->next = NULL;
                }
            }
            args = args->seqdecl.next;
            if(args!= NULL){
                    lst->next =MALLOC(typelist_t);
                    lst = lst->next;
                    lst->next = NULL;

                    arg_list_tail->next = MALLOC(symbol_t);
                    arg_list_tail = arg_list_tail->next;
                    arg_list_tail->next = NULL;
            }
        }
    }

    // Get the return type
    if(decl->returntype == NULL){
        symbol->type.type = k_TYPEVOID;
        symbol->type.name = "void";
    }else {
        symbol->type = *(_convert_type(table, decl->returntype));
        
    }    
    decl->symbol = symbol;
    
    if(strcmp(decl->name, "init") != 0 && print_symbols){
        _print_symbol(symbol);
    }
    table = _scope_sym_table(table);
    
    // Push the symbol variables 
    while(arg_list != NULL){
        symbol_t* temp = arg_list;
        arg_list = arg_list->next;
        _put_symbol(table, temp);
    }
    // Check the statements 
    _make_symtab_stmt(table, decl->stmts);
    table = _unscope_sym_table(table);
    // Remove function context
    current_func_sym = NULL;
}

static void _make_symtab_expr(symboltable_t* table, expr_t* expr)
{
    if(expr == NULL)
        return;
    symbol_t* sym = NULL;
    char* name = NULL;
    switch(expr->kind)
    {
        case k_LITERALEXPR:
            return;
        case k_IDENTEXPR:
            name = expr->identexpr.value;
            sym = get_symbol(table, name, true, MASKVARIABLE | MASKCONSTANT);
            if(strcmp(name , "_") == 0){
                expr->identexpr.symbol = _make_base_type("_", k_TYPEINFERED);
                expr->identexpr.symbol->kind = k_VARIABLE;
                expr->identexpr.symbol->codegen_name = _get_next_blankid_name();
            } else if(sym == NULL && strcmp(name , "_")!= 0){
                fprintf(stderr, "Error: [line %d] undeclared indentifier %s. \n",expr->lineno, name);
                exit(1);
            } else {
                expr->identexpr.symbol = sym;
            }
            return;
        case k_UNARYEXPR:
            _make_symtab_expr(table, expr->unaryexpr.expr);
            return;
        case k_BINARYEXPR:
            _make_symtab_expr(table, expr->binaryexpr.l_expr);
            _make_symtab_expr(table, expr->binaryexpr.r_expr);
            return;
        case k_FUNCCALLEXPR:
            expr->funccallexpr.symbol = _funccall_symbol(table, expr->funccallexpr.name);
            _make_symtab_expr(table, expr->funccallexpr.args);
            return;
        case k_BUILTINCALLEXPR:
            expr_stmt_scope = 0; // doesnt apply for builtins
            _make_symtab_expr(table, expr->funccallexpr.args);
            return;
        case k_INDEXINGEXPR:
            _make_symtab_expr(table, expr->indexingexpr.array);
            _make_symtab_expr(table, expr->indexingexpr.index);
            return;
        case k_FIELDSELECTOREXPR:
            // only check receiver, selector will be checked in typechecking
            _make_symtab_expr(table, expr->fieldselectorexpr.receiver);
            return;
        case k_SEQEXPR:
            _make_symtab_expr(table, expr->seqexpr.main);
            _make_symtab_expr(table, expr->seqexpr.next);
            return;
        case k_PARENTHESESEXPR:
            _make_symtab_expr(table, expr->parenthesesexpr.main);
            return;
        default:
            fprintf(stderr, "Error: Unknown expression type\n.");
            exit(1);   
    }
}

static symbol_t* _funccall_symbol(symboltable_t* table, expr_t* expr)
{
    while(expr->kind == k_PARENTHESESEXPR){
        expr = expr->parenthesesexpr.main;
    }
    if(expr->kind == k_IDENTEXPR){
        char* name = expr->identexpr.value;
        symbol_t* sym = get_symbol(table, name, true, 0b1111);
        if(sym == NULL){
            fprintf(stderr, "Error: [line %d] undeclared type or function %s. \n",expr->lineno, name);
            exit(1);
        }
        if(sym->kind != k_TYPE && sym->kind != k_FUNCTION){
            fprintf(stderr, "Error: [line %d] %s is not a type or function. \n",expr->lineno, name);
            exit(1);
        }
        if(sym->kind == k_TYPE && expr_stmt_scope){
            fprintf(stderr, "Error: [line %d] %s is not a function. \n",expr->lineno, name);
            exit(1);
        }
        expr_stmt_scope = 0; // can only be used once
        expr->identexpr.symbol = sym;
        return sym;
    } else {
        fprintf(stderr, "Error: [line %d] Functions or types need to be identifiers. \n",expr->lineno);
        exit(1);
    }

}

static symboltype_t* _convert_type(symboltable_t* table, type_t* type)
{
    symboltype_t* new_type = MALLOC(symboltype_t);
    new_type->isarray = type->array_type;
    new_type->dimensions = type->dimensions;
    symbol_t* sym = NULL;
    switch(type->kind)
    {
        case k_SIMPLETYPE:
            sym = get_symbol(table, type->name, true, 0b1111);
            if(sym == NULL){
                fprintf(stderr, "Error: [line %d] Unknown type %s\n", type->lineno, type->name);
                exit(1);
            }
            if(sym->kind != k_TYPE){
                fprintf(stderr, "Error: [line %d] %s is not a type.\n", type->lineno, type->name);
                exit(1);
            }            
            new_type->name = sym->name;
            new_type->type = k_TYPENOTBASE;
            new_type->parent = sym;
            break;
        case k_STRUCTTYPE:
            sym = _make_struct_type(table, NULL, type->structdef);
            new_type->structsymbols = sym->type.structsymbols;
            new_type->type = k_TYPESTRUCT;
            break;
        default:
            fprintf(stderr, "Error: Unknown type declaration type\n.");
            exit(1);           
    }
    return new_type;
}

static void _special_func_check(int lineno, char* name)
{
    if(strcmp(name, "main") == 0 || strcmp(name, "init") == 0){
        fprintf(stderr,"Error: [line %d] %s expected to be a function.\n", lineno,name);
        exit(1);
    }
}

static void _append_globalvar(char* codegen_name)
{
    // Init
    if(current_func_sym->funcglobalvar == NULL){
        current_func_sym->funcglobalvar = MALLOC(namelist_t);
        current_func_sym->funcglobalvar->codegen_name = codegen_name;
        current_func_sym->funcglobalvar->next = NULL;
        return;
    }
    namelist_t* current = current_func_sym->funcglobalvar;
    while(1){
        // already in the list, we could actually just compare the addresses.
        if(strcmp(current->codegen_name, codegen_name) == 0){
            return;
        }
        // save the tail;
        if(current->next == NULL)
            break;

        current = current->next;
    }

    // append the new name to the list
    current->next = MALLOC(namelist_t);
    current = current->next;
    current->codegen_name = codegen_name;
    current->next = NULL;
}
