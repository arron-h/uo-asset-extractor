#include <stdio.h>
#include <stdlib.h>

#include "uoae.h"
#include "dataindex.h"

int getNextIndex(FILE* fileHandle, IndexReference* ref)
{
	if (feof(fileHandle))
		return UOAE_ERROR;

	fread(&ref->offset, sizeof(UOAE_DWORD), 1, fileHandle);
	fread(&ref->size, sizeof(UOAE_DWORD), 1, fileHandle);
	fseek(fileHandle, sizeof(UOAE_DWORD), SEEK_CUR);

	return UOAE_OK;
}

