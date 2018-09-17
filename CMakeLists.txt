cmake_minimum_required (VERSION 3.9.3)

project (stravaV10)

# On inclut notre bibliothèque dans le processus de CMake
# add_subdirectory(source/routes)
# add_subdirectory(libraries/utils)

# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

add_definitions(-DTDD -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti -fno-builtin --short-enums -nostdlib -MMD -nostdlib)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/TDD/main_tdd.h.in"
  "${PROJECT_BINARY_DIR}/main_tdd.h"
  )
 
# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_BINARY_DIR}/..")
include_directories("${PROJECT_BINARY_DIR}/../libraries/utils")
include_directories("${PROJECT_BINARY_DIR}/../sources/routes")

# Crée des variables avec les fichiers à compiler
set(SRCS
    main_tdd.cpp
	libraries/utils/WString.cpp
	libraries/utils/utils.c
    source/routes/ListePoints.cpp
	source/routes/Parcours.cpp
	source/routes/Points.cpp
	source/routes/Segment.cpp
	source/routes/Vecteur.cpp
    )
    
set(HEADERS
    main_tdd.h
	source/routes/Points.h
    )
 
# add the executable
add_executable(StravaV10 ${SRCS} ${HEADERS})

# Et pour que l'exécutable fonctionne, il faut lui indiquer la bibliothèque dont il dépend
# target_link_libraries(StravaV10 libs)
# target_link_libraries(StravaV10 routes)