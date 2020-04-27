
let GPXService = require('./GpxService').GPXService;
GPXService = new GPXService();
GPXService.setAuthor("Vincent Golle");

const fs = require('fs');
const readline = require('readline');

if (process.argv.length === 2) {
    console.error('Expected at least one argument!');
    process.exit(1);
}

var f_name = process.argv[2];

console.info('Processing file ' + f_name);

const readInterface = readline.createInterface({
    input: fs.createReadStream(f_name),
    output: process.stdout,
    console: false
});


readInterface
.on('line', function(line) {

    //console.debug(line);

    var d_fields = f_name.slice(1, -4);
    //console.debug(d_fields);

    var annee = 2000 + Number(d_fields.substring(d_fields.length - 2, d_fields.length));
    d_fields = d_fields.slice(0, -2);
    var mois  = Number(d_fields.substring(d_fields.length - 2, d_fields.length)) - 1;
    d_fields = d_fields.slice(0, -2);
    var jour = Number(d_fields);

    if (line.charAt(0) != 'l') {

        var arr = line.split(";").map(val => Number(val));

        var secj = Number(arr[3]);
        var hours = secj / 3600;
        var mins = (secj % 3600) / 60;
        secj = secj % 60;

        var date_ = new Date(Date.UTC(annee, mois, jour, Math.floor(hours), Math.floor(mins), Math.floor(secj)));
        //console.debug('date: ' + annee + ' ' + mois + ' ' + jour + ' ' + Math.floor(hours) + ' ' + Math.floor(mins) + ' ' + Math.floor(secj));

        GPXService.addPoint({
            lat: arr[0],
            lon: arr[1],
            date: date_,
            elevation: arr[2],
            hrm: arr[5],
            cad: arr[6],
        });

    }
})
.on('close', function(line) {

    // EOF: save GPX
    let output = GPXService.toXML();
    var new_name = f_name.slice(1, -4) + '.gpx';
    // reset file and write
    fs.writeFileSync(new_name, output);
});


