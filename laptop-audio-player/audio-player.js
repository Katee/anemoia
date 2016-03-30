require('sugar');
var serialport = require("serialport");
var spawn = require('child_process').spawn;
var fs = require('fs');

var options = require('./options');

var SerialPort = serialport.SerialPort;
var arduino = module.exports = new SerialPort(options.serialDevice, {baudrate: options.baudrate, parser: serialport.parsers.readline("\n")});
var player;

arduino.on("data", function (data) {
  console.log("arudino> " + data);

  var audioFilename = 'audio/' + data.compact() + ".wav";

  if (fs.existsSync(audioFilename)) {
    if (player != null) {
      player.kill();
      player = null;
    }

    console.log("## playing file: " + audioFilename);
    player = spawn('afplay', [audioFilename]);
  }
});

