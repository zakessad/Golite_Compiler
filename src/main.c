#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "weeder.h"
#include "pretty.h"

#include "weeder.h" 
#include "symbol.h"
#include "typecheck.h"
#include "codegen.h"
#include "preproc_codegen.h"

int print_tokens = 0;

int print_symbols = 0;


int yylex();

void yyparse();

decl_t* root;
symboltable_t* symtab;

void error() {
    fprintf(stderr,"Error: please enter one of the following option <tokens|scan|parse|symbol|typecheck|codegen> \n");
    exit(1);
}
int main(int argc, const char* argv[])
{ 
    if(argc < 2) {
        error();
    }else if(strcmp(argv[1], "tokens") == 0){
        print_tokens = 1;
        while(yylex());
        return 0;
    } else if (strcmp(argv[1] , "scan") == 0 ){
        while(yylex());
        printf("OK\n");
    } else if (strcmp(argv[1], "parse") == 0){
        yyparse();
        weed(root);
        printf("OK\n");
    } else if (strcmp(argv[1], "pretty") == 0){
        yyparse();
        weed(root);
        pretty(root);
    }else if (strcmp(argv[1], "symbol") == 0){
        print_symbols = 1;
        yyparse();
        weed(root);
        make_table(root);
    }else if (strcmp(argv[1], "typecheck") == 0){
        yyparse();
        weed(root);
        symtab = make_table(root);
        typecheck(root);
        printf("OK\n");
    }else if (strcmp(argv[1], "codegen") == 0){
        yyparse();
        weed(root);
        symtab = make_table(root);
        typecheck(root);
        preproc(root);
        codegen(root);
    }else{
        error();
    }

    return 0;

}
