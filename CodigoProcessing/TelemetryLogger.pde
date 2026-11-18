import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.File;

/**
 * Clase encargada del manejo en memoria (ArrayList) y persistencia en archivo.
 */
class TelemetryLogger {
  private ArrayList<TelemetryRecord> records;
  private String filePath;

  /**
   * Constructor de la clase TelemetryLogger.
   */
  TelemetryLogger(String fileName) {
    this.filePath = sketchPath(fileName);
    this.records = new ArrayList<TelemetryRecord>();
    loadHistoryFromFile();
  }

  /**
   * Recupera los datos previos del archivo al iniciar para no perder el historial.
   */
  void loadHistoryFromFile() {
    File f = new File(filePath);
    if (!f.exists()) return;

    String[] lines = loadStrings(filePath);
    if (lines == null) return;

    for (String line : lines) {
      String[] parts = split(line, '|');
      if (parts.length == 4) {
        String ts = trim(parts[0]);
        int st = int(trim(parts[1].replace("ESTADO:", "")));
        int drv = int(trim(parts[2].replace("TRACCION:", "")));
        int str = int(trim(parts[3].replace("DIRECCION:", "")));
        records.add(new TelemetryRecord(ts, st, drv, str));
      }
    }
  }

  /**
   * Agrega un nuevo registro al contenedor en memoria y lo guarda en modo append.
   */
  void addRecord(int stateCode, int drivePWM, int steerPWM) {
    String ts = nf(hour(), 2) + ":" + nf(minute(), 2) + ":" + nf(second(), 2);
    TelemetryRecord rec = new TelemetryRecord(ts, stateCode, drivePWM, steerPWM);
    records.add(rec);
    appendToFile(rec);
  }

  /**
   * Escribe el registro al final del archivo sin borrar los datos anteriores.
   */
  private void appendToFile(TelemetryRecord rec) {
    try {
      FileWriter fw = new FileWriter(filePath, true); // true = Modo APPEND
      BufferedWriter bw = new BufferedWriter(fw);
      bw.write(rec.toFileString());
      bw.newLine();
      bw.close();
      fw.close();
    } catch (IOException e) {
      println("Error de escritura en archivo: " + e.getMessage());
    }
  }

  /**
   * Retorna el total de registros cargados en el contenedor dinámico.
   */
  int getTotalRecords() {
    return records.size();
  }

  /**
   * Retorna el último registro almacenado para mostrar en pantalla.
   */
  String getLastRecordSummary() {
    if (records.isEmpty()) return "Sin registros aun";
    return records.get(records.size() - 1).toFileString();
  }
}
