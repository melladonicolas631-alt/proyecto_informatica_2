import processing.serial.*;

Serial espPort;
TelemetryLogger logger;
AppState currentAppState = AppState.STATE_DISCONNECTED;

String connectedPortName = "NINGUNO";
int rxDrive = 0;
int rxSteer = 0;

/**
 * Configura la ventana grafica, recupera el archivo historico y abre el puerto sin bloquear el ESP32.
 */
void setup() {
  size(620, 380);
  logger = new TelemetryLogger("datos_auto.txt");

  String[] ports = Serial.list();
  printArray(ports);

  if (ports.length > 0) {
    try {
      String targetPort = ports[ports.length - 1];
      for (String p : ports) {
        if (p.equals("COM3")) targetPort = "COM3";
      }
      espPort = new Serial(this, targetPort, 115200);
      
      // CRÍTICO: Libera las líneas DTR y RTS para no apagar ni bloquear el ESP32
      espPort.setDTR(false);
      espPort.setRTS(false);
      
      espPort.bufferUntil('\n');
      connectedPortName = targetPort;
      currentAppState = AppState.STATE_STANDBY;
    } catch (Exception e) {
      currentAppState = AppState.STATE_DISCONNECTED;
    }
  }
}

/**
 * Bucle principal de renderizado de la interfaz grafica.
 */
void draw() {
  background(25, 30, 38);
  drawHeader();
  drawTelemetryPanel();
  drawHistoryPanel();
}

/**
 * Dibuja el bloque superior con el estado de la FSM y el puerto activo.
 */
void drawHeader() {
  fill(40, 48, 60);
  rect(20, 20, 580, 60, 8);
  textSize(18);
  fill(255);
  text("UTN FRM - Informatica II | Estacion de Control y Telemetria", 35, 48);
  textSize(13);
  fill(120, 220, 140);
  text("Estado FSM: " + currentAppState.name() + " | Puerto: " + connectedPortName, 35, 70);
}

/**
 * Dibuja los indicadores de potencia recibidos desde el ESP32.
 */
void drawTelemetryPanel() {
  fill(40, 48, 60);
  rect(20, 95, 580, 150, 8);
  fill(220);
  textSize(14);
  text("Controles PC: [W/S] Traccion | [A/D] Giro | [ESPACIO] Freno", 35, 130);
  text("PWM Traccion Real (ESP32): " + rxDrive, 35, 170);
  text("PWM Direccion Real (ESP32): " + rxSteer, 35, 205);
}

/**
 * Dibuja el contador del contenedor ArrayList y el ultimo registro persistido.
 */
void drawHistoryPanel() {
  fill(40, 48, 60);
  rect(20, 260, 580, 100, 8);
  fill(255, 210, 100);
  text("Registros en Memoria (ArrayList + Modo Append): " + logger.getTotalRecords(), 35, 290);
  fill(200);
  textSize(12);
  text("Ultimo registro: " + logger.getLastRecordSummary(), 35, 325);
}

/**
 * Transmite el caracter de comando al presionar una tecla en la ventana.
 */
void keyPressed() {
  if (espPort == null) return;
  if (key == 'w' || key == 'W') espPort.write('W');
  if (key == 's' || key == 'S') espPort.write('S');
  if (key == 'a' || key == 'A') espPort.write('A');
  if (key == 'd' || key == 'D') espPort.write('D');
  if (key == ' ') espPort.write('X');
}

/**
 * Transmite el caracter de parada al soltar la tecla correspondiente.
 */
void keyReleased() {
  if (espPort == null) return;
  if (key == 'w' || key == 'W' || key == 's' || key == 'S') espPort.write('F');
  if (key == 'a' || key == 'A' || key == 'd' || key == 'D') espPort.write('C');
}

/**
 * Procesa las tramas de telemetria entrantes y actualiza la FSM y el archivo.
 */
void serialEvent(Serial port) {
  String line = port.readStringUntil('\n');
  if (line == null) return;
  line = trim(line);

  if (line.startsWith("ST:")) {
    String[] parts = split(line, ',');
    if (parts.length == 3) {
      int st = int(trim(parts[0].replace("ST:", "")));
      rxDrive = int(trim(parts[1].replace("DRV:", "")));
      rxSteer = int(trim(parts[2].replace("STR:", "")));

      if (st == 0) currentAppState = AppState.STATE_STANDBY;
      else if (st == 1) currentAppState = AppState.STATE_DRIVING;
      else if (st == 2) currentAppState = AppState.STATE_BRAKING;

      if (rxDrive != 0 || rxSteer != 0) {
        logger.addRecord(st, rxDrive, rxSteer);
      }
    }
  }
}
