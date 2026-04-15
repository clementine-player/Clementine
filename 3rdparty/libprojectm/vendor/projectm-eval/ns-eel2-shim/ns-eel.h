/**
 * @file ns-eel2.h
 * @brief Nullsoft Expression Evaluator Library (NS-EEL) API Shim
 *
 * Contains declarations matching the original ns-eel2.h file to provide a compatible API.
 * See ReadMe.md in "ns-eel2-shim" for details.
 */
#pragma once

/* Includes and defines in case code including this header expects these. */
#include <stdlib.h>
#include <stdio.h>

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp _strnicmp
#endif

/* Must be defined to the same value as PRJM_F_SIZE */
#ifndef EEL_F_SIZE
#define EEL_F_SIZE 8
#endif

#if EEL_F_SIZE == 4
typedef float EEL_F;
#else
typedef double EEL_F;
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef void *NSEEL_VMCTX;
typedef void *NSEEL_CODEHANDLE;

void NSEEL_HOSTSTUB_EnterMutex();
void NSEEL_HOSTSTUB_LeaveMutex();

void NSEEL_quit();

NSEEL_VMCTX NSEEL_VM_alloc(); // return a context handle
void NSEEL_VM_free(NSEEL_VMCTX ctx); // free when done with a VM and ALL of its code have been freed, as well
void NSEEL_VM_resetvars(NSEEL_VMCTX ctx); // clears all vars to 0.0.
EEL_F *NSEEL_VM_regvar(NSEEL_VMCTX ctx, const char *name); // register a variable (before compilation)
void NSEEL_VM_freeRAM(NSEEL_VMCTX ctx); // clears and frees all (VM) RAM used
void NSEEL_VM_SetGRAM(NSEEL_VMCTX ctx, void **gram); // Same as in ns-eel2, MUST be called before compiling any code!
void NSEEL_VM_FreeGRAM(void **ufd); // frees a gmem context.
#ifdef __cplusplus
NSEEL_CODEHANDLE NSEEL_code_compile(NSEEL_VMCTX ctx, char *code, int lineoffs=0);
#else
NSEEL_CODEHANDLE NSEEL_code_compile(NSEEL_VMCTX ctx, char *code, int lineoffs);
#endif
char *NSEEL_code_getcodeerror(NSEEL_VMCTX ctx);
void NSEEL_code_execute(NSEEL_CODEHANDLE code);
void NSEEL_code_free(NSEEL_CODEHANDLE code);

#ifdef __cplusplus
}
#endif
