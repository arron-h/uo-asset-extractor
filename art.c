#include <string.h>
#include <stdlib.h>

#include "uoae.h"
#include "art.h"
#include "dataindex.h"
#include "pngspritewriter.h"

int art_processRowFromRawTile(FILE* mulFile, int numPixels, UOAE_UDWORD* rgbaBuffer,
		UOAE_UWORD* rowPixels, int row)
{
	fread(rowPixels, sizeof(UOAE_UWORD), numPixels, mulFile);

	for(int pixel = 0; pixel < numPixels; ++pixel)
	{
		UOAE_UWORD argb1555 = rowPixels[pixel];

		unsigned int rgba8888 = ( ((( argb1555 >> 10) & 0x1F) * 0xFF / 0x1F) |
								  ((((argb1555 >> 5)  & 0x1F) * 0xFF / 0x1F) << 8) |
								  ((( argb1555 & 0x1F) * 0xFF / 0x1F) << 16) |
								  (   0xFF << 24) );

		int x = (22-(numPixels/2)) + pixel;
		int offset = row * 44 + x;

		rgbaBuffer[offset] = rgba8888;
	}
}

int art_extractRawTile(FILE* mulFile, const char* outputPath)
{
	unsigned long tileNumber = ftell(mulFile);

	UOAE_UDWORD* rgbaBuffer = (UOAE_UDWORD*)malloc(44 * 44 * sizeof(UOAE_UDWORD));
	memset(rgbaBuffer, 0, 44 * 44 * sizeof(UOAE_UDWORD));

	UOAE_UWORD* rowPixels = (UOAE_UWORD*)malloc(44 * sizeof(UOAE_UWORD));

	// rows 0 - 21
	for(int i = 0; i < 22; ++i)
	{
		int numPixels = 2+i*2;
		art_processRowFromRawTile(mulFile, numPixels, rgbaBuffer, rowPixels, i);
	}

	// rows 22 - 43
	for(int i = 21; i >= 0; --i)
	{
		int numPixels = 2+i*2;
		art_processRowFromRawTile(mulFile, numPixels, rgbaBuffer, rowPixels,
				22+(22-(i+1)));
	}

	// Write the sprite
	// TODO - probably want to write the sheet, not just this file.
	char* outputFilenameBuffer = (char*)malloc(strlen(outputPath) + 11 +
			strlen(".png"));
	sprintf(outputFilenameBuffer, "%s/%lu.png", outputPath, tileNumber);

	writeSinglePng(44, 44, rgbaBuffer,
			outputFilenameBuffer);

	free(rowPixels);
	free(rgbaBuffer);
}

int art_extractFromMul(FILE* mulFile, const char* outputPath, const IndexReference* ref)
{
	fseek(mulFile, ref->offset, SEEK_SET);

	UOAE_DWORD flag;
	fread(&flag, sizeof(UOAE_DWORD), 1, mulFile);

	int extractionResult = UOAE_OK;
	if (flag > 0xFFFF || flag == 0)
	{
		extractionResult = art_extractRawTile(mulFile, outputPath);
	}
	else
	{
		// RLE tile
	}

	return extractionResult;
}

int extractArt(const char* assetPath, const char* outputPath)
{
	// Try load the index file
	int filenameBufferSize = strlen(assetPath) + strlen("/artidx.xxx") + 1;
	char* filenameBuffer = (char*)malloc(filenameBufferSize);

	sprintf(filenameBuffer, "%s/artidx.mul", assetPath);
	FILE* indexFile = fopen(filenameBuffer, "rb");
	if (!indexFile)
	{
		printf("ERROR: Failed to open %s\n", filenameBuffer);
	}

	sprintf(filenameBuffer, "%s/art.mul", assetPath);
	FILE* mulFile = fopen(filenameBuffer, "rb");
	if (!mulFile)
	{
		printf("ERROR: Failed to open %s\n", filenameBuffer);
	}

	if (indexFile && mulFile)
	{
		// Read an index
		IndexReference artIdx;
		int tempCount = 0;
		while(getNextIndex(indexFile, &artIdx) == 0 && tempCount < 200)
		{
			if (artIdx.offset == -1 || artIdx.size == -1)
				continue;

			UOAE_DEBUG_OUTPUT("Got index offset: %d size: %d", artIdx.offset, artIdx.size);
			art_extractFromMul(mulFile, outputPath, &artIdx);

			tempCount++;
		}
	}

	if (indexFile)
		fclose(indexFile);

	if (mulFile)
		fclose(mulFile);

	free(filenameBuffer);

	return UOAE_OK;
}
