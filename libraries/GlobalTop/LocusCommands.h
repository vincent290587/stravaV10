/*
 * LocusCommands.h
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_GLOBALTOP_LOCUSCOMMANDS_H_
#define LIBRARIES_GLOBALTOP_LOCUSCOMMANDS_H_

#define PMTK_SET_NMEA        "$PMTK253,0*2A\r\n"
#define PMTK_SET_BIN         "$PMTK253,1*2B\r\n"

#define PMTK_SET_NMEA_BAUD_115200 "$PMTK251,115200*1F\r\n"
#define PMTK_SET_NMEA_BAUD_57600  "$PMTK251,57600*2C\r\n"
#define PMTK_SET_NMEA_BAUD_9600   "$PMTK251,9600*17\r\n"

// turn on only the second sentence (GPRMC)
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"
// turn on GPRMC and GGA
#define PMTK_SET_NMEA_OUTPUT_RMCGGA  "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"
// turn on ALL THE DATA
#define PMTK_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"
// turn off output
#define PMTK_SET_NMEA_OUTPUT_OFF     "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"

// to generate your own sentences, check out the MTK command datasheet and use a checksum calculator
// such as the awesome http://www.hhhh.org/wiml/proj/nmeaxor.html

#define PMTK_ENABLE_SBAS "$PMTK313,1*2E\r\n"
#define PMTK_ENABLE_WAAS "$PMTK301,2*2E\r\n"

// standby command & boot successful message
#define PMTK_STANDBY "$PMTK161,0*28\r\n"
#define PMTK_STANDBY_SUCCESS "$PMTK001,161,3*36"  // Not needed currently
#define PMTK_AWAKE "$PMTK000*32\r\n" // random sentence to wake

#define PMTK_FULL_COLD "$PMTK104*37\r\n" // full cold start
#define PMTK_COLD      "$PMTK103*30\r\n" // cold start

// ask for the release and version
#define PMTK_Q_RELEASE "$PMTK605*31\r\n"

// request for updates on antenna status
#define PGCMD_ANTENNA "$PGCMD,33,1*6C\r\n"
#define PGCMD_NOANTENNA "$PGCMD,33,0*6D\r\n"


#endif /* LIBRARIES_GLOBALTOP_LOCUSCOMMANDS_H_ */
