# ble_ant_lns

Sample application using the ble_lns_c library on the nRF52832/nRF52840

## Environment

You will need softdevice s332 V5 with nRF SDK V14

## Compilation

cd pca100XX/s332/armgcc
make && make flash

## Use case

You can for exemple use the iPhone application "LE GPS" to send data to the chip (tested & works)

You could also send this data from another nRF chip using Nordic "experimental_ble_app_lns" exemple (not tested)


Have fun :-)
