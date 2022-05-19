// Se define un timer de 200ms para enviar y recibir las señales
$(function(){
	setInterval(setTimerFunction, 200);
});

// Se definen las variables globales
let message = "T";
let send_count = 0;
let socket = io();
let flask_response = true;

let flask_data = {"value": "S", "S": "0", "W": "0"};

let gpsS = ""
let gpsW = ""
let counter = 0;

// Se añade un escuchador de eventos para cuando llega un mensaje por websockets desde flask
socket.addEventListener('message', function(event) {
	console.log("Flask manda: ", event);
	flask_response = true;
	flask_data = event;
});

// Esta es la función que se ejecutará cada 200ms
function setTimerFunction() {
	UpdateMessageWithButtonValue('btnL'); // Izq
	UpdateMessageWithButtonValue('btnR'); // Der
	UpdateMessageWithButtonValue('btnF'); // Avanzar
	UpdateMessageWithButtonValue('btnB'); // Retroceder
	UpdateMessageWithButtonValue('btnW'); // Servo horizontal izq
	UpdateMessageWithButtonValue('btnU'); // Servo horizontal der
	UpdateMessageWithButtonValue('btnV'); // Servo vertical arriba
	UpdateMessageWithButtonValue('btnX'); // Servo horizontal abajo
	
	gpshandler();
	
	// Gestionador de la respuesta de flask y envía los datos correspondientes
	if (flask_response) {
		document.getElementById('text').innerHTML = "Señales: "+send_count+"<br>GPS S: "+gpsS+"<br>GPS W: "+gpsW;	
		send_count++;
		
		send(message);
		flask_response = false;
	}
}

// Esta función envía el mensaje por websockets
function send(message){
	socket.emit('update',
	{'value':message})
}

// Esta función actualiza el valor de la variable global "Message" con el valor del botón presionado
function UpdateMessageWithButtonValue(button){
	let element = document.getElementById(button);
	
	element.onmousedown = function() {
		message = element.value;
	}
	element.onmouseup = function() {
		message = "S";
	}
	element.ontouchstart = function() {
		message = element.value;
	}
	element.ontouchend = function() {
		message = "S";
	}
	
}

// Esta función manda a procesar los valores en bruto del gps para luego ser usados por la API de google maps y actualizar el mapa en la página web.
function gpshandler() {
	if (flask_data['S'] != "0") {
		gpsS = flask_data['S'];
		gpsW = flask_data['W'];
		
		gpsS = parseGPS(gpsS);
		gpsW = parseGPS(gpsW);
		
		counter++;
		if (counter == 10) {
			document.getElementById('maps').src = "https://www.google.com/maps/embed/v1/place?key=AIzaSyDq_MXrxxllKLv5Ck-t9uV18ydNnbviTyU&q="+gpsS+","+gpsW;
			counter = 0;
		}
	}
}

// Toma el valor S y W de NMEA GPRMC y les hace un parsing para convertilos en longitud y latitud 
function parseGPS(gps_data){
	let first = "";
	let second = "";
	
	if (gps_data[0]=="0"){
		gps_data = gps_data.substring(1);
	}
	console.log(gps_data);
	first = gps_data.slice(0,2);
	second = gps_data.slice(2, 4)+gps_data.slice(5,-1);
	
	second = ((parseFloat(second)/60).toString()).split(".")[0];
	gps_data = ("-"+first+"."+second);
	console.log(gps_data);
	return gps_data;
}
