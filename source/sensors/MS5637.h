/*
 * MS5637.h
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_MS5637_H_
#define LIBRARIES_MS5637_H_


/* Module supports a range of lower oversampling levels, for faster
   less accurate results.

   Default is maximum accuracy.
 */
enum BaroOversampleLevel {
  OSR_256, OSR_512, OSR_1024, OSR_2048, OSR_4096, OSR_8192 };

typedef enum {
  CELSIUS,
  FAHRENHEIT
} TempUnit;

/* error codes */
#define ERR_NOREPLY -1
#define ERR_BAD_READLEN -2
#define ERR_NEEDS_BEGIN -3

class MS5637 {
 public:
  MS5637();
  bool init();
  uint32_t reset();

  bool wireWriteByte(uint8_t val);
  int wireReadDataBlock(uint8_t reg, uint8_t *val, unsigned int len);

  /* Return temperature in C or Fahrenheit */
  float getTemperature(TempUnit scale = CELSIUS,
                       BaroOversampleLevel level = OSR_8192);
  /* Return pressure in mbar */
  float getPressure(BaroOversampleLevel level = OSR_8192);

  /* Update both temperature and pressure together. This takes less
     time than calling each function separately (as pressure result
     depends on temperature.) Returns true for success, false on an
     error */
  bool getTempAndPressure(float *temperature,
                          float *pressure,
                          TempUnit tempScale = CELSIUS,
                          BaroOversampleLevel level = OSR_8192);

  inline bool isOK() { return initialised && err == 0; }
  inline int8_t getError() { return initialised ? err : ERR_NEEDS_BEGIN; }

private:
  bool initialised;
  int8_t err;
  uint16_t c1,c2,c3,c4,c5,c6; // Calibration constants used in producing results

  uint32_t takeReading(uint8_t trigger_cmd, BaroOversampleLevel oversample_level);
};


#endif /* LIBRARIES_MS5637_H_ */
