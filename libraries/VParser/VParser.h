/*
   File:   Nordic.h
   Author: vincent

   Created on November 5, 2015, 9:19 AM
*/

#ifndef NORDIC_H
#define	NORDIC_H

#include <string.h>
#include <stdint.h>
#include "WString.h"

#define MAX_SIZE 150

#define COMBINE(sentence_type, term_number) (((unsigned)(sentence_type) << 5) | term_number)

enum {
  _SENTENCE_NONE, _SENTENCE_OTHER, _SENTENCE_LOC, _SENTENCE_HRM, _SENTENCE_CAD, _SENTENCE_ANCS, _SENTENCE_PC, _SENTENCE_QY, _SENTENCE_BTN, _SENTENCE_DBG
};

class VParser {
  public:

    VParser();
    VParser(const VParser& orig);
    virtual ~VParser();
    uint8_t encode(char c); // process one character received from GPS
    uint8_t encodeSentence(const char *sent_); // process one character received from GPS
    static String encodeOrder(float avance, float curTime);
    VParser &operator<<(char c) {
      encode(c);
      return *this;
    }

	uint8_t getPC() {
		return _pc;
	}
	uint8_t getBTN() {
		return _btn;
	}
	int32_t getLat() const {
      return _lat;
    }
	int32_t getLon() const {
      return _lon;
    }
	int32_t getGpsSpeed() const {
      return _gps_speed;
    }
	int32_t getEle() const {
      return _ele;
    }
	unsigned long getSecJ() const {
      return _sec_jour;
    }
    unsigned long getBPM() {
      return _bpm;
    }
    unsigned long getRR() {
      return _rr;
    }
    unsigned long getRPM() {
      return _rpm;
    }
    unsigned long getCadSpeed() {
      return _cad_speed;
    }
    unsigned long getANCS_type() {
      return _ancs_type;
    }
    const char *getANCS_msg() {
      return _ancs_msg.c_str();
    }
    const char *getANCS_title() {
      return _ancs_title.c_str();
    }
	
	unsigned long getDBG_type() {
      return _dbg_type;
    }
    unsigned long getDBG_code() {
      return _dbg_code;
    }
    unsigned long getDBG_line() {
      return _dbg_line;
    }
	const char *getDBG_msg() {
      return _dbg_file.c_str();
    }

	uint8_t _qy;
    String  _qy_msg;

  private:

    int nstrcmp(const char *str1, const char *str2);
    long natol(const char *str);
    bool isdigit(char c) {
      return c >= '0' && c <= '9';
    }
    int32_t parse_decimal();
	int32_t parse_sint();
    uint8_t term_complete();

	uint8_t _pc;

	uint8_t _btn;
    unsigned long _bpm, _rr;
    unsigned long _rpm;
    unsigned long _cad_speed;
	
	int32_t _lat;
	int32_t _lon;
	int32_t _ele;
	int32_t _gps_speed;
	unsigned long _sec_jour;
	
    unsigned long _ancs_type;
    String _ancs_msg;
    String _ancs_title;
	
	unsigned long _dbg_type, _dbg_line, _dbg_code;
	String _dbg_file;

    unsigned long _encoded_characters;

    // parsing state variables
    uint8_t _parity;
    bool _is_checksum_term;
    bool _started;
    char _term[MAX_SIZE];
    uint8_t _sentence_type;
    uint8_t _term_number;
    uint8_t _term_offset;
    bool _gps_data_good;
};

#endif	/* NORDIC_H */

