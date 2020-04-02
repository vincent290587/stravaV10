
try {
    console.log('Require: ZwiftPacketMonitor')
    var ZwiftPacketMonitor = require('@zwfthcks/zwift-packet-monitor')
    console.log('Create monitor')
} catch(e) {
    console.log(e)
}

try {
    var Cap = require('cap').Cap;
} catch(e) {
    console.log(e)
}

const winston = require('winston')
const myWinstonOptions = {
    format: winston.format.simple(),
    transports: [
        new winston.transports.File({ filename: 'LNS.log' }),
    ],
    exitOnError: false,
    handleExceptions: true,
}
const logger = new winston.createLogger(myWinstonOptions)

const ip = require('internal-ip').v4.sync()

try {
	const SerialPort = require('serialport')
	var port = new SerialPort('COM22', {
        baudRate: 115200,
        rtscts: true,
    })

    port.on('data', function(data) {
        console.log('< ' + data)
        logger.info('< ' + data)
    });

	port.on('close', () => {
        try {
            logger.error('Port reconnect')
            setTimeout(this.reconnect.bind(this), 5000);
        } catch(e) {
            console.log(e)
        }
    });

	port.on('error', () => {
        try {
            logger.error('Port reconnect')
            setTimeout(this.reconnect.bind(this), 5000);
        } catch(e) {
            console.log(e)
        }
    });

} catch(e) {
    console.log(e)
}

function decimalToHexString(number)
{
  let ret = ''
  if (number < 16)
  {
    ret += '0'
  }

  ret += number.toString(16).toUpperCase();

  return ret;
}

function xor_checksum(byteArray) {
    let checksum = 0x00
    for(let i = 1; i < byteArray.length - 1; i++)
      checksum ^= byteArray[i]
    return checksum
  }

var m_sec_j = 0;
var m_x = 0;
var m_y = 0;
var m_alt = 0;
var m_speed = 0;

function send_lns(sec_j) {

	var date = new Date();

	if (m_sec_j != sec_j) {
		// world is watopia

	    console.log('Send LOC');

		//update time
		m_sec_j = sec_j;

		// x: Math.round((lat - 348.35518) * 11050000),
		// y: Math.round((long - 166.95292) * 10920000)
		var lat = 10000000 * ((m_x / 11050000) + 348.35518 - 360);
		var lon = 10000000 * ((m_y / 10920000) + 166.95292);
		var ele = m_alt * 100; // ele in cm
		var spd = m_speed * 100 / 3.6; // speed in cm / s

		let ser_msg = '$LOC,' + sec_j.toFixed(0)
		ser_msg += ',' + lat.toFixed(0)
		ser_msg += ',' + lon.toFixed(0)
		ser_msg += ',' + ele.toFixed(0)
		ser_msg += ',' + spd.toFixed(0)

		let chk = xor_checksum(ser_msg);

        ser_msg += '*' + decimalToHexString(chk)
        ser_msg += '\r\n'

		port.write(ser_msg)
		console.log(ser_msg);
	}
}

if (ZwiftPacketMonitor && Cap) {

    console.log('Listening on: ', ip, JSON.stringify(Cap.findDevice(ip),null,4));

    // determine network interface associated with external IP address
    interface = Cap.findDevice(ip);

    // ... and setup monitor on that interface:
    const monitor = new ZwiftPacketMonitor(interface);

    monitor.on('outgoingPlayerState', (playerState, serverWorldTime) => {

        //logger.info('New state')
        //console.log(playerState)

        m_x = playerState.x;
        m_y = playerState.y;
        m_alt = playerState.altitude / 4000; // m_alt in m
        m_speed = playerState.speed / 1000000; // m_speed in km/h

        send_lns(playerState.time);

    })

    monitor.on('incomingPlayerState', (playerState, serverWorldTime) => {

        //console.log(playerState)

        //m_x = playerState.x;
        //m_y = playerState.y;
        //m_alt = playerState.altitude / 200;
        //m_speed = playerState.speed / 1000000;

        //send_lns(playerState.time);

    })

    monitor.start()

}
