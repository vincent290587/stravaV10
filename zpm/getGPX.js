
console.log('Create monitor')

const fs = require('fs');

var has_started = 0;

// Get process.stdin as the standard input object.
var standard_input = process.stdin;
var f_name = 'tmp_log.csv'

try {
    const SerialPort = require('serialport')
    var port = new SerialPort('COM8', {
        baudRate: 115200,
        rtscts: true,
    })

    port.on('data', function (data) {

        if (has_started == 0) {
            console.log('< ' + data)
            // Prompt user to input data in console.
            console.log("Please input file name :");
        } else {
            has_started++;
            console.log('Xfers nb : ' + has_started)
            fs.appendFileSync(f_name, '' + data);
        }
    });

    port.on('close', () => {
        try {
            console.log('Port reconnect')
            setTimeout(this.reconnect.bind(this), 5000);
        } catch (e) {
            console.log(e)
        }
    });

    port.on('error', () => {
        try {
            console.log('Port reconnect')
            setTimeout(this.reconnect.bind(this), 5000);
        } catch (e) {
            console.log(e)
        }
    });

} catch (e) {
    console.log(e)
}

// Set input character encoding.
standard_input.setEncoding('utf-8');

// send it to device
for (var i = 1; i <= 1; i++) {
    setTimeout(function (i) {

        try {
            if (has_started == 0) {
                port.write('$QRY,1,EMPTY.TXT\r\n');
            }
        } catch (e) {
            console.log(e)
        }
    }, 1000 * i, i);
}

// When user input data and click enter key.
standard_input.on('data', function (data) {

    // User input exit.
    if (data === 'exit\n') {
        // Program exit.
        console.log("User input complete.");
        process.exit();
    } else {
        if (has_started) {
            // TODO call conversion
            process.exit();
        }
        // Print user input in console.
        console.log('User Input Data : ' + data);
        console.log('Requesting file...');
        // send it to device
        f_name = '' + data;
        f_name = f_name.replace("\r\n", "");
        port.write('$QRY,2,' + f_name + '\r\n');
        fs.writeFileSync(f_name, 'lat;lon;alt;secj;pwr;hrm;cad;alpha_bar;alpha_zero;baro_alt;baro_corr;climb;f_ele;gps_alt;v_asc;');

        has_started = 1;
    }
});
