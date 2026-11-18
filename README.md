# Trabajo Final Integrador - Informática II (UTN FRM)

**Autor:** Nicolás Gabriel Mellado Mala  
**Carrera:** Ingeniería en Telecomunicaciones  
**Institución:** Universidad Tecnológica Nacional - Facultad Regional Mendoza (UTN FRM)  
**Año:** 2026  

---

## 1. Descripción General del Proyecto
Este proyecto implementa un sistema integral de control vehicular radiocontrolado y estación de telemetría bidireccional. El prototipo físico utiliza un chasis con tracción trasera mediante motor de corriente continua (DC) y tren de dirección delantero accionado por un segundo motor DC con retorno mecánico al centro, gobernados a través de un microcontrolador **ESP32 (ESP-WROOM-32 de 38 pines)** y un puente H doble **L298N**.

El ecosistema de software permite operar el vehículo de forma dual:
1. **Desde la PC (Estación de Control en Processing):** Comunicación serie UART a 115200 bps con interfaz gráfica en tiempo real, control por teclado y registro histórico en disco.
2. **Desde Dispositivo Móvil (Dabble App):** Control inalámbrico mediante Bluetooth Low Energy (BLE).

---

## 2. Cumplimiento de Requisitos Académicos (Informática II)
El desarrollo satisface la totalidad de las pautas exigidas por la cátedra:

* **Comunicación Bidireccional PC-Microcontrolador:** Protocolo serie asíncrono de tiempo real mediante envío de caracteres de comando (`W`, `S`, `A`, `D`, `F`, `C`, `X`) desde Processing y retorno periódico de tramas de telemetría (`ST`, `DRV`, `STR`) cada 200 ms desde el ESP32, con control de líneas `DTR/RTS` desacoplado para evitar reinicios de hardware.
* **Máquina de Estados Finitos (FSM) e Indicador Lumínico:** Implementada tanto en el ESP32 (`STATE_STANDBY`, `STATE_ACTIVE`, `STATE_BRAKE`) como en Processing (`STATE_DISCONNECTED`, `STATE_STANDBY`, `STATE_DRIVING`, `STATE_BRAKING`). El ESP32 señaliza su estado mediante el LED azul integrado (`GPIO 2`):
  * *Parpadeo lento (1 Hz):* Sistema en espera (`STATE_STANDBY`).
  * *Encendido fijo:* Tracción o dirección activa (`STATE_ACTIVE`).
  * *Parpadeo rápido (5 Hz):* Bloqueo de freno de emergencia (`STATE_BRAKE`).
* **Programación Orientada a Objetos (POO) y Librerías Propias:** Separación estricta entre la declaración de tipos y la implementación lógica en ambos entornos (`ChassisActuator.h` / `ChassisActuator.cpp` en C++ y `VehicleTypes.pde` / `TelemetryLogger.pde` en Processing).
* **Contenedores en Memoria y Persistencia en Archivo (Modo Append):** La clase `TelemetryLogger` gestiona un contenedor dinámico `ArrayList<TelemetryRecord>` en memoria RAM. Al iniciar la aplicación, **recupera el historial previo** desde el archivo `datos_auto.txt` y anexa los nuevos eventos de conducción al final del archivo (`FileWriter` en modo *append*) sin sobrescribir sesiones anteriores.
* **Optimización Electromecánica:** Reloj de CPU escalado a 80 MHz para eficiencia energética, inhibición del registro de *Brownout* por picos inductivos y algoritmo de compensación de zona muerta (*Deadband Compensation*) con rampa de aceleración progresiva a 1 kHz PWM.

---

## 3. Estructura del Repositorio

    proyecto_informatica_2/
    ├── README.md
    ├── Arduino/
    │   └── Auto_dabble/
    │       ├── Auto_dabble.ino         # Lógica principal, FSM, UART no bloqueante y BLE
    │       ├── ChassisActuator.h       # Declaración de la clase y prototipos de librería
    │       └── ChassisActuator.cpp     # Implementación de control PWM y puentes H
    └── Processing/
        └── ControlPanel/
            ├── ControlPanel.pde        # Interfaz gráfica (GUI), eventos de teclado y puerto serie
            ├── VehicleTypes.pde        # Declaración de tipos (enum AppState y clase TelemetryRecord)
            └── TelemetryLogger.pde     # Gestión de ArrayList, lectura inicial y guardado en modo append

---

## 4. Mapa de Conexiones Físicas (Flanco Derecho ESP32 a L298N)

| Subsistema | Pin L298N / Componente | Pin ESP32 (GPIO) | Función Técnica |
| :--- | :--- | :--- | :--- |
| **Tracción (Canal A)** | `ENA` | `GPIO 14` (D14) | Señal PWM de velocidad trasera (1 kHz, 8 bits) |
| **Tracción (Canal A)** | `IN1` | `GPIO 12` (D12) | Sentido de giro A (Avance / Reversa) |
| **Tracción (Canal A)** | `IN2` | `GPIO 13` (D13) | Sentido de giro B (Avance / Reversa) |
| **Dirección (Canal B)** | `ENB` | `GPIO 25` (D25) | Señal PWM de fuerza de dirección (1 kHz, 8 bits) |
| **Dirección (Canal B)** | `IN3` | `GPIO 27` (D27) | Giro delantero (Invertido por software para corrección de fase) |
| **Dirección (Canal B)** | `IN4` | `GPIO 26` (D26) | Giro delantero (Invertido por software para corrección de fase) |
| **Estado FSM** | LED Azul Integrado | `GPIO 2` (D2) | Señalización visual de la Máquina de Estados |
| **Alimentación** | `12V` / `GND` / `+5V` | `VIN` / `GND` | Baterías 18650 (2S) a etapa de potencia y masa común |

---

## 5. Instrucciones de Uso y Controles

### A. Operación desde PC (Processing)
1. Conectar el ESP32 a la PC mediante cable USB (puerto `COM3` o último puerto enumerado) y encender la alimentación del puente H.
2. Abrir `Processing/ControlPanel/ControlPanel.pde` en Processing IDE y presionar **Run**.
3. Hacer clic sobre la ventana gráfica y utilizar el teclado:
   * **`W` / `S`:** Acelerar hacia adelante / Retroceder.
   * **`A` / `D`:** Girar dirección a la izquierda / derecha.
   * **`ESPACIO`:** Freno de bloqueo inmediato (`STATE_BRAKE`).

### B. Operación desde Smartphone (Dabble)
1. Abrir la aplicación **Dabble** en el teléfono, seleccionar el módulo **Gamepad** y vincular con el dispositivo Bluetooth **`Auto_Nicolas`**.
2. Controles del Gamepad:
   * **Cruz (`X`):** Avanzar.
   * **Círculo (`O`):** Retroceder.
   * **Cuadrado (`□`):** Frenar.
   * **Flechas Izquierda / Derecha:** Doblar dirección.