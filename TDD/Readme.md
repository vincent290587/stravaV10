# Description

This is the test driven development folder.  
Basically with it you can compile the whole firmware under ubuntu and emulate all functionalities !  
Very useful for building new GUI features, or testing mathematical operations ;-)

## Use

It uses cmake to easily build the TDD firmware, to do that you need to run the script ccmake.sh in the folder.
Once the Makefile is created you can use only the make command

## Screen simulation

Starting the StravaV10 executable and then running the LS027simulator.jar allows you to see what will really be displayed on the device's screen  
(The option LS027_GUI needs to be enabled at compile time)  
