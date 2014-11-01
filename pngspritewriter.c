#include <png.h>
#include <stdio.h>
#include <stdlib.h>

#include "pngspritewriter.h"

UOAE_UBYTE** allocateRowPointers(UOAE_UDWORD imageHeight, UOAE_UDWORD imageWidth,
		UOAE_UDWORD* rgbaBuffer);

int writeSpritesAsPngSheet(UOAE_UDWORD spriteWidth, UOAE_UDWORD spriteHeight,
		UOAE_UDWORD* spriteRgbaBuffers, int numSprites, const char* fileName)
{
	return UOAE_OK;
}

int writeSinglePng(UOAE_UDWORD width, UOAE_UDWORD height,
		UOAE_UDWORD* rgbaBuffer, const char* fileName)
{
	FILE* outputFile = fopen(fileName, "wb");
	if (!outputFile)
	{
		printf("Failed to open '%s' for writing.\n", fileName);
		return UOAE_ERROR;
	}

	UOAE_UBYTE** rowPointers = allocateRowPointers(height, width, rgbaBuffer);

	png_structp pngPtr;
	png_infop infoPtr;

	pngPtr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	infoPtr = png_create_info_struct(pngPtr);
	setjmp(png_jmpbuf(pngPtr));

	png_init_io(pngPtr, outputFile);

	// Write the header
	setjmp(png_jmpbuf(pngPtr));
	png_set_IHDR(pngPtr, infoPtr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(pngPtr, infoPtr);

	// Write the image data
	setjmp(png_jmpbuf(pngPtr));
	png_write_image(pngPtr, rowPointers);

	// Done
	setjmp(png_jmpbuf(pngPtr));
	png_write_end(pngPtr, NULL);

	free(rowPointers);
	fclose(outputFile);

	return UOAE_OK;
}

UOAE_UBYTE** allocateRowPointers(UOAE_UDWORD imageHeight, UOAE_UDWORD imageWidth,
		UOAE_UDWORD* rgbaBuffer)
{
	UOAE_UBYTE** rowPointers = (UOAE_UBYTE**)malloc(imageHeight * sizeof(UOAE_UBYTE*));

	for(UOAE_UDWORD row = 0; row < imageHeight; ++row)
	{
		rowPointers[row] = (UOAE_UBYTE*)&rgbaBuffer[row * imageWidth];
	}

	return rowPointers;
}

