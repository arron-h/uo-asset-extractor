#include <string.h>
#include <stdlib.h>

#include "uoae.h"
#include "anim.h"
#include "dataindex.h"
#include "pngspritewriter.h"

typedef struct
{
	UOAE_WORD centreX;
	UOAE_WORD centreY;
	UOAE_WORD width;
	UOAE_WORD height;
} BitmapDescriptor;

int extractPixelChunkFromAnim(FILE* mulFile, const UOAE_UWORD* palette, int stride, UOAE_UDWORD* ptr)
{
	UOAE_DWORD chunkHeader;
	fread(&chunkHeader, sizeof(UOAE_DWORD), 1, mulFile);

	if (chunkHeader == 0x7FFF7FFF) // Completed marker
		return UOAE_READ_COMPLETE;

	chunkHeader ^= (0x200 << 22) | (0x200 << 12); // Remove the sign bit

	int xOffset    =  (chunkHeader >> 22) & 0x3FF;
	int yOffset    = ((chunkHeader >> 12) & 0x3FF) * stride;
	int pixelCount =   chunkHeader & 0xFFF;

	UOAE_UDWORD* curPtr = ptr + yOffset + xOffset;
	UOAE_UDWORD* endPtr = curPtr + pixelCount;

	while (!feof(mulFile) && curPtr < endPtr)
	{
		UOAE_UBYTE paletteIdx;
		fread(&paletteIdx, sizeof(UOAE_UBYTE), 1, mulFile);

		UOAE_UWORD argb1555 = palette[paletteIdx];

		unsigned int rgba8888 = ( ((( argb1555 >> 10) & 0x1F) * 0xFF / 0x1F) |
		                          ((((argb1555 >> 5) & 0x1F) * 0xFF / 0x1F) << 8) |
		                          ((( argb1555 & 0x1F) * 0xFF / 0x1F) << 16) |
		                          (   0xFF << 24) );

		*curPtr++ = rgba8888;
	}

	return UOAE_OK;
}

int extractFrameFromAnim(FILE* mulFile, const char* outputPath, const UOAE_UWORD* palette)
{
	BitmapDescriptor bitmapDescriptor;

	unsigned long frameNumber = ftell(mulFile);

	fread(&bitmapDescriptor.centreX, sizeof(UOAE_WORD), 1, mulFile);
	fread(&bitmapDescriptor.centreY, sizeof(UOAE_WORD), 1, mulFile);
	fread(&bitmapDescriptor.width,   sizeof(UOAE_WORD), 1, mulFile);
	fread(&bitmapDescriptor.height,  sizeof(UOAE_WORD), 1, mulFile);

	UOAE_DEBUG_OUTPUT("\t\tFrame x:%d y:%d w:%d h:%d", bitmapDescriptor.centreX,
		bitmapDescriptor.centreY, bitmapDescriptor.width, bitmapDescriptor.height);

	UOAE_UDWORD* rgbaBuffer = (UOAE_UDWORD*)malloc(bitmapDescriptor.width * bitmapDescriptor.height * sizeof(UOAE_UDWORD));
	memset(rgbaBuffer, 0, bitmapDescriptor.width * bitmapDescriptor.height * sizeof(UOAE_UDWORD));

	UOAE_UDWORD* ptr = rgbaBuffer;
	int stride = (bitmapDescriptor.width * sizeof(UOAE_UDWORD)) >> 2;

	int xBase = bitmapDescriptor.centreX - 0x200;
	int yBase = (bitmapDescriptor.centreY + bitmapDescriptor.height) - 0x200;

	ptr += xBase;
	ptr += yBase * stride;

	while(!feof(mulFile) &&
		extractPixelChunkFromAnim(mulFile, palette, stride, ptr) != UOAE_READ_COMPLETE)
	{
	}

	// Write the sprite
	// TODO - probably want to write the sheet, not just this file.
	char* outputFilenameBuffer = (char*)malloc(strlen(outputPath) + 11 +
			strlen(".png"));
	sprintf(outputFilenameBuffer, "%s/%lu.png", outputPath, frameNumber);

	writeSinglePng(bitmapDescriptor.width, bitmapDescriptor.height, rgbaBuffer,
			outputFilenameBuffer);

	free(outputFilenameBuffer);
	free(rgbaBuffer);

	return UOAE_OK;
}

int extractFromMul(FILE* mulFile, const char* outputPath, const IndexReference* ref)
{
	fseek(mulFile, ref->offset, SEEK_SET);

	UOAE_UWORD palette[256];
	UOAE_DWORD  frameCount   = 0;
	fread(palette,     sizeof(UOAE_WORD),  256, mulFile);
	fread(&frameCount, sizeof(UOAE_DWORD), 1, mulFile);

	UOAE_DWORD* frameOffsets = (UOAE_DWORD*)malloc(frameCount * sizeof(UOAE_DWORD));
	fread(frameOffsets, sizeof(UOAE_DWORD), frameCount, mulFile);

	UOAE_DEBUG_OUTPUT("\t%d frames in animation", frameCount);

	int frameIdx = 0;
	while(!feof(mulFile) && frameIdx < frameCount)
	{
		int frameStartOffset = ref->offset + sizeof(UOAE_WORD) * 256 + frameOffsets[frameIdx];
		fseek(mulFile, frameStartOffset, SEEK_SET);

		UOAE_DEBUG_OUTPUT("\t\tFrame %d is %d bytes offset", frameIdx, frameOffsets[frameIdx]);
		extractFrameFromAnim(mulFile, outputPath, palette);

		frameIdx++;
	}

	free(frameOffsets);

	return UOAE_OK;
}

int extractAnim(const char* assetPath, const char* outputPath)
{
	// Try load the index file
	int filenameBufferSize = strlen(assetPath) + strlen("/anim.xxx") + 1;
	char* filenameBuffer = (char*)malloc(filenameBufferSize);

	sprintf(filenameBuffer, "%s/anim.idx", assetPath);
	FILE* indexFile = fopen(filenameBuffer, "rb");
	if (!indexFile)
	{
		printf("ERROR: Failed to open %s\n", filenameBuffer);
	}

	sprintf(filenameBuffer, "%s/anim.mul", assetPath);
	FILE* mulFile = fopen(filenameBuffer, "rb");
	if (!mulFile)
	{
		printf("ERROR: Failed to open %s\n", filenameBuffer);
	}

	if (indexFile && mulFile)
	{
		// Read an index
		IndexReference animIdx;
		int tempCount = 0;
		while(getNextIndex(indexFile, &animIdx) == 0 && tempCount < 200)
		{
			if (animIdx.offset == -1 || animIdx.size == -1)
				continue;

			UOAE_DEBUG_OUTPUT("Got index offset: %d size: %d", animIdx.offset, animIdx.size);
			extractFromMul(mulFile, outputPath, &animIdx);

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

