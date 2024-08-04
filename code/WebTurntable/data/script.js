var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Update contents of the webpage
function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        document.getElementById(key).value = myObj[key];
    }
}

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function UpdateDirection() {
    var data = {
        "type": "stepping_direction",
        "content" : parseInt(document.getElementById('stepping_direction').value, 10),
    }
    websocket.send(JSON.stringify(data));
}

function UpdateSpeed() {
    var data = {
        "type": "stepping_speed",
        "content" : parseInt(document.getElementById('stepping_speed').value, 10),
    }
    websocket.send(JSON.stringify(data));
}

function UpdateMode() {
    var data = {
        "type": "stepping_mode",
        "content" : parseInt(document.getElementById('stepping_mode').value, 10),
    }
    websocket.send(JSON.stringify(data));
}