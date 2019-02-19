[![CircleCI](https://circleci.com/gh/vincent290587/stravaV10/tree/develop.svg?style=svg)](https://circleci.com/gh/vincent290587/stravaV10/tree/develop)

# Project

This application is an open source bicycle GPS.  

Included features: 
* Real-time compete with the internally stored Strava Segments (500+)
* Store a GPX file and follow it on-screen
* ANT+ connection to a heart rate monitors, any FE-C devices (smart home trainer), cadence & speed sensors...
* BLE connection to use the LNS, UART and Komoot (see https://www.komoot.com/)
* Estimates power using a precise barometer and special algorithms

Autonomy depends on the use: it uses <8mA in indoor mode (70+ hours), and around 35mA in outdoor modes (20+ hours).


## Hardware

This project uses Nordic nRF52840, a Cortex M4F MCU which can do BLE/ANT+ RF communications.  
The board can be found under: https://github.com/vincent290587/EAGLE/tree/master/Projects/myStravaB_V3  
The BOM costs around 100€


## Compilation & Programming

Must be compiled with GCC 6 2017-q2-update and nRF SDK V15.2 with softdevice s332 V5.0.0

cd pca10056/s332/armgcc  
make  
make flash_softdevice (or) make dfu_softdevice  
make flash (or) make dfu  


## Roadmap

- [x] Strava segments compete
- [x] GPX file following on a map
- [x] Better menu-system
- [x] Turn-by-turn navigation (using the Komoot app)
- [x] Better slope computation
- [ ] Connection to a BLE AP to unload recorded data
- [ ] More LED interaction


## Screenshots

![](docs/crs.png) ![](docs/crs_2seg.png) ![](docs/prc.png)






