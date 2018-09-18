cmake_minimum_required (VERSION 3.9.3)

project (stravaV10)

# On inclut notre bibliothèque dans le processus de CMake
# add_subdirectory(source/routes)
# add_subdirectory(libraries/utils)

# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

add_definitions(-DTDD)
add_definitions(-D__HEAP_SIZE=150000)
add_definitions(-D_USE_MATH_DEFINES)

set(CMAKE_C_FLAGS  "-fno-builtin --short-enums -nostdlib -MMD -nostdlib -ffunction-sections -fdata-sections -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS  "-std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti")


# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/TDD/main_tdd.h.in"
  "${PROJECT_BINARY_DIR}/main_tdd.h"
  )
 
# add the binary tree to the search path for include files

include_directories(
    ./
	TDD
	TDD/DB
	TDD/DB/fatfs/src
	TDD/drivers
	TDD/timer
	TDD/vue
	libraries/GlobalTop
	libraries/utils
	libraries/TinyGPSPlus
	rf
    source
    source/routes
	source/model
	source/vue
	source/parsers
	source/display
	source/sensors
    )

# Crée des variables avec les fichiers à compiler
set(SRCS
    main_tdd.cpp
	Model_tdd.cpp
	millis_tdd.c
	DB/sd_functions.cpp
	DB/sd_hal.c
	timer/timer.c
    sensors/GPSMGMT.cpp
	drivers/uart_tdd.c
	drivers/neopixel_tdd.c
	vue/Vue_TDD.cpp
	libraries/utils/WString.cpp
	libraries/utils/utils.c
	libraries/GlobalTop/MTK.cpp
	libraries/TinyGPSPlus/TinyGPS++.cpp
	source/display/SegmentManager.cpp
	source/display/notifications.c
    source/routes/ListePoints.cpp
	source/routes/Parcours.cpp
	source/routes/Points.cpp
	source/routes/Segment.cpp
	source/routes/Vecteur.cpp
	source/model/Attitude.cpp
	source/model/Boucle.cpp
	source/model/BoucleCRS.cpp
	source/model/BoucleFEC.cpp
	source/model/Locator.cpp
	source/vue/Screenutils.cpp
    )
    
set(HEADERS
    main_tdd.h
    )
 
# add the executable
add_executable(StravaV10 ${SRCS} ${HEADERS})

# Et pour que l'exécutable fonctionne, il faut lui indiquer la bibliothèque dont il dépend
# target_link_libraries(StravaV10 libs)
# target_link_libraries(StravaV10 routes)