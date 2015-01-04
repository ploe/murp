#ifdef __cplusplus
extern "C" {
#endif

#ifndef MURP_PRIVATES
#define MURP_PRIVATES

/*	murp headers	*/

#include "./murp.h"
#include "./Lexer.h"
#include "./Grammar.h"

/*	std C headers	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*	private types	*/

enum {
	YES = -1,
	NO
};

#define _mp_SEND_ATOM NULL

#endif

#ifdef __cplusplus
}
#endif


