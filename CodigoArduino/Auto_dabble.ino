#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include "ChassisActuator.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Pines flanco derecho ESP32 (Traccion)
#define PIN_IN1 12
#define PIN_IN2 13
#define PIN_ENA 14

// Pines de Direccion (Intercambiados 27 y 26 para corregir sentido fisico)
#define PIN_IN3 27
#define PIN_IN4 26
#define PIN_ENB 25

// Pin LED de estado integrado en placa
#define PIN_LED_STATUS 2

/**
 * @enum VehicleState
 * @brief Estados principales de la Maquina de Estados Finitos (FSM) en el ESP32.
 */
enum VehicleState {
  STATE_STANDBY,
  STATE_ACTIVE,
  STATE_BRAKE
};

ChassisActuator car(PIN_IN1, PIN_IN2, PIN_ENA, PIN_IN3, PIN_IN4, PIN_ENB);
VehicleState currentState = STATE_STANDBY;

int currentSpeed = 0;
int pcSpeed = 0;
int pcSteer = 0;
unsigned long lastTelemetryTime = 0;

// Parametros calibrados de potencia y compensacion de zona muerta
const int MIN_START_PWM = 90;  // Impulso inicial inmediato para vencer friccion
const int MAX_DRIVE_PWM = 245; // Potencia maxima de traccion aumentada
const int STEER_PWM = 235;     // Fuerza de direccion aumentada para doblar con firmeza
const int RAMP_STEP = 18;      // Escalones agiles de aceleracion

/**
 * @brief Inicializa registros de energia, reloj de CPU, Bluetooth, UART y puente H.
 */
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  setCpuFrequencyMhz(80);

  pinMode(PIN_LED_STATUS, OUTPUT);
  Serial.begin(115200);
  Dabble.begin("Auto_Nicolas"); 
  car.begin();
}

/**
 * @brief Bucle principal que coordina entradas, maquina de estados, LED y telemetria.
 */
void loop() {
  Dabble.processInput();
  readPCCharNonBlocking();
  executeStateMachine();
  updateLedIndicator();
  delay(20);
}

/**
 * @brief Evalua las entradas de Dabble y PC y ejecuta la Maquina de Estados.
 */
void executeStateMachine() {
  // 1. Determinacion de estado y calculo de traccion
  if (GamePad.isSquarePressed()) {
    currentState = STATE_BRAKE;
    currentSpeed = 0;
    pcSpeed = 0;
  } 
  else if (GamePad.isCrossPressed() || pcSpeed > 0) {
    currentState = STATE_ACTIVE;
    if (currentSpeed < MIN_START_PWM) currentSpeed = MIN_START_PWM;
    else {
      currentSpeed += RAMP_STEP;
      if (currentSpeed > MAX_DRIVE_PWM) currentSpeed = MAX_DRIVE_PWM;
    }
  } 
  else if (GamePad.isCirclePressed() || pcSpeed < 0) {
    currentState = STATE_ACTIVE;
    if (currentSpeed > -MIN_START_PWM) currentSpeed = -MIN_START_PWM;
    else {
      currentSpeed -= RAMP_STEP;
      if (currentSpeed < -MAX_DRIVE_PWM) currentSpeed = -MAX_DRIVE_PWM;
    }
  } 
  else {
    currentSpeed = 0;
  }

  car.setDrive(currentSpeed);

  // 2. Calculo de direccion
  int steerOut = 0;
  if (GamePad.isLeftPressed() || pcSteer < 0) {
    steerOut = -STEER_PWM;
    if (currentState != STATE_BRAKE) currentState = STATE_ACTIVE;
  } 
  else if (GamePad.isRightPressed() || pcSteer > 0) {
    steerOut = STEER_PWM;
    if (currentState != STATE_BRAKE) currentState = STATE_ACTIVE;
  }

  car.setSteering(steerOut);

  // 3. Retorno a reposo si no hay actividad
  if (currentSpeed == 0 && steerOut == 0 && !GamePad.isSquarePressed() && currentState != STATE_BRAKE) {
    currentState = STATE_STANDBY;
  }

  // 4. Transmision de telemetria bidireccional a Processing
  sendTelemetry(steerOut);
}

/**
 * @brief Lee comandos de un caracter desde Processing en 1 microsegundo sin bloquear.
 */
void readPCCharNonBlocking() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'W' || c == 'w') { pcSpeed = MAX_DRIVE_PWM; currentState = STATE_ACTIVE; }
    else if (c == 'S' || c == 's') { pcSpeed = -MAX_DRIVE_PWM; currentState = STATE_ACTIVE; }
    else if (c == 'F' || c == 'f') { pcSpeed = 0; if (pcSteer == 0) currentState = STATE_STANDBY; }
    else if (c == 'A' || c == 'a') { pcSteer = -STEER_PWM; currentState = STATE_ACTIVE; }
    else if (c == 'D' || c == 'd') { pcSteer = STEER_PWM; currentState = STATE_ACTIVE; }
    else if (c == 'C' || c == 'c') { pcSteer = 0; if (pcSpeed == 0) currentState = STATE_STANDBY; }
    else if (c == 'X' || c == 'x') { pcSpeed = 0; pcSteer = 0; currentState = STATE_BRAKE; }
  }
}

/**
 * @brief Controla el LED integrado (GPIO 2) para reflejar visualmente el estado FSM.
 */
void updateLedIndicator() {
  if (currentState == STATE_STANDBY) {
    // Parpadeo lento (1 Hz) en espera
    digitalWrite(PIN_LED_STATUS, (millis() % 1000) < 500);
  } else if (currentState == STATE_ACTIVE) {
    // Encendido continuo en movimiento
    digitalWrite(PIN_LED_STATUS, HIGH);
  } else {
    // Parpadeo rapido (5 Hz) en freno de emergencia
    digitalWrite(PIN_LED_STATUS, (millis() % 200) < 100);
  }
}

/**
 * @brief Envia el estado actual y valores PWM hacia la PC cada 200 ms.
 * @param steerVal Valor PWM actual aplicado al eje de direccion.
 */
void sendTelemetry(int steerVal) {
  if (millis() - lastTelemetryTime >= 200) {
    lastTelemetryTime = millis();
    Serial.print("ST:");
    Serial.print((int)currentState);
    Serial.print(",DRV:");
    Serial.print(currentSpeed);
    Serial.print(",STR:");
    Serial.println(steerVal);
  }
}