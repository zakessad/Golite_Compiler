#ifndef TYPECHECK_H
#define TYPECHECK_H


#include "symbol.h"
#include "tree.h"


#ifdef __cplusplus
extern "C"
{
#endif


void typecheck(decl_t* root);


#ifdef __cplusplus
}
#endif


#endif /* TYPECHECK_H */
