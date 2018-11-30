cmake_minimum_required (VERSION 3.5.1)

project (fatfs)

# Crée des variables avec les fichiers à compiler
set(SRCS
    src/ff.c
    )
	
set(HEADERS
    src/ff.h
    src/diskio.h
    )


# add the executable
add_library(fatfs ${SRCS} ${HEADERS})