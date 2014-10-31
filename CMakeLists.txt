cmake_minimum_required (VERSION 2.8.0)
project (UOAssetExtractor)

set (SOURCE_FILES
	extractor.c
	anim.c
	dataindex.c)

set (HEADER_FILES
	anim.h
	uoae.h
	dataindex.h)

add_executable (UOAssetExtractor ${SOURCE_FILES} ${HEADER_FILES})
