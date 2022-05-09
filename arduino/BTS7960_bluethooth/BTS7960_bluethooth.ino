
// Control de IBT2 (con BTS7960) usando módulo HC-06 de bluethooth.

/* CONFIGURACIÓN FÍSICA DEL IBT2
 * En la configuración física del IBT2, se encuentran los siguientes pines 
 * con su configuración respectiva:
 * R_IS, L_IS: LOW
 * R_EN, L_EN: HIGH 
*/

/* GLOSARIO
 * SV: Servo Vertical
 * SH: Servo horizontal
 * acc: aceleración
 * izq: izquierda
 * der: derecha
 */

// Librerias
#include <Servo.h>

// Servo motores
Servo SH;
Servo SV;

// OUTPUTS físicos. Estos son los pines del arduino los cuales envían una señal PWM al módulo IBT2 para 
// controlar el motor de dirección y el motor de tracción en ambos sentidos
int pwm_traccion_der = 6;
int pwm_traccion_izq = 5;
int pwm_direccion_der = 11;
int pwm_direccion_izq = 3;
int SH_pwm_pin = 9;
int SV_pwm_pin = 10;

// INPUTS físicos. Estos son los pines del arduino los cuales reciben un HIGH o un LOW de los 
// switches límite de carrera para limitar el movimiento del motor de tracción.
int limite_carrera_der = 8;
int limite_carrera_izq = 7;

// INPUTS digitales. Estos son los valores que llegan al arduino desde el módulo bluethooth HC-06. 
// Los valores enviados al módulo son letras, las cuales se reciben como números del sistema ASCII
int izq = 76; // L
int der = 82; // R
int parar = 83; // S
int avanzar = 70; // F
int avanzar_izq = 71; // G 
int avanzar_der = 73; // I
int retroceder = 66; // B
int retroceder_izq = 72; // H
int retroceder_der = 74; // J

int SH_izq = 87; // W
int SH_der = 85; // U
int SV_izq = 86; // V
int SV_der = 88; // X

// VARIABLES DE CONTROL ¡SE PUEDEN AJUSTAR!
float velocidad_direccion = 0.25; // Velocidad dirección 0.15 a 1.0
float velocidad_traccion = 1.0;  // Velocidad tracción 0 a 1
float acc_traccion = 0.25;  // Aceleración tracción 0 a 1
float acc_servo = 20; // Aceleración servo motores de PantTilt 0.01 a 0.20

// Variables de código
unsigned long time_on_bt_read = 0;
float acc_pwm = 255.0*velocidad_traccion;
int bluethooth_input = 0;
unsigned long tiempo_anterior = 0; // Para "delay" de 1 milisegundo
bool mov_izq = false;
bool mov_der = false;
bool mov_avanzar = false;
bool mov_retroceder = false;
int SH_pwm = 120;
int SV_pwm = 120;

void setup() {
  Serial.begin(9600);
 
  // 4 posibles señales PWM enviadas desde arduino a IBT2 para el control de ambos motores en ambos sentidos.
  pinMode(pwm_direccion_der, OUTPUT);
  pinMode(pwm_direccion_izq, OUTPUT);
  pinMode(pwm_traccion_der, OUTPUT);
  pinMode(pwm_traccion_izq, OUTPUT);

  // 4 señales de entrada con PULL_UP interna del arduino para los limite de carrera
  pinMode(limite_carrera_der, INPUT_PULLUP);
  pinMode(limite_carrera_izq, INPUT_PULLUP);

  // Pines servo motores
  SH.attach(9);
  SV.attach(10);
}

void loop() {
  // "Delay" de 1 milisegundo para repetir todo este bloque de código
  if (millis() - tiempo_anterior >= 1){
    tiempo_anterior = millis();
    
    // Lectura bluethooth
    // El Serial.available() solo entrará cuando llegue una señal bluethooth
    if (Serial.available()){
      // Uso la función millis() para guardar el tiempo que lleva corriendo arduino, esto permite que
      // si se pierda la señal bluethooth, luego de X tiempo se apagarán los motores. El apagado se realiza más adelante.
      time_on_bt_read = millis(); 
      // Se guarda la letra recibida por Bluethooth en valor ASCII
      bluethooth_input=Serial.read();
    }
    
    // Activación movimiento dirección
    // Cada vez que la señal bluethooth diga que hay que moverse hacia la der o hacia la izq, se activaran o desactivarán respectivamente estos movimientos
    if(bluethooth_input == der or bluethooth_input == avanzar_der or bluethooth_input == retroceder_der ){
      mov_der = true;
      mov_izq = false;
    } else {
      mov_der = false;
    }
    if(bluethooth_input == izq or bluethooth_input == avanzar_izq or bluethooth_input == retroceder_izq ){
      mov_izq = true;
      mov_der = false;
    } else {
      mov_izq = false;
    }
  
    // Activación de límites de carrera en la dirección
    // Si la señal leída por el arduino de los limites de carrera es un 0 lógico, se desactivará el movimiento hacia la der e izq
    if (digitalRead(limite_carrera_der) == LOW){
      mov_der = false;  
    }
    if (digitalRead(limite_carrera_izq) == LOW){
      mov_izq = false;  
    }
  
    // Activación movimiento tracción
    // Cada vez que la señal bluethooth diga que hay que avanzar o retroceder, se activaran o desactivarán respectivamente estos movimientos
    if(bluethooth_input == avanzar or bluethooth_input == avanzar_der or bluethooth_input == avanzar_izq ){
      mov_avanzar = true;
      mov_retroceder = false;
    } else {
      mov_avanzar = false;
    }
    if(bluethooth_input == retroceder or bluethooth_input == retroceder_der or bluethooth_input == retroceder_izq ){
      mov_avanzar = false;
      mov_retroceder = true;
    } else {
      mov_retroceder = false;  
    }
  
    // Activación movimiento servos
    if (bluethooth_input == SH_izq) {
      SH_pwm = pwmServoCalculo(SH_pwm, -1, acc_servo, 0, 180);
    } else if (bluethooth_input == SH_der) {
      SH_pwm = pwmServoCalculo(SH_pwm, 1, acc_servo, 0, 180);
    } else if (bluethooth_input == SV_izq) {
      SV_pwm = pwmServoCalculo(SV_pwm, -1, acc_servo/2, 40, 150);  
    } else if (bluethooth_input == SV_der) {
      SV_pwm = pwmServoCalculo(SV_pwm, 1, acc_servo/2, 40, 150);
    }
    
    // Si por 400 milisegundos no llega señal bluethooth (Se perdió la señal), se desactivan los motores
    if (time_on_bt_read < millis() - 400){
      mov_der = false;
      mov_izq = false;
      mov_avanzar = false;
      mov_retroceder = false;
    }
    
    // Movimiento dirección
    // Si llegó una señal activa de movimiento hacia la izq o der, se activan los motores de dirección respectivamente
    if (mov_izq == true) {
      analogWrite(pwm_direccion_der, 0);
      analogWrite(pwm_direccion_izq, velocidad_direccion*255);
    } else if (mov_der == true) {
      analogWrite(pwm_direccion_der, velocidad_direccion*255);
      analogWrite(pwm_direccion_izq, 0);
    } else {
      analogWrite(pwm_direccion_der, 0);
      analogWrite(pwm_direccion_izq, 0);
    } 
  
    // Movimiento tracción
    // Si llegó una señal activa de movimiento para avanzar o retroceder, se sumará o restará a la aceleración de la tracción
    if (mov_avanzar) {
      if (acc_pwm < 250.0){
        acc_pwm += acc_traccion;
      }
    } else if (mov_retroceder) {
      if (acc_pwm > -250.0){
        acc_pwm -= acc_traccion;
      }
    } else {
      if (acc_pwm < 0.0) {
        acc_pwm += acc_traccion;
      } else {
        acc_pwm -= acc_traccion;
      }  
    }
  
    // Dependiendo si la aceleración es negativa o positiva, el carro avanzará o retrocederá
    if (acc_pwm > 0) {
      analogWrite(pwm_traccion_der, round(acc_pwm*velocidad_traccion));
      analogWrite(pwm_traccion_izq, 0);
    } else {
      analogWrite(pwm_traccion_der, 0);
      analogWrite(pwm_traccion_izq, round(-1.0*acc_pwm*velocidad_traccion));  
    }
  
    // Movimiento servo motores
    if (bluethooth_input == SH_izq or bluethooth_input == SH_der){
      SH.write(round(SH_pwm));
    } else if (bluethooth_input == SV_izq or bluethooth_input == SV_der){
      SV.write(round(SV_pwm));
    }    
  }
}

float pwmServoCalculo(float pwm, int dir, float acc, int minimo, int maximo){
  if ((dir == -1) and (pwm >= minimo+acc_servo)){
    pwm -= acc;
  } else if ((dir == 1) and (pwm <= maximo-acc_servo)){
    pwm += acc;
  }
  return pwm;
}
