///////////////////ESP32 1//////////////////////
//            PNBP GMF CEMARAN GAS            //
//                ESP32 WROOM                 //
//                   2024                     //
//               HAK CIPTA KODE               //
//                                            //
///////////////////////////////////////////////////
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFiUdp.h>

#define DHTPIN 27       // Pin for the DHT-11 sensor
#define MQ135PIN 32     // Pin for the MQ-135 sensor (ADC1 channel)
#define MQ4PIN 33       // Metana
#define MQ137PIN 35     //CO2
#define LEDindicator 2  // Built-in LED for ESP32
#define LEDPIN_POWER 17
#define LEDPIN_WIFI 18
#define LEDPIN_KIRIMDATA 19


////////////// DHT CONFIG /////////////////////
// Uncomment the type of sensor in use:
#define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE DHT21  // DHT 21 (AM2301)

// SSID and WiFi password
const char* ssid = "KSI-WORKSHOP";
const char* password = "12344321";

// URL Server
String URL_temperature = "http://is4ac.research-ai.my.id/public/temperature_data.php";
String URL_metana = "http://is4ac.research-ai.my.id/public/metana_data.php";
String URL_humidity = "http://is4ac.research-ai.my.id/public/humidity_data.php";
String URL_dioksida = "http://is4ac.research-ai.my.id/public/dioksida_data.php";
String URL_amonia = "http://is4ac.research-ai.my.id/public/amonia_data.php";

// Local IP
String ip_local = "192.168.100.3";

// Local URL
String URL_temperature_local = "http://" + ip_local + "/is4ac_local/temperature_data.php";
String URL_metana_local = "http://" + ip_local + "/is4ac_local/metana_data.php";
String URL_humidity_local = "http://" + ip_local + "/is4ac_local/humidity_data.php";
String URL_dioksida_local = "http://" + ip_local + "/is4ac_local/dioksida_data.php";

///////////////////////////////////////////////////
//
//          DEVICE CONFIGURATION
//
//
//////////////////////////////////////////////////

int id_alat_iot = 3;              // For identifying each device
const int tipe_data = 3;          // all real = 1, all dummy = 2, mq137dummy = 3
const int metode_kirim_data = 1;  // Automatic = 1, Manual = 2
const int config_server = 1;      // Public = 1, Local = 2

// Type 1 in Serial to Trigger Manual Data Sending
///////////////////////////////////////////////////

int t;
int h;
int mq135Value;
int mq4Value;
int mq137Value;
int ulang;
DHT_Unified dht(DHTPIN, DHTTYPE);
//bool sudahJalan = false; // Variabel penanda
sensors_event_t event;



unsigned long previousMillis = 0;  // variable to store the last time
const long interval = 60000;       // interval time (in milliseconds)

// Function to initialize
void setup() {
  Serial.begin(115200);

  // Start Sensor
  dht.begin();  //StartDHT Sensor
  delay(5);

  //Pin Config IN or OUT
  pinMode(LEDindicator, OUTPUT);
  pinMode(LEDPIN_KIRIMDATA, OUTPUT);
  pinMode(LEDPIN_POWER, OUTPUT);
  pinMode(LEDPIN_WIFI, OUTPUT);
  pinMode(MQ135PIN, INPUT);
  delay(5);

  //Pin condition on first runing
  digitalWrite(LEDindicator, LOW);
  digitalWrite(LEDPIN_KIRIMDATA, LOW);
  digitalWrite(LEDPIN_POWER, LOW);
  digitalWrite(LEDPIN_WIFI, LOW);
  delay(5);
  digitalWrite(LEDPIN_POWER, HIGH);
  delay(5);

  //Connect to wifi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    analogWrite(LEDPIN_WIFI, 150);
    delay(200);
    analogWrite(LEDPIN_WIFI, 0);
  }
  Serial.println("WiFi connected");
  analogWrite(LEDPIN_WIFI, 150);
  delay(10);
  Serial.println(WiFi.localIP());
  tampilkan_konfigurasi();
  delay(10);

  cek_sensor_dht();
  delay(10);
  cek_sensor_mq();

  delay(5);
  getDataSensor();
  delay(5);
}

// Main loop function
void loop() {

  delay(10);
  sendDataSensor();
  delay(5);
}

void cek_sensor_mq() {
  updateSensorMQ135BeforeGet();
  delay(10);

  updateSensorMQ4BeforeGet();
  delay(10);
}

// Check dht sensor
void cek_sensor_dht() {
  //Get DHT TPYE
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  delay(10);
  dht.humidity().getSensor(&sensor);

  //Check Temperature Sensor
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  Serial.print(F("Temperature: "));
  Serial.print(event.temperature);
  Serial.println(F("°C"));
  delay(10);

  //Check Humidity Sensor
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  Serial.print(F("Humidity: "));
  Serial.print(event.relative_humidity);
  Serial.println(F("%"));

  delay(10);
}

// Show this device configuration, set on the top of code
void tampilkan_konfigurasi() {
  Serial.println("Current Device Configuration");

  if (tipe_data == 1 || tipe_data == 2) {
    if (tipe_data == 1) {
      Serial.println("Using Sensor Data");
    }
    if (tipe_data == 2) {
      Serial.println("Using Dummy Data");
    }
    if (tipe_data == 3) {
      Serial.println("DHT sensor data, ammonia, dioxide dummy data");
    }
  }

  if (metode_kirim_data == 1 || metode_kirim_data == 2) {
    if (metode_kirim_data == 1) {
      Serial.println("Data sent Automatically at regular intervals");
    }
    if (metode_kirim_data == 2) {
      Serial.println("Data sent manually using a trigger");
    }
  }

  if (config_server == 1 || config_server == 2) {
    if (config_server == 1) {
      Serial.println("Using Public Server");
    }
    if (config_server == 2) {
      Serial.println("Using Local Server");
    }
  }
  Serial.println(id_alat_iot);
  delay(5);
}

// function for send data on http url that set first on the top
void dataSensorToHTTP(int idalat, int nilaisensor, String URL) {
  getDataSensor();
  delay(20);
  String postData = (String) "id_alat=" + idalat + "&nilai=" + nilaisensor;

  WiFiClient client;
  HTTPClient http;
  http.begin(client, URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  auto httpCode = http.POST(postData);
  String payload = http.getString();
  Serial.println(URL);
  Serial.println(postData);
  Serial.println(payload);

  blinkled();
  http.end();
}

// for blinking led on indicator led
void blinkled() {
  analogWrite(LEDPIN_KIRIMDATA, 200);
  delay(300);
  analogWrite(LEDPIN_KIRIMDATA, 0);
  delay(10);
}

// Get data from al/ sensor, configure data tyoe first on top
// This funtion can generate dummy data for testing only

void updateSensorMQ137BeforeGet() {
  //MQ 135 Sensor Config
  // Konstanta untuk kalibrasi
  const float RLOAD = 20.0;        // Nilai resistor load (dalam KOhm)
  const float RZERO = 76.63;       // Nilai resistansi dalam udara bersih
  const float PARA = 116.6020682;  // Konstanta kalibrasi
  const float PARB = 2.769034857;  // Konstanta kalibrasi

  mq137Value = 0.75 * mq135Value + -0.635;

  Serial.print("NH3 | PPM: ");
  Serial.println(mq137Value);

  delay(10);  // Jeda 1 detik
}

void updateSensorMQ135BeforeGet() {
  //MQ 135 Sensor Config
  // Konstanta untuk kalibrasi
  const float RLOAD = 20.0;        // Nilai resistor load (dalam KOhm)
  const float RZERO = 76.63;       // Nilai resistansi dalam udara bersih
  const float PARA = 116.6020682;  // Konstanta kalibrasi
  const float PARB = 2.769034857;  // Konstanta kalibrasi

  // Membaca nilai analog dari sensor
  int analogValue = analogRead(MQ135PIN);

  // Mengonversi nilai ADC menjadi tegangan (0 - 3.3V untuk ESP32)
  float voltage = analogValue * (3.3 / 4095.0);

  // Menghitung resistansi sensor (RS) menggunakan tegangan yang dibaca
  float rs = (3.3 * RLOAD / voltage) - RLOAD;

  // Menghitung rasio resistansi RS/RZERO
  float ratio = rs / RZERO;

  // Menghitung PPM berdasarkan formula logaritma
  mq135Value = PARA * pow(ratio, -PARB);

  // Tampilkan hasil pembacaan ke Serial Monitor
  // Serial.print("Analog Value: ");
  // Serial.print(analogValue);
  // Serial.print(" | Voltage: ");
  // Serial.print(voltage);
  Serial.print("CO2 | PPM: ");
  Serial.println(mq135Value);

  delay(10);  // Jeda 1 detik
}

void updateSensorMQ4BeforeGet() {
  //MQ 4 Sensor Config
  // Konstanta untuk kalibrasi MQ-4 untuk metana
  const float RLOAD = 30.0;        // Nilai resistor load (dalam KOhm)
  const float RZERO = 76.63;       // Nilai resistansi dalam udara bersih (nilai ini bisa berbeda)
  const float PARA = 116.6020682;  // Konstanta kalibrasi untuk metana (contoh, sesuaikan dengan data)
  const float PARB = 2.769034857;  // Konstanta kalibrasi untuk metana (contoh, sesuaikan dengan data)

  int analogValue = analogRead(MQ4PIN);

  // Mengonversi nilai ADC menjadi tegangan (0 - 3.3V untuk ESP32)
  float voltage = analogValue * (3.3 / 4095.0);

  // Menghitung resistansi sensor (RS) menggunakan tegangan yang dibaca
  float rs = (3.3 * RLOAD / voltage) - RLOAD;

  // Menghitung rasio resistansi RS/RZERO
  float ratio = rs / RZERO;

  // Menghitung PPM metana berdasarkan formula logaritma
  mq4Value = PARA * pow(ratio, -PARB);

  // Tampilkan hasil pembacaan ke Serial Monitor
  // Serial.print("Analog Value: ");
  // Serial.print(analogValue);
  // Serial.print(" | Voltage: ");
  // Serial.print(voltage);
  Serial.print("Methane PPM: ");
  Serial.println(mq4Value);

  delay(10);  // Jeda 1 detik
}

void getDataSensor() {
  if (tipe_data == 1) {
    dht.temperature().getEvent(&event);
    t = event.temperature;
    delay(10);
    dht.humidity().getEvent(&event);
    h = event.relative_humidity;
    delay(10);

    updateSensorMQ135BeforeGet();
    updateSensorMQ4BeforeGet();
    delay(5);
  }
  if (tipe_data == 2) {

    t = random(10, 33);
    h = random(33, 44);
    mq135Value = random(10, 33);
    mq4Value = random(33, 44);
    mq137Value = random(33, 44);
    delay(5);
  }
  if (tipe_data == 3) {
    dht.temperature().getEvent(&event);
    t = event.temperature;
    delay(10);
    dht.humidity().getEvent(&event);
    h = event.relative_humidity;
    delay(10);

    updateSensorMQ135BeforeGet();
    updateSensorMQ4BeforeGet();
    mq137Value = 1;
    delay(5);
  }
  delay(10);
}

// This function is combination of some code to send data on server
void sendDataSensor() {
  if (config_server == 1) {
    if (metode_kirim_data == 1) {
      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval) {
        // save the last time
        previousMillis = currentMillis;
        dataSensorToHTTP(id_alat_iot, t, URL_temperature);
        delay(5);
        dataSensorToHTTP(id_alat_iot, h, URL_humidity);
        delay(5);
        dataSensorToHTTP(id_alat_iot, mq4Value, URL_metana);
        delay(5);
        dataSensorToHTTP(id_alat_iot, mq135Value, URL_dioksida);
        delay(5);
        dataSensorToHTTP(id_alat_iot, mq137Value, URL_amonia);
        delay(5);
        ulang = 1;
        delay(10);
      }
    }

    if (metode_kirim_data == 2) {
      if (Serial.available() > 0) {
        int data = Serial.parseInt();
        if (data == 1) {
          Serial.println("Sending data to server");
          dataSensorToHTTP(id_alat_iot, t, URL_temperature);
          delay(5);
          dataSensorToHTTP(id_alat_iot, h, URL_humidity);
          delay(5);
          dataSensorToHTTP(id_alat_iot, mq4Value, URL_metana);
          delay(5);
          dataSensorToHTTP(id_alat_iot, mq135Value, URL_dioksida);
          delay(5);
          dataSensorToHTTP(id_alat_iot, mq137Value, URL_amonia);
          delay(5);
          ulang = 1;
          delay(10);
        }
      }
    }
  }
  if (config_server == 2) {
    if (metode_kirim_data == 1) {
      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval) {
        // save the last time
        previousMillis = currentMillis;
        dataSensorToHTTP(id_alat_iot, t, URL_temperature);
        delay(5);
        dataSensorToHTTP(id_alat_iot, h, URL_humidity);
        delay(5);
        dataSensorToHTTP(id_alat_iot, mq4Value, URL_metana);
        delay(5);
        dataSensorToHTTP(id_alat_iot, mq135Value, URL_dioksida);
        delay(5);
        dataSensorToHTTP(id_alat_iot, mq137Value, URL_amonia);
        delay(5);
        ulang = 1;
        delay(10);
      }
    }

    if (metode_kirim_data == 2) {
      if (Serial.available() > 0) {
        int data = Serial.parseInt();
        if (data == 1) {
          dataSensorToHTTP(id_alat_iot, t, URL_temperature);
          delay(5);
          dataSensorToHTTP(id_alat_iot, h, URL_humidity);
          delay(5);
          dataSensorToHTTP(id_alat_iot, mq4Value, URL_metana);
          delay(5);
          dataSensorToHTTP(id_alat_iot, mq135Value, URL_dioksida);
          delay(5);
          dataSensorToHTTP(id_alat_iot, mq137Value, URL_amonia);
          delay(5);
          ulang = 1;
          delay(10);
        }
      }
    }
  }
}