///////////////////ESP32 1//////////////////////
//            PNBP GMF CEMARAN GAS            //
//                ESP32 WROOM                 //
//                   2024                     //
//               HAK CIPTA KODE               //
//                                            //
///////////////////////////////////////////////////
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <WiFiUdp.h>

#define DHTPIN 27          // Pin for the DHT-11 sensor
#define DHTTYPE DHT21     // DHT-21 sensor type
#define MQ135PIN 32       // Pin for the MQ-135 sensor (ADC1 channel)
#define MQ4PIN 33
#define MQ137PIN 33
#define LEDindicator 2    // Built-in LED for ESP32
#define LEDPIN_GREEN 17
#define LEDPIN_RED 18
#define LEDPIN_YELLOW 19

DHT dht(DHTPIN, DHTTYPE);

// SSID and WiFi password
const char* ssid = "KSI-STUDENT";
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

int id_alat_iot = 2;              // For identifying each device
const int tipe_data = 3;          // real = 1, dummy = 2, ammonia&dioksida&metana dummy = 3
const int metode_kirim_data = 1;  // Automatic = 1, Manual = 2
const int config_server = 1;      // Public = 1, Local = 2

// Type 1 in Serial to Trigger Manual Data Sending
///////////////////////////////////////////////////

int t;
int h;
int mq135Value;
int dioksida;
int amonia;

unsigned long previousMillis = 0;  // variable to store the last time
const long interval = 60000;       // interval time (in milliseconds)

// Function to initialize
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LEDindicator, OUTPUT);
  digitalWrite(LEDindicator, LOW);
  delay(5);
  Serial.println("Connecting to WiFi...");
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
  tampilkan_konfigurasi();
  delay(5);
  getDataSensor();
  delay(5);
}

// Main loop function
void loop() {
  // CHECK SENSOR
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  delay(5);
  sendDataSensor();
  delay(5);
}

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

void blinkled() {
  digitalWrite(LEDindicator, HIGH);
  delay(300);
  digitalWrite(LEDindicator, LOW);
  delay(10);
}

void getDataSensor() {
  if (tipe_data == 1) {
    h = dht.readHumidity();
    t = dht.readTemperature();
    mq135Value = analogRead(MQ135PIN);
    dioksida = analogRead(MQ4PIN);
    dioksida = analogRead(MQ137PIN);
    delay(5);
  }
  if (tipe_data == 2) {
    h = random(20, 30);
    t = random(50, 40);
    mq135Value = random(10, 33);
    dioksida = random(33, 44);
    delay(5);
  }
  if (tipe_data == 3) {
    h = dht.readHumidity();
    t = dht.readTemperature();
    mq135Value = random(200, 400);
    dioksida = mq135Value + 10;
    amonia = random(200, 400);
    delay(5);
  }
  delay(10);
}

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
        dataSensorToHTTP(id_alat_iot, mq135Value, URL_metana);
        delay(5);
        dataSensorToHTTP(id_alat_iot, dioksida, URL_dioksida);
        delay(5);
        dataSensorToHTTP(id_alat_iot, dioksida, URL_amonia);
        delay(5);
      }
    }

    if (metode_kirim_data == 2) {
      if (Serial.available() > 0) {
        int data = Serial.parseInt();
        if (data == 1) {
          Serial.println("Sending data to server");
          dataSensorToHTTP(id_alat_iot, h, URL_temperature);
          delay(5);
          dataSensorToHTTP(id_alat_iot, t, URL_metana);
          delay(5);
          dataSensorToHTTP(id_alat_iot, mq135Value, URL_humidity);
          delay(5);
          dataSensorToHTTP(id_alat_iot, dioksida, URL_dioksida);
          delay(5);
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
        dataSensorToHTTP(id_alat_iot, h, URL_temperature_local);
        delay(5);
        dataSensorToHTTP(id_alat_iot, t, URL_metana_local);
        delay(5);
        dataSensorToHTTP(id_alat_iot, mq135Value, URL_humidity_local);
        delay(5);
        dataSensorToHTTP(id_alat_iot, dioksida, URL_dioksida_local);
        delay(5);
      }
    }

    if (metode_kirim_data == 2) {
      if (Serial.available() > 0) {
        int data = Serial.parseInt();
        if (data == 1) {
          Serial.println("Sending data to Local Server");
          dataSensorToHTTP(id_alat_iot, h, URL_temperature_local);
          delay(5);
          dataSensorToHTTP(id_alat_iot, t, URL_metana_local);
          delay(5);
          dataSensorToHTTP(id_alat_iot, mq135Value, URL_humidity_local);
          delay(5);
          dataSensorToHTTP(id_alat_iot, dioksida, URL_dioksida_local);
          delay(5);
        }
      }
    }
  }
}