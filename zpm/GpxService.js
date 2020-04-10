/**
 * Created by Silvio Rainoldi on 05.12.2016.
 *
 * Create your GPX (1.1) file
 */
function GPXService(name) {
    const self = this;


    const metadata = {
        name: '',
        author: {
            name: ''
        },
        description: '',
        date: new Date()
    };


    const xmlStart = `<?xml version="1.0"?>
<gpx
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns="http://www.topografix.com/GPX/1/1"
    xsi:schemaLocation="http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd"
    version="1.1"
    creator="${metadata.author.name}" >`;
    const xmlEnd = '</gpx>';

    /**
     * Array of points. A point can be formed by latitude, longitude, elevation and time. To add a point please use self.addPoint()
     * @type {Array}
     */
    let points = [];

    /**
     * Set the file's author
     * @param author
     */
    self.setAuthor = function (author) {
        metadata.author.name = author;
    };

    /**
     * Adds a point
     * @param point
     * {
     *   lat: float number (required)
     *   lon: float number (required)
     *   date: Date object (optional)
     *   elevation: number (optional)
     *   hrm: number (optional)
     *   cad: number (optional)
     * }
     */
    self.addPoint = function (point) {
        if (point.lat && point.lon) {
            points.push(point);
        }
        else throw Error("Point has to have 'lat' and 'lon'");
    };

    /**
     * Returns the generated XML string in GPX format
     */
    self.toXML = function () {
        let xmlOutput = xmlStart;
        xmlOutput += `<metadata>
    <name>${metadata.name}</name>
    <author><name>${metadata.author.name}</name></author>
    <time>${metadata.date.toISOString()}</time>
</metadata>`;

        xmlOutput += `<trk>
    <name>${metadata.name}</name>
    <cmt></cmt>
    <desc></desc>
    <src></src>
<trkseg>
`;

        for (let i = 0; i < points.length; i++) {
            let point = points[i];
            xmlOutput += `<trkpt lat="${point.lat}" lon="${point.lon}">
`;
            if (1) {
                xmlOutput += `    <ele>${point.elevation}</ele>
`;
            }
            if (1) {
                xmlOutput += `    <time>${point.date.toISOString()}</time>
`;
            }
            if (1) {
                xmlOutput += `    <gpxtpx:TrackPointExtension>
`;
            }
            if (1) {
                xmlOutput += `        <gpxtpx:hr>${point.hrm}</gpxtpx:hr>
`;
            }
            if (1) {
                xmlOutput += `        <gpxtpx:cad>${point.cad}</gpxtpx:cad>
`;
            }
            if (1) {
                xmlOutput += `    </gpxtpx:TrackPointExtension>
`;
            }
            xmlOutput += `</trkpt>
`;
        }

        // And add the end
        xmlOutput += `</trkset>
</trk>
${xmlEnd}`;
        return xmlOutput;
    };
}
exports.GPXService = GPXService;
