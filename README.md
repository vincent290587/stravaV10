# Project

This application is an open source advanced bicycle GPS.  

Why is is advanced ?  
* When riding, it computes your advance with the stored strava segments in real time
* You can follow a GPX file stored in memory
* You can connect to a heart rate monitor using ANT+ or a FE-C device (smart home trainer)
* it uses sensors fusion (accelerometer magnetometer barometer) to estimate power etc...

Autonomy depends on the use: it consums 15 mA in smart trainer mode, and around 45mA in outdoor modes.


## Hardware

This project uses Nordic nRF52840, a Cortex M4F MCU which can do BLE/ANT+ RF communications.  
The board can be found under: https://github.com/vincent290587/EAGLE  
The BOM costs around 100€


## Compilation & Programming

Must be compiled with GCC 6 2017-q2-update  
You will need nRF SDK V15.2 and the softdevice s332 V5.0.0

cd pca10056/s332/armgcc  
make  
make flash_softdevice (or) make dfu_softdevice  
make flash (or) make dfu  


## Roadmap

- [x] Strava segments compete
- [x] GPX file following on a map
- [x] Connection to a BLE AP to unload recorded data
- [ ] Smarter interaction with the AP
- [ ] Sensors fusion (for slope compute)
- [ ] Better menu-system


## Screenshots




