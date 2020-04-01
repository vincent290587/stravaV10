/*
   File:   Nordic.cpp
   Author: vincent

   Created on November 5, 2015, 9:19 AM
*/

#include <math.h>
#include "VParser.h"

#define _LOC_TERM    "LOC"
#define _HRM_TERM    "HRM"
#define _CAD_TERM    "CAD"
#define _ANCS_TERM   "ANCS"
#define _PC_TERM     "DWN"
#define _QY_TERM     "QRY"
#define _BTN_TERM    "BTN"
#define _DBG_TERM    "DBG"

VParser::VParser() {
  _bpm = 0;
  _rr = 0;
  _rpm = 0;
  _ele = 0;
  _ancs_type = 0;
  _dbg_type = 0;
  _sentence_type = 0;
  _btn = 0;
  _gps_speed = 0;
  _lon = 0;
  _lat = 0;
  _pc = 0;
  _qy = 0;
  _qy_msg = "";
  _cad_speed = 0.;
  _sec_jour = 0;
  _dbg_code = 0;
  _dbg_line = 0;
  _gps_data_good = 0;
  _encoded_characters = 0;
  _is_checksum_term = false;
  _term_number = _term_offset = 0;
  _parity = 0;
  _started = false;
  memset(_term, 0, MAX_SIZE * sizeof(char));
}

VParser::VParser(const VParser& orig) : VParser() {
}

VParser::~VParser() {
}


//
// public methods
//

uint8_t VParser::encode(char c) {
  uint8_t valid_sentence = _SENTENCE_NONE;

  if (_started == true) ++_encoded_characters;
  switch (c) {
    case ',': // term terminators
      _parity ^= c;
      // no break;
    case '\r':
    case '\n':
    case '*':
      if (_started == true) {
        if (_term_offset < sizeof (_term) - 1) {
          _term[_term_offset] = 0;
          valid_sentence = term_complete();
        }
        ++_term_number;
      }
      _term_offset = 0;
      if (valid_sentence != _SENTENCE_NONE && valid_sentence != _SENTENCE_OTHER) {
        _started = false;
      }
      _is_checksum_term = false;
      return valid_sentence;

    case '$': // sentence begin
      memset(_term, 0, MAX_SIZE * sizeof(char));
      _term_number = _term_offset = 0;
      _parity = 0;
      _started = true;
      _sentence_type = _SENTENCE_OTHER;
      _is_checksum_term = false;
      return valid_sentence;
  }

  // ordinary characters
  if (_term_offset < sizeof (_term) - 1 && _started == true)
    _term[_term_offset++] = c;
  if (!_is_checksum_term)
    _parity ^= c;

  return valid_sentence;
}


uint8_t VParser::encodeSentence(const char *sent_) {
  uint16_t i;
  uint8_t ret_val = _SENTENCE_NONE;

  for (i = 0; i < strlen(sent_); i++) {
    ret_val = encode(sent_[i]);
  }

  return ret_val;
}

// Processes a just-completed term
// Returns true if new sentence is validated

uint8_t VParser::term_complete() {

  uint8_t ret_val = _SENTENCE_NONE;

  // the first term determines the sentence type
  if (_term_number == 0) {
    if (!nstrcmp(_term, _LOC_TERM))
      _sentence_type = _SENTENCE_LOC;
    else if (!nstrcmp(_term, _HRM_TERM))
      _sentence_type = _SENTENCE_HRM;
    else if (!nstrcmp(_term, _CAD_TERM))
      _sentence_type = _SENTENCE_CAD;
    else if (!nstrcmp(_term, _ANCS_TERM))
      _sentence_type = _SENTENCE_ANCS;
    else if (!nstrcmp(_term, _PC_TERM))
      _sentence_type = _SENTENCE_PC;
    else if (!nstrcmp(_term, _BTN_TERM))
      _sentence_type = _SENTENCE_BTN;
    else if (!nstrcmp(_term, _DBG_TERM))
      _sentence_type = _SENTENCE_DBG;
    else if (!nstrcmp(_term, _QY_TERM))
      _sentence_type = _SENTENCE_QY;
    else
      _sentence_type = _SENTENCE_OTHER;
    return false;
  }

  if (_sentence_type != _SENTENCE_OTHER && _term[0]) {
    // on doit parser l'info (_term)
    switch (COMBINE(_sentence_type, _term_number)) {
      case COMBINE(_SENTENCE_LOC, 1):
        _sec_jour = natol(_term);
        break;
	  case COMBINE(_SENTENCE_LOC, 2):
        _lat = parse_sint();
        break;
      case COMBINE(_SENTENCE_LOC, 3):
        _lon = parse_sint();
        break;
	  case COMBINE(_SENTENCE_LOC, 4):
        _ele = parse_sint();
        break;
	  case COMBINE(_SENTENCE_LOC, 5):
        _gps_speed = parse_sint();
        ret_val = _SENTENCE_LOC;
        break;

      case COMBINE(_SENTENCE_HRM, 1):
        _bpm = natol(_term);
        break;
      case COMBINE(_SENTENCE_HRM, 2):
        _rr = natol(_term);
        ret_val = _SENTENCE_HRM;
        break;

      case COMBINE(_SENTENCE_CAD, 1):
        _rpm = natol(_term);
        break;
      case COMBINE(_SENTENCE_CAD, 2):
        _cad_speed = natol(_term);
        ret_val = _SENTENCE_CAD;
        break;

      case COMBINE(_SENTENCE_ANCS, 1):
        _ancs_type = natol(_term);
        break;
      case COMBINE(_SENTENCE_ANCS, 2):
        if (_ancs_type == 0) {
          _ancs_msg = _term;
          ret_val = _SENTENCE_ANCS;
        } else {
          _ancs_title = _term;
        }
        break;
      case COMBINE(_SENTENCE_ANCS, 3):
        _ancs_msg = _term;
        ret_val = _SENTENCE_ANCS;
        break;
		
	  case COMBINE(_SENTENCE_DBG, 1):
        _dbg_type = natol(_term);
        break;
      case COMBINE(_SENTENCE_DBG, 2):
        _dbg_code = natol(_term);
        break;
	  case COMBINE(_SENTENCE_DBG, 3):
        _dbg_line = natol(_term);
		if (_dbg_type == 0) {
          ret_val = _SENTENCE_DBG;
        }
        break;
      case COMBINE(_SENTENCE_DBG, 4):
        if (_dbg_type == 1) {
          _dbg_file = _term;
          ret_val = _SENTENCE_DBG;
        }
        break;
      
	  case COMBINE(_SENTENCE_BTN, 1):
        _btn = natol(_term) + 1;
        ret_val = _SENTENCE_BTN;
        break;
		
      case COMBINE(_SENTENCE_PC, 1):
        _pc = natol(_term);
      	ret_val = _SENTENCE_PC;
        break;

      case COMBINE(_SENTENCE_QY, 1):
        _qy = natol(_term);
        _qy_msg = "";
        break;
      case COMBINE(_SENTENCE_QY, 2):
        _qy_msg = _term;
        ret_val = _SENTENCE_QY;
        break;
    }
  }

  return ret_val;
}

long VParser::natol(const char *str) {
  long ret = 0;
  while (isdigit(*str))
    ret = 10 * ret + *str++ -'0';
  return ret;
}

int VParser::nstrcmp(const char *str1, const char *str2) {
  while (*str1 && *str1 == *str2)
    ++str1, ++str2;
  return *str1;
}

int32_t VParser::parse_decimal()
{
  char *p = _term;
  bool isneg = *p == '-';
  if (isneg) ++p;
  int32_t ret = 100 * (int32_t)natol(p);
  while (isdigit(*p)) ++p;
  if (*p == '.')
  {
    if (isdigit(p[1]))
    {
      ret += 10 * (p[1] - '0');
      if (isdigit(p[2]))
        ret += p[2] - '0';
    }
  }
  return isneg ? -ret : ret;
}

int32_t VParser::parse_sint()
{
  char *p = _term;
  bool isneg = *p == '-';
  if (isneg) ++p;
  int32_t ret = natol(p);

  return isneg ? -ret : ret;
}


String VParser::encodeOrder(float avance, float curTime) {
  String res = "$";
  String tmp;
  int i_tmp;
  uint8_t avt_pt, ap_pt;

  float partner = 100 * avance / curTime;

  if (avance > 0.) {
    res += "01";
  } else {
    res += "02";
  }

  //Serial.print(String("Partner= ") + String(partner, 2) + "  -> ");

  avt_pt = (uint8_t)fabs(partner);
  i_tmp = (int)(fabsf(partner) * 100.f);
  i_tmp = i_tmp % 100;
  ap_pt = (uint8_t)i_tmp;

  if (avt_pt < 16) res += "0";
  res += String(avt_pt, 16);

  if (ap_pt < 16) res += "0";
  res += String(ap_pt, 16);

  while(res.length() <= 8) {
    res += "0";
  }

#ifdef __DEBUG_NRF__
  Serial.println(res);
#endif
  
  return " " + res;
}


