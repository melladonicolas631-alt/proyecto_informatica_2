#ifndef CHASSIS_ACTUATOR_H
#define CHASSIS_ACTUATOR_H

#include <Arduino.h>

/**
 * @class ChassisActuator
 * @brief Libreria propia para el control de traccion y direccion mediante puente H L298N.
 */
class ChassisActuator {
  private:
    int pinIN1, pinIN2, pinENA;
    int pinIN3, pinIN4, pinENB;

  public:
    /**
     * @brief Constructor que asigna los pines fisicos del flanco derecho del ESP32.
     */
    ChassisActuator(int in1, int in2, int ena, int in3, int in4, int enb);

    /**
     * @brief Inicializa los pines GPIO y configura los generadores PWM a 1 kHz.
     */
    void begin();

    /**
     * @brief Controla la velocidad y sentido del motor trasero de traccion.
     * @param speed Valor PWM entre -255 (reversa) y 255 (avance).
     */
    void setDrive(int speed);

    /**
     * @brief Controla la fuerza y sentido del motor delantero de direccion.
     * @param force Valor PWM entre -255 (izquierda) y 255 (derecha).
     */
    void setSteering(int force);
};

#endif