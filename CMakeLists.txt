cmake_minimum_required (VERSION 2.8.0)
project (UOAssetExtractor)

set (SOURCE_FILES
	extractor.c
	anim.c
	dataindex.c
	pngspritewriter.c)

set (HEADER_FILES
	anim.h
	uoae.h
	dataindex.h
	pngspritewriter.h)

find_package (PNG)

include_directories (${PNG_INCLUDE_DIRS})
add_definitions (${PNG_DEFINITIONS})

set (CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-std=c99")

add_executable (UOAssetExtractor ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries (UOAssetExtractor ${PNG_LIBRARIES})

