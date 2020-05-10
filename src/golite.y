%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tree.h"

    #pragma GCC diagnostic ignored "-Wunused-function"

    int yylex();

    extern int yylineno;

    extern decl_t* root;
    
    void yyerror(const char *s) 
    {
        fprintf(stderr, "Error: [line %d] %s \n", yylineno, s);
        exit(1);
    }
%}

%union {
    int intval; 
    double floatval;
    char* identifier; 
    char* const_string;
    char* chrval;
    struct decl* decl;
    struct stmt* stmt;
    struct expr* expr;
    struct type* type;
    int opassignkind;
}

%type <decl>
             prog
             global_stmt
             global_stmts
             dcl
             pkg_dcl
             var_dcl
             var_dcl_spec
             var_dcl_specs
             type_dcl
             type_dcl_spec
             type_dcl_specs
             func_dcl
             func_arg
             func_args
%type <stmt> 
             break
             continue
             return
             if
             if_init
             elseif_else
             for
             switch
             case_list
             case
             stmts
             stmt
             simple_stmt
             block_stmt
             inc_dec
             short_dcl
             assignment
             op_assignment

%type <expr> expression
             expression_list
             literal
             identifier_list
             print
             println
             function_call
             builtin_call

%type <type> type
             type_or_struct
             struct

%type <opassignkind> assign_operator

/***** LITERAL TOKENS *****/
%token <intval> tINTVAL 
%token <chrval> tRUNEVAL 
%token <intval> tBOOLVAL 
%token <floatval> tFLOATVAL
%token <identifier> tIDENTIFIER
%token <const_string> tCSTRING

/***** KEYWORD TOKENS *****/
%token tBREAK
%token tCASE
%token tCHAN
%token tCONST
%token tCONTINUE
%token tDEFAULT
%token tDEFER
%token tELSE
%token tFALLTHROUGH
%token tFOR
%token tFUNC
%token tGO
%token tGOTO
%token tIF
%token tIMPORT
%token tINTERFACE
%token tMAP
%token tPACKAGE
%token tRANGE
%token tRETURN
%token tSELECT
%token tSTRUCT
%token tSWITCH
%token tTYPE
%token tVAR
%token tPRINT
%token tPRINTLN
%token tAPPEND
%token tLEN
%token tCAP

/***** TYPE TOKENS *****/
%token tINT
%token tFLOAT
%token tBOOL
%token tRUNE
%token tSTRING

/***** OPERATOR TOKENS *****/
%token tPLUS
%token tMINUS
%token tMUL
%token tDIV
%token tMOD
%token tLSHIFT
%token tRSHIFT
%token tPLUSEQUAL
%token tMINUSEQUAL
%token tMULEQUAL
%token tDIVEQUAL
%token tMODEQUAL
%token tANDEQUAL
%token tOREQUAL
%token tNOTEQUAL
%token tLSHIFTEQUAL
%token tRSHIFTEQUAL
%token tBOOLAND
%token tBOOLOR
%token tBACKARROW
%token tINCREMENT
%token tDECREMENT
%token tEQ
%token tNEQ
%token tGT
%token tGTE
%token tLT
%token tLTE
%token tSHORTDCL
%token tELLIPSIS
%token tBITCLEAR
%token tBITCLEAREQUAL
%token tUPLUS
%token tUMINUS
%token tBITNOT

/***** ASSOCIATIVITY AND PRECENDENCE *****/
%left ','
%left tBOOLOR
%left tBOOLAND
%left tEQ tNEQ tLT tLTE tGT tGTE
%left '+' '-' '|' '^'
%left '*' '/' '%' tLSHIFT tRSHIFT '&' tBITCLEAR
%left '!' tUPLUS tUMINUS tBITNOT
%left '.' '[' '('


%start prog

%locations

%define parse.error verbose

%%
prog: pkg_dcl global_stmts
    {
        int lineno = @1.first_line;
        root = make_seqdecl(lineno, $1, $2);
    }
    | pkg_dcl { root = $1; }
    ;

/***** GLOBAL STATEMENTS *****/
global_stmts: global_stmt global_stmts
            {
                int lineno = @1.first_line;
                $$ = make_seqdecl(lineno, $1, $2);
            }
            | global_stmt { $$ = $1; }
            ;

global_stmt: dcl { $$ = $1; }
           | func_dcl { $$ = $1; }
           ;

/***** GENERAL STATEMENTS *****/
stmts: stmt stmts
     {
        int lineno = @1.first_line;
        $$ = make_seqstmt(lineno, $1, $2);
     }
     | { $$ = NULL; }
     ;

stmt: dcl { $$ = make_declstmt(@1.first_line,$1); }
    | block_stmt { $$ = $1; }
    | short_dcl ';' { $$ = $1; }
    | assignment ';' { $$ = $1; }
    | op_assignment ';' { $$ = $1; }
    | inc_dec ';' { $$ = $1; }
    | return { $$ = $1; }
    | if { $$ = $1; }
    | if_init { $$ = $1; }
    | switch { $$ = $1; }
    | for { $$ = $1; }
    | break { $$ = $1; }
    | continue { $$ = $1; }
    | ';' 
    {
        int lineno = @1.first_line;
        $$ = make_emptystmt(lineno);
    }
    | function_call ';'
    {
        int lineno = @1.first_line;
        $$ = make_exprstmt(lineno, $1);
    }
    | print ';'
    {
        int lineno = @1.first_line;
        $$ = make_exprstmt(lineno, $1);
    }
    | println ';'
    {
        int lineno = @1.first_line;
        $$ = make_exprstmt(lineno, $1);
    }

    ;

simple_stmt: short_dcl { $$ = $1; }
           | expression { $$ = make_exprstmt(@1.first_line, $1); }
           | assignment { $$ = $1; }
           | op_assignment { $$ = $1; }
           | inc_dec { $$ = $1; }
           |  { $$ = NULL; }
           ;

block_stmt: '{' stmts '}'
            {
                int lineno = @1.first_line;
                $$ = make_blockstmt(lineno, $2);
            } 
          ;

/***** DECLARATIONS *****/
dcl: var_dcl { $$ = $1; }
   | type_dcl { $$ = $1; }
   ;

/*** PACKAGE DECLARATION ***/
pkg_dcl: tPACKAGE tIDENTIFIER ';'
       {
            int lineno = @1.first_line;
            $$ = make_pkgdecl(lineno, $2);
       }
       ;

/*** VARIABLE DECLARATION ***/
var_dcl: tVAR var_dcl_spec { $$ = $2; }
       | tVAR '(' var_dcl_specs ')' ';' { $$ = $3; }
       ;

var_dcl_specs: var_dcl_spec var_dcl_specs
             {
                int lineno = @1.first_line;
                $$ = make_seqdecl(lineno, $1, $2);
             }
             | { $$ = NULL; }
             ;

var_dcl_spec: identifier_list type_or_struct ';'
            {
                int lineno = @1.first_line;
                $$ = make_identvardecl(lineno, $1, NULL, $2);
            }
            | identifier_list '=' expression_list ';'
            {
                int lineno = @1.first_line;
                $$ = make_identvardecl(lineno, $1, $3, NULL);
            }
            | identifier_list type_or_struct '=' expression_list ';'
            {
                int lineno = @1.first_line;
                $$ = make_identvardecl(lineno, $1, $4, $2);
            }
            ;

/*** TYPE DECLARATION ***/
type_dcl: tTYPE type_dcl_spec { $$ = $2; }
        | tTYPE '(' type_dcl_specs ')' ';' { $$ = $3; }
        ;

type_dcl_specs: type_dcl_spec type_dcl_specs
              {
                  int lineno = @1.first_line;
                  $$ = make_seqdecl(lineno, $1, $2);
              }
              | { $$ = NULL; }
              ;

type_dcl_spec: tIDENTIFIER type_or_struct ';'
             {
                int lineno = @1.first_line;
                $$ = make_typedecl(lineno, $1, $2);
             }
             ;

/*** FUNCTION DECLARATION ***/
func_dcl: tFUNC tIDENTIFIER '(' func_args ')' '{' stmts '}' ';'
        {
            int lineno = @1.first_line;
            $$ = make_funcdecl(lineno, $2, NULL, $4, $7);
        }
        | tFUNC tIDENTIFIER '('  ')' '{' stmts '}' ';'
        {
            int lineno = @1.first_line;
            $$ = make_funcdecl(lineno, $2, NULL, NULL, $6);
        }
        | tFUNC tIDENTIFIER '(' func_args ')' type_or_struct '{' stmts '}' ';'
        {
            int lineno = @1.first_line;
            $$ = make_funcdecl(lineno, $2, $6, $4, $8);
        }
        | tFUNC tIDENTIFIER '('  ')' type_or_struct '{' stmts '}' ';'
        {
            int lineno = @1.first_line;
            $$ = make_funcdecl(lineno, $2, $5, NULL, $7);
        }
        ;

func_args: func_arg ',' func_args 
         { 
            int lineno = @1.first_line;
            $$ = make_seqdecl(lineno, $1, $3); 
         }
         | func_arg 
         {
            int lineno = @1.first_line;
            $$ = make_seqdecl(lineno, $1, NULL);
         }
         ;

func_arg: identifier_list type_or_struct
        {
            int lineno = @1.first_line;
            $$ = make_funcvardecl(lineno, $1, $2);
        }
        ;

/*** SHORT DECLARATION ***/
short_dcl: expression_list tSHORTDCL expression_list 
         {
            int lineno = @1.first_line;
            $$ = make_shortdeclstmt(lineno, $1, $3);
         }
         ;

/*** ASSIGNMENTS ***/
assignment: expression_list '=' expression_list  
          {
              int lineno = @1.first_line;
              $$ = make_assignstmt(lineno, $1, $3);
          }
          ;

op_assignment: expression assign_operator expression 
             {
                int lineno = @1.first_line;
                $$ = make_opassignstmt(lineno, $2, $1, $3);
             }
             ;

assign_operator: tPLUSEQUAL { $$ = k_ADDASSIGN; }
               | tMINUSEQUAL { $$ = k_SUBASSIGN; }
               | tMULEQUAL { $$ = k_MULASSIGN; }
               | tDIVEQUAL { $$ = k_DIVASSIGN; }
               | tMODEQUAL { $$ = k_MODASSIGN; }
               | tANDEQUAL { $$ = k_BITWISEANDASSIGN; }
               | tOREQUAL { $$ = k_BITWISEORASSIGN; }
               | tNOTEQUAL { $$ = k_NOTASSIGN; }
               | tLSHIFTEQUAL { $$ = k_LSHIFTASSIGN; }
               | tRSHIFTEQUAL { $$ = k_RSHIFTASSIGN; }
               | tBITCLEAREQUAL { $$ = k_BITWISECLRASSIGN; }
               ;

/***** GENERAL STATEMENTS *****/
inc_dec: expression tINCREMENT 
       {
            int lineno = @1.first_line;
            $$ = make_incrementstmt(lineno, $1);
       }
       | expression tDECREMENT
       {
            int lineno = @1.first_line;
            $$ = make_decrementstmt(lineno, $1);
       }
       ;

print: tPRINT '(' ')' 
     {
         int lineno = @1.first_line;
         expr_t* name = make_identexpr(lineno, "print");
         $$ = make_builtincallexpr(lineno, name, NULL);
     }
     | tPRINT '(' expression_list ')' 
     {
         int lineno = @1.first_line;
         expr_t* name = make_identexpr(lineno, "print");
         $$ = make_builtincallexpr(lineno, name, $3);
     }
     ;

println: tPRINTLN '(' ')' 
       {
           int lineno = @1.first_line;
           expr_t* name = make_identexpr(lineno, "println");
           $$ = make_builtincallexpr(lineno, name, NULL);
       }
       | tPRINTLN '(' expression_list ')' 
       {
           int lineno = @1.first_line;
           expr_t* name = make_identexpr(lineno, "println");
           $$ = make_builtincallexpr(lineno, name, $3);
       }
       ;

return: tRETURN ';'
      {
          int lineno = @1.first_line;
          $$ = make_returnstmt(lineno, NULL);
      }
      | tRETURN expression ';'
      {
          int lineno = @1.first_line;
          $$ = make_returnstmt(lineno, $2);
      }
      ;

/*** IF STATEMENT ***/
if: tIF expression '{' stmts '}' elseif_else 
  {
        int lineno = @1.first_line;
        $$ = make_ifstmt(lineno, NULL, $2, $4, $6);
  }
  ;

if_init: tIF simple_stmt ';' expression '{' stmts '}' elseif_else 
       {
           int lineno = @1.first_line;
           $$ = make_ifstmt(lineno, $2, $4, $6, $8);
       }
;

elseif_else: tELSE if { $$ = $2; }
           | tELSE if_init {$$ = $2;}
           | tELSE '{' stmts '}' ';' { $$ = $3; }
           | ';' { $$ = NULL; }
           ;
        
/*** SWITCH STATEMENT ***/
switch: tSWITCH expression '{' case_list '}' ';'
      {
          int lineno = @1.first_line;
          $$ = make_switchstmt(lineno, NULL, $2, $4);
      }
      | tSWITCH '{' case_list '}' ';'
      {
          int lineno = @1.first_line;
          $$ = make_switchstmt(lineno, NULL, NULL, $3);
      }
      | tSWITCH simple_stmt ';' expression '{' case_list '}' ';'
      {
          int lineno = @1.first_line;
          $$ = make_switchstmt(lineno, $2, $4, $6);
      }
      | tSWITCH simple_stmt ';'  '{' case_list '}' ';'
      {
          int lineno = @1.first_line;
          $$ = make_switchstmt(lineno, $2, NULL, $5);
      }
      ;

case_list: case case_list
         {
             int lineno = @1.first_line;
             $$ = make_seqstmt(lineno, $1, $2);
         }
         | { $$ = NULL; }
         ;

case: tCASE expression_list ':' stmts
    {
        int lineno = @1.first_line;
        $$ = make_switchcasestmt(lineno, $2, $4);
    }
    | tDEFAULT ':' stmts
    {
        int lineno = @1.first_line;
        $$ = make_switchcasestmt(lineno, NULL, $3);
    }
    ;

/*** FOR LOOP STATEMENTS ***/
for: tFOR '{' stmts '}' ';'
   {
        int lineno = @1.first_line;
        $$ = make_forstmt(lineno, NULL, NULL, NULL, $3);
   }
   | tFOR expression '{' stmts '}' ';'
   {
        int lineno = @1.first_line;
        $$ = make_forstmt(lineno, NULL, $2, NULL, $4);
   }
   | tFOR simple_stmt ';' expression ';' simple_stmt '{' stmts '}' ';'
   {
        int lineno = @1.first_line;
        $$ = make_forstmt(lineno, $2, $4, $6, $8);
   }
   | tFOR simple_stmt ';' ';' simple_stmt '{' stmts '}' ';'
   {
        int lineno = @1.first_line;
        $$ = make_forstmt(lineno, $2, NULL, $5, $7);
   }
   ;

break: tBREAK ';'  
     {
         int lineno = @1.first_line;
         $$ = make_breakstmt(lineno);
     }
     ;

continue: tCONTINUE ';'  
        {
            int lineno = @1.first_line;
            $$ = make_continuestmt(lineno);
        }
        ;

/***** EXPRESSIONS *****/
expression: '(' expression ')' 
          { 
              int lineno = @1.first_line;
              $$ = make_parenthesesexpr(lineno, $2); 
          }
          | literal { $$ = $1; }
          | tIDENTIFIER
          {
              int lineno = @1.first_line;
              $$ = make_identexpr(lineno, $1);
          }
          | function_call { $$ = $1; }
          | builtin_call { $$ = $1; }
          | '+' expression %prec tUPLUS
          {
              int lineno = @1.first_line;
              $$ = make_unaryexpr(lineno, k_POSUNARYEXPR, $2);
          }
          | '-' expression %prec tUMINUS
          {
              int lineno = @1.first_line;
              $$ = make_unaryexpr(lineno, k_NEGUNARYEXPR, $2);
          }
          | '!' expression
          {
              int lineno = @1.first_line;
              $$ = make_unaryexpr(lineno, k_NOTUNARYEXPR, $2);
          }
          | '^' expression %prec tBITNOT
          {
              int lineno = @1.first_line;
              $$ = make_unaryexpr(lineno, k_BITNOTUNARYEXPR, $2);
          }
          | expression '+' expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_ADDBINARYEXPR, $1, $3);
          }
          | expression '-' expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_SUBBINARYEXPR, $1, $3);
          }
          | expression '*' expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_MULBINARYEXPR, $1, $3);
          }
          | expression '/' expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_DIVBINARYEXPR, $1, $3);
          }
          | expression '%' expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_MODBINARYEXPR, $1, $3);
          }
          | expression tLSHIFT expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_LSHIFTBINARYEXPR, $1, $3);
          }
          | expression tRSHIFT expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_RSHIFTBINARYEXPR, $1, $3);
          }
          | expression '&' expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_BITWISEANDBINARYEXPR, $1, $3);
          }
          | expression '|' expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_BITWISEORBINARYEXPR, $1, $3);
          }
          | expression '^' expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_BITWISEXORBINARYEXPR, $1, $3);
          }
          | expression tGT expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_GTBINARYEXPR, $1, $3);
          }
          | expression tLT expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_LTBINARYEXPR, $1, $3);
          }
          | expression tGTE expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_GEBINARYEXPR, $1, $3);
          }
          | expression tLTE expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_LEBINARYEXPR, $1, $3);
          }
          | expression tEQ expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_EQBINARYEXPR, $1, $3);
          }
          | expression tNEQ expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_NEBINARYEXPR, $1, $3);
          }
          | expression tBOOLAND expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_LOGICALANDBINARYEXPR, $1, $3);
          }
          | expression tBOOLOR expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_LOGICALORBINARYEXPR, $1, $3);
          }
          | expression tBITCLEAR expression
          {
              int lineno = @1.first_line;
              $$ = make_binaryexpr(lineno, k_BITWISECLRBINARYEXPR, $1, $3);
          }
          | expression '[' expression ']'
          {
              int lineno = @1.first_line;
              $$ = make_indexingexpr(lineno, $1, $3);
          }
          | expression '['  ']'
          {
              int lineno = @1.first_line;
              $$ = make_indexingexpr(lineno, $1, NULL);
          }
          | expression '.' tIDENTIFIER
          {
              int lineno = @1.first_line;
              $$ = make_fieldselectorexpr(lineno, $1, make_identexpr(lineno, $3));
          }
          ;

function_call: expression '(' ')'
             {
                 int lineno = @1.first_line;
                 $$ = make_funccallexpr(lineno, $1, NULL);
             }
             | expression '(' expression_list ')'
             {
                 int lineno = @1.first_line;
                 $$ = make_funccallexpr(lineno, $1, $3);
             }
             ;

builtin_call: tAPPEND '(' expression_list ')'
            {
                int lineno = @1.first_line;
                expr_t* expr = make_identexpr(lineno, "append");
                $$ = make_builtincallexpr(lineno, expr, $3);
            }
            | tLEN '(' expression_list ')'
            {
                int lineno = @1.first_line;
                expr_t* expr = make_identexpr(lineno, "len");
                $$ = make_builtincallexpr(lineno, expr, $3);
            }
            | tCAP '(' expression_list ')'
            {
                int lineno = @1.first_line;
                expr_t* expr = make_identexpr(lineno, "cap");
                $$ = make_builtincallexpr(lineno, expr, $3);
            }
            ;

/***** HELPER GRAMMARS *****/
type: tIDENTIFIER
    {
        int lineno = @1.first_line;
        $$ = make_simpletype(lineno, $1);
    }
    | '(' type ')'
    {
        $$ = $2;
    }
    | '[' ']' type
    {
        $$ = add_arraydimension($3, -1); 
    }
    | '[' tINTVAL ']' type
    { 
        $$ = add_arraydimension($4, $2); 
    }
    ;

struct: tSTRUCT '{' var_dcl_specs '}'
      {
          int lineno = @1.first_line;
          mark_struct_vardecl($3);
          $$ = make_structtype(lineno, $3);
      }
      | '[' ']' struct
      {
          $$ = add_arraydimension($3, -1); 
      }
      | '[' tINTVAL ']' struct
      { 
          $$ = add_arraydimension($4, $2); 
      }
      ;

type_or_struct: type { $$ = $1; }
              | struct { $$ = $1; }
              ;

identifier_list: tIDENTIFIER ',' identifier_list
               {
                   int lineno = @1.first_line;
                   expr_t* id = make_identexpr(lineno, $1);
                   $$ = make_seqexpr(lineno, id, $3);
               }
               | tIDENTIFIER
               {
                   int lineno = @1.first_line;
                   expr_t* id = make_identexpr(lineno, $1);
                   $$ = make_seqexpr(lineno, id, NULL);
               }
               ;

expression_list: expression ',' expression_list
               {
                   int lineno = @1.first_line;
                   $$ = make_seqexpr(lineno, $1, $3);
               }
               | expression
               {
                   int lineno = @1.first_line;
                   $$ = make_seqexpr(lineno, $1, NULL);
               }
               ;

literal: tINTVAL
       {
           int lineno = @1.first_line;
           $$ = make_literalexpr_intval(lineno, $1);
       }
       | tFLOATVAL
       {
           int lineno = @1.first_line;
           $$ = make_literalexpr_fltval(lineno, $1);
       }
       | tRUNEVAL
       {
           int lineno = @1.first_line;
           $$ = make_literalexpr_chrval(lineno, $1);
       }
       | tBOOLVAL
       {
           int lineno = @1.first_line;
           $$ = make_literalexpr_intval(lineno, $1);
       }
       | tCSTRING
       {
           int lineno = @1.first_line;
           $$ = make_literalexpr_strval(lineno, $1);
       }
       ;

%%

