#ifndef DATAINDEX_H_
#define DATAINDEX_H_

typedef struct
{
	int offset;
	int size;
} IndexReference;

int getNextIndex(FILE* fileHandle, IndexReference* ref);

#endif

