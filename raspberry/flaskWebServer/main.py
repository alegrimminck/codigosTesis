#!/usr/bin/env python3
#Se importa la librería serial para hacer comunicación serial entre Raspberry y arduino
import serial

#Se importan módulos de flask t SocketIO para montar un servidor con flask y hacer comunicación por websockets
from flask import Flask, render_template
from flask_socketio import SocketIO, send

app = Flask(__name__)
socketio = SocketIO(app,cors_allowed_origins="*")

# Ruta principal que retorna el index.html
@app.route("/")
def index():
    return render_template("index.html")

# Ruta update que se activa cada vez que recibe señal del cliente
@socketio.on("update")
def update(data):
	value = data['value']
	# Envía los valores al arduino por comunicación serial
	ser.write(value.encode('UTF-8'))
		
	# GPS parsing
	line = gps.readline().decode('UTF-8')
	data = line.split(",")
	
	S="0"
	W="0"
	
	if data[0] == "$GPRMC":
		neededData = (data[3], data[5])
		S = data[3]
		W = data[5]
		
	# Envía los valores por websockets al cliente
	send({"value": value, "S": S, "W": W})
	# Imprime los valores en consola
	print('Current Value', value)

if __name__ == "__main__":
	# Define una comunicación serial de 9600 baudios con arduino y una comunicación de 9600 baudios con el GPS
	gps = serial.Serial('/dev/ttyUSB-GPS', 9600, timeout=1)
	ser = serial.Serial('/dev/ttyUSB-ARDUINO', 9600, timeout=1)
	ser.reset_input_buffer()
	# Corre la aplicación web en el puerto 80 en localhost
	socketio.run(app, host='0.0.0.0', port=80, debug=True)
