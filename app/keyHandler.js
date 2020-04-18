var config = require('./config.js'),
lastTime = {},
windowID = 'unfilled',
throttledCommands = config.throttledCommands,
regexThrottle = new RegExp('^(' + throttledCommands.join('|') + ')$', 'i'),
regexFilter = new RegExp('^(' + config.filteredCommands.join('|') + ')$', 'i');

var net = require('net');
var normalize = require('normalize-space');

var HOST = '127.0.0.1';
var PORT = 1025;

var clientReady = false;
var client = new net.Socket();
client.connect(PORT, HOST, function() {
  console.log('CONNECTED TO: ' + HOST + ':' + PORT);
  clientReady = true;
  setInterval(function() {
    client.write("nothing\n");
  }, 1500);
});

// Add a 'data' event handler for the client socket
// data is what the server sent to this socket
client.on('data', function(data) {
  // Nothing here, maybe
});

// Add a 'close' event handler for the client socket
client.on('close', function() {
  console.log('Connection closed');
});

function sendKey(command) {
  //if doesn't match the filtered words
  if (!command.match(regexFilter) &&
      clientReady) {
    client.write(normalize(command) + '\n');
  }
}

exports.sendKey = sendKey;
