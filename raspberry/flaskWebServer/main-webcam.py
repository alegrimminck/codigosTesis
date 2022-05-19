#Se importan módulos de flask para montar un servidor con flask y se importa la función videocamera de camera.py
from flask import Flask, render_template, Response
from camera import VideoCamera
app = Flask(__name__)

# En la ruta principal se retorna el index-webcam.html
@app.route('/')
def index():
	# rendering webpage
	return render_template('index-webcam.html')

# Función utilizada por el generador de video para construir la imagen
def gen(camera):
	while True:
		#get camera frame
		frame = camera.get_frame()
		yield (b'--frame\r\n'b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n\r\n')

# Ruta en la cual se mostrará el streaming, esta respuesta utiliza la función VideoCamera() la cual viene del archivo camera.py
@app.route('/video_feed')
def video_feed():
    return Response(gen(VideoCamera()),mimetype='multipart/x-mixed-replace; boundary=frame')
    
if __name__ == '__main__':
    # Se define el puerto 8081 para stremear la camara
    app.run(host='0.0.0.0',port='8081', debug=True)
