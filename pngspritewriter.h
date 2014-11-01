#ifndef PNGSPRITEWRITER_H_
#define PNGSPRITEWRITER_H_

#include "uoae.h"

int writeSpritesAsPngSheet(UOAE_UDWORD spriteWidth, UOAE_UDWORD spriteHeight,
		UOAE_UDWORD* spriteRgbaBuffers, int numSprites, const char* fileName);

int writeSinglePng(UOAE_UDWORD width, UOAE_UDWORD height,
		UOAE_UDWORD* rgbaBuffer, const char* fileName);

#endif
