cmake_minimum_required (VERSION 3.5.1)

project (stravaV10 LANGUAGES C CXX)

add_definitions(-DTDD)
add_definitions(-D__HEAP_SIZE=150000)
add_definitions(-D_USE_MATH_DEFINES)
add_definitions(-DLOC_SOURCE_GPS)
#add_definitions(-DTDD_RANDOMIZE)
#add_definitions(-DLS027_GUI)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE)

  include(CodeCoverage.cmake)

  APPEND_COVERAGE_COMPILER_FLAGS()

endif(CODE_COVERAGE)

if(CMAKE_HOST_UNIX)
set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -Og -rdynamic -fsanitize=undefined -fsanitize=float-divide-by-zero -fno-sanitize-recover")
else()
set(CMAKE_C_FLAGS    "${CMAKE_CXX_FLAGS} -msse2 -mno-ms-bitfields")
add_definitions(-DWIN32)
add_definitions(-D_WIN32_WINNT=0x0600)
add_definitions(-DWINVER=0x600)
include_directories(./TDDW)
endif()


set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -ggdb -fno-omit-frame-pointer --short-enums -MMD -ffunction-sections -fdata-sections -fno-strict-aliasing -fsingle-precision-constant -frounding-math -fsignaling-nans")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=gnu++0x -fno-rtti -fpermissive -fexceptions")


# On inclut notre bibliotheque dans le processus de CMake
# add_subdirectory(source/routes)
add_subdirectory(TDD/sd/fatfs)


# add the binary tree to the search path for include files
include_directories(
    ./
	TDD
	TDD/sd
	TDD/sd/fatfs/src
	TDD/drivers
    TDD/logger
	TDD/timer
	TDD/sensors
	TDD/scheduler
	libraries/AdafruitGFX
    libraries/AdafruitGFX/Fonts
	libraries/AltiBaro
	libraries/GlobalTop
	libraries/utils
    libraries/filters
    libraries/komoot
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

# Cree des variables avec les fichiers a compiler
set(SRCS
    TDD/main_tdd.cpp
    TDD/GUI_connector.cpp
    TDD/scheduler/task_scheduler.c
    TDD/unit_testing.cpp
	TDD/Model_tdd.cpp
    TDD/Simulator.cpp
	TDD/millis_tdd.c
	TDD/order1_filter_tdd.c
	TDD/sd/sd_functions.cpp
	TDD/sd/sd_hal.c
	TDD/timer/timer.c
    TDD/sensors/GPSMGMT.cpp
    TDD/sensors/STC3100.cpp
    TDD/sensors/fxos.cpp
    TDD/sensors/bme280.c
	TDD/drivers/uart_tdd.c
	TDD/drivers/ant_tdd.c
	TDD/drivers/gpio_tdd.cpp
	TDD/drivers/ls027_tdd.cpp
	TDD/drivers/neopixel_tdd.c
    TDD/logger/tdd_logger.c
    segger_wrapper.c
    rf/ant_device_manager.cpp
	libraries/utils/WString.cpp
	libraries/AdafruitGFX/Adafruit_GFX.cpp
	libraries/AdafruitGFX/Print.cpp
	libraries/AltiBaro/AltiBaro.cpp
	libraries/utils/utils.c
    libraries/komoot/komoot_nav.c
	libraries/GlobalTop/EPONmeaPacket.cpp
	libraries/TinyGPSPlus/TinyGPS++.cpp
	source/display/SegmentManager.cpp
	source/display/Zoom.cpp
	source/display/notifications.c
    source/routes/ListePoints.cpp
	source/routes/Parcours.cpp
	source/routes/Points.cpp
	source/routes/Segment.cpp
	source/routes/Vecteur.cpp
    source/sensors/fram.c
	source/parsers/file_parser.cpp
	source/model/Attitude.cpp
	source/model/Boucle.cpp
	source/model/BoucleCRS.cpp
	source/model/BoucleFEC.cpp
	source/model/Locator.cpp
	source/model/PowerZone.cpp
	source/model/SufferScore.cpp
	source/model/UserSettings.cpp
	source/vue/Screenutils.cpp
	source/vue/Menuable.cpp
    source/vue/MenuObjects.cpp
	source/vue/Vue.cpp
	source/vue/VueCRS.cpp
	source/vue/VueDebug.cpp
	source/vue/VueGPS.cpp
	source/vue/VuePRC.cpp
	source/vue/VueFEC.cpp
    )

set(HEADERS
    main_tdd.h
    Simulator.h
    )

# add the executable
add_executable(StravaV10 ${SRCS})

# Et pour que l'executable fonctionne, il faut lui indiquer la bibliotheque dont il depend
target_link_libraries(StravaV10 fatfs)


if(CMAKE_HOST_UNIX)

  target_link_libraries(StravaV10 ubsan)

else()

  target_link_libraries(StravaV10 imagehlp)

endif()

if(CODE_COVERAGE)

SETUP_TARGET_FOR_COVERAGE_LCOV(
   NAME cov						    # Name for custom target.
   EXECUTABLE StravaV10				# Name of the test driver executable that runs the tests.
   # NOTE! This should always have a ZERO as exit code
   # otherwise the coverage generation will not complete.
   DEPENDENCIES StravaV10			# Name of output directory.
   )

SETUP_TARGET_FOR_COVERAGE_GCOVR_XML(
   NAME cov_xml						# Name for custom target.
   EXECUTABLE StravaV10				# Name of the test driver executable that runs the tests.
   # NOTE! This should always have a ZERO as exit code
   # otherwise the coverage generation will not complete.
   DEPENDENCIES StravaV10			# Name of output directory.
   )

endif(CODE_COVERAGE)


# Create the gcov-clean target. This cleans the build as well as generated
# .gcda and .gcno files.
add_custom_target(scrub
	COMMAND ${CMAKE_MAKE_PROGRAM} clean
	COMMAND rm -f ${OBJECT_DIR}/*.gcno
	COMMAND rm -f ${OBJECT_DIR}/*.gcda
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
