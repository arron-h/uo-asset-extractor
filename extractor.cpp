#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

typedef struct
{
	int offset;
	int size;
} IndexReference;

typedef struct
{
	UOAE_WORD centreX;
	UOAE_WORD centreY;
	UOAE_WORD width;
	UOAE_WORD height;
} BitmapDescriptor;

int getNextIndex(FILE* fileHandle, IndexReference* ref)
{
	if (feof(fileHandle))
		return UOAE_ERROR;

	fread(&ref->offset, sizeof(UOAE_DWORD), 1, fileHandle);
	fread(&ref->size, sizeof(UOAE_DWORD), 1, fileHandle);
	fseek(fileHandle, sizeof(UOAE_DWORD), SEEK_CUR);

	return UOAE_OK;
}

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

void writeDebugPpm(int imgIdx, const UOAE_UDWORD* rgbaBuffer, UOAE_WORD width, UOAE_WORD height)
{
    char outpathBuffer[256];
    sprintf(outpathBuffer, "/Users/arronhartley/tmp/%d.ppm", imgIdx);
    FILE* ppmFile = fopen(outpathBuffer, "wb");
    
    if (ppmFile)
    {
        fprintf(ppmFile, "P6\n%d %d\n255\n", (int)width, (int)height);
        
        const UOAE_UDWORD* bufPtr = rgbaBuffer;
        for (int i = 0; i < width * height; ++i)
        {
            UOAE_UBYTE rgb[3];
            rgb[0] = bufPtr[i] & 0xFF;  // R
            rgb[1] = (bufPtr[i] >> 8) & 0xFF;  // G
            rgb[2] = (bufPtr[i] >> 16) & 0xFF;  // B
            fwrite(rgb, sizeof(UOAE_UBYTE), 3, ppmFile);
            bufPtr++;
        }
        
        fclose(ppmFile);
    }

}

int extractFrameFromAnim(FILE* mulFile, const char* outputPath, const UOAE_UWORD* palette)
{
	BitmapDescriptor bitmapDescriptor;

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
    
#if 1
    static int debugImageCount = 0;
    writeDebugPpm(debugImageCount, rgbaBuffer, bitmapDescriptor.width, bitmapDescriptor.height);
    debugImageCount++;
#endif

	free(rgbaBuffer);

	return UOAE_OK;
}

int extractFromMul(FILE* mulFile, const char* outputPath, const IndexReference* ref)
{
	fseek(mulFile, ref->offset, SEEK_SET);
	
	UOAE_UWORD palette[256];
	UOAE_DWORD  frameCount   = 0;
	fread(palette,      sizeof(UOAE_WORD),  256, mulFile);
	fread(&frameCount,  sizeof(UOAE_DWORD), 1, mulFile);

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
	static const int PATH_BUFFER_LENGTH = 256;

	// Try load the index file
	if (strlen(assetPath) + 10 > PATH_BUFFER_LENGTH)
		return UOAE_ERROR;
	
	char pathBuffer[256];
	sprintf(pathBuffer, "%s/anim.idx", assetPath);
	FILE* indexFile = fopen(pathBuffer, "rb");
	if (!indexFile)
	{
		printf("ERROR: Failed to open %s\n", pathBuffer);
		return UOAE_ERROR;
	}

	sprintf(pathBuffer, "%s/anim.mul", assetPath);
	FILE* mulFile = fopen(pathBuffer, "rb");
	if (!mulFile)
	{
		printf("ERROR: Failed to open %s\n", pathBuffer);
		return UOAE_ERROR;
	}

	// Read an index
	IndexReference animIdx;
	int tempCount = 0;
	while(getNextIndex(indexFile, &animIdx) == 0 && tempCount < 10)
	{
		if (animIdx.offset == -1 || animIdx.size == -1)
			continue;

		UOAE_DEBUG_OUTPUT("Got index offset: %d size: %d", animIdx.offset, animIdx.size);
		extractFromMul(mulFile, outputPath, &animIdx);

		tempCount++;
	}

	fclose(indexFile);
	fclose(mulFile);

	return UOAE_OK;
}

int main(int numArgs, char** args)
{
	if (numArgs <= 3)
	{
		puts("Invalid number of arguments. Usage:");
		printf("\t%s [assetPath] [assetType (anim)] [outputPath]\n", args[0]);
		return 1;
	}

	const char* assetPath  = args[1];
	const char* assetType  = args[2];
	const char* outputPath = args[3];

	if (strcmp(assetType, "anim") == 0)
		return extractAnim(assetPath, outputPath);

	return 0;
}
