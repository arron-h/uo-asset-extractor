#include <stdio.h>
#include <string.h>

#include "uoae.h"
#include "anim.h"

void printUsage(const char* binaryName)
{
	printf("Usage: %s [--help] ASSET_PATH ASSET_TYPE OUTPUT_PATH\n\n", binaryName);
	puts("Arguments:");
	puts("\t--help\t\t\tThis help (optional)");
	puts("\tASSET_PATH\t\tPath to the asset (probably the UO root folder)");
	puts("\tASSET_TYPE\t\tThe asset type to extract. Values:");
	puts("\t\t\t\t\tanim");
	puts("\tOUTPUT_PATH\t\tPath where the extracted assets will be dumped");
}

int main(int numArgs, char** args)
{
	if (numArgs > 1 && strcmp(args[1], "--help") == 0)
	{
		printUsage(args[0]);
		return 0;
	}

	if (numArgs <= 3)
	{
		puts("Invalid number of arguments.");
		printUsage(args[0]);

		return 1;
	}

	const char* assetPath  = args[1];
	const char* assetType  = args[2];
	const char* outputPath = args[3];

	if (strcmp(assetType, "anim") == 0)
		return extractAnim(assetPath, outputPath);

	return 0;
}

