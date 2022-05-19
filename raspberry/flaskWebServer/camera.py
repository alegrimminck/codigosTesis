# Se importa openCV con cv2 para la inteligencia artificial de reconocimiento de caras
import cv2
# El haarcascade tiene precargado un reconocmiento de caras
face_cascade=cv2.CascadeClassifier("haarcascade_frontalface_default.xml")
ds_factor=0.6

class VideoCamera(object):
	def __init__(self):
		# De esta forma se captura la webcam conectada por defecto
		self.video = cv2.VideoCapture(0)

	def __del__(self):
		# Con esto se suelta a la camara
		self.video.release()
		
	def get_frame(self):
		# En esta función se extrae cada imagen del video y se procesa para encontrar un rostro
		ret, frame = self.video.read()
		frame=cv2.resize(frame,None,fx=ds_factor,fy=ds_factor,interpolation=cv2.INTER_AREA)                    
		gray=cv2.cvtColor(frame,cv2.COLOR_BGR2GRAY)
		face_rects=face_cascade.detectMultiScale(gray,1.3,5)
		
		# Se dibuja un rectángulo encima de cada rostro encontrado
		for (x,y,w,h) in face_rects:
			cv2.rectangle(frame,(x,y),(x+w,y+h),(0,255,0),2)
			break
			
		# Se encripta el frame bruto de OpenCV a JPG
		ret, jpeg = cv2.imencode('.jpg', frame)
		return jpeg.tobytes()
