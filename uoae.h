#ifndef UOAE_H_
#define UOAE_H_

#include <stdio.h>

#define UOAE_DEBUG_OUTPUT(x, ...) printf((x"\n"), __VA_ARGS__);

#define UOAE_OK    0
#define UOAE_ERROR 1
#define UOAE_READ_COMPLETE 2

#define UOAE_CHAR  char
#define UOAE_BYTE  signed char
#define UOAE_UBYTE unsigned char
#define UOAE_WORD  short
#define UOAE_UWORD unsigned short
#define UOAE_DWORD int
#define UOAE_UDWORD int

#endif

