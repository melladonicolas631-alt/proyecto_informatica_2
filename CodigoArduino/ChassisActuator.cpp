#include "ChassisActuator.h"

/**
 * @brief Inicializa los atributos privados con los pines del puente H.
 */
ChassisActuator::ChassisActuator(int in1, int in2, int ena, int in3, int in4, int enb) {
    pinIN1 = in1; pinIN2 = in2; pinENA = ena;
    pinIN3 = in3; pinIN4 = in4; pinENB = enb;
}

/**
 * @brief Configura los pines como salidas digitales y enlaza el PWM en Core 3.x.
 */
void ChassisActuator::begin() {
    pinMode(pinIN1, OUTPUT); pinMode(pinIN2, OUTPUT); pinMode(pinENA, OUTPUT);
    pinMode(pinIN3, OUTPUT); pinMode(pinIN4, OUTPUT); pinMode(pinENB, OUTPUT);
    
    // Frecuencia de 1000 Hz: par optimo de arranque en transistores bipolares L298N
    ledcAttach(pinENA, 1000, 8);
    ledcAttach(pinENB, 1000, 8);
}

/**
 * @brief Acciona el Canal A del L298N (Traccion trasera).
 */
void ChassisActuator::setDrive(int speed) {
    speed = constrain(speed, -255, 255);
    if (speed > 0) {
        digitalWrite(pinIN1, HIGH); digitalWrite(pinIN2, LOW); 
        ledcWrite(pinENA, speed);
    } else if (speed < 0) {
        digitalWrite(pinIN1, LOW); digitalWrite(pinIN2, HIGH); 
        ledcWrite(pinENA, -speed);
    } else {
        digitalWrite(pinIN1, LOW); digitalWrite(pinIN2, LOW); 
        ledcWrite(pinENA, 0);
    }
}

/**
 * @brief Acciona el Canal B del L298N (Direccion delantera).
 */
void ChassisActuator::setSteering(int force) {
    force = constrain(force, -255, 255);
    if (force > 0) { 
        digitalWrite(pinIN3, HIGH); digitalWrite(pinIN4, LOW); 
        ledcWrite(pinENB, force);
    } else if (force < 0) { 
        digitalWrite(pinIN3, LOW); digitalWrite(pinIN4, HIGH); 
        ledcWrite(pinENB, -force);
    } else { 
        digitalWrite(pinIN3, LOW); digitalWrite(pinIN4, LOW); 
        ledcWrite(pinENB, 0);
    }
}