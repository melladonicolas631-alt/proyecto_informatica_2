/**
 * Enumeración de estados de la máquina de estados en PC.
 */
enum AppState {
  STATE_DISCONNECTED,
  STATE_STANDBY,
  STATE_DRIVING,
  STATE_BRAKING
}

/**
 * Clase contenedora que modela un registro de telemetría del vehículo.
 */
class TelemetryRecord {
  String timestamp;
  int stateCode;
  int drivePWM;
  int steerPWM;

  /**
   * Constructor para inicializar un nodo de telemetría.
   */
  TelemetryRecord(String ts, int st, int drv, int str) {
    this.timestamp = ts;
    this.stateCode = st;
    this.drivePWM = drv;
    this.steerPWM = str;
  }

  /**
   * Formatea el objeto como una línea de texto estructurada para persistencia.
   */
  String toFileString() {
    return timestamp + " | ESTADO:" + stateCode + " | TRACCION:" + drivePWM + " | DIRECCION:" + steerPWM;
  }
}
