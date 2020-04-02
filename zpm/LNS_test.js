
console.log('Create monitor')

const winston = require('winston')
const myWinstonOptions = {
    format: winston.format.simple(),
    transports: [
        new winston.transports.File({ filename: 'LNS_test.log' }),
    ],
    exitOnError: false,
    handleExceptions: true,
}
const logger = new winston.createLogger(myWinstonOptions)


try {
	const SerialPort = require('serialport')
	var port = new SerialPort('COM8', {
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

var interval = 1 * 1000; // 1 second;

for (var i = 0; i <=20; i++) {
    setTimeout( function (i) {

        let ser_msg = '$LOC,' + i + ',-116445536,1669552923,46,797\r\n'
        console.log(ser_msg)

        try {
            port.write(ser_msg)
            logger.info(ser_msg)
        } catch(e) {
            console.log(e)
        }
    }, interval * i, i);
}


