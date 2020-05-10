#ifndef _PREPROC_CODEGEN_H_
#define _PREPROC_CODEGEN_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "symbol.h"
#include "tree.h"

void preproc(decl_t* decl);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif