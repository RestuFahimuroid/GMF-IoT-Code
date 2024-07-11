///////////////////ESP ------ 1////////////////////
//        MATCHING FUNG GMF CEMARAN GAS          //
//                    2024                       //
//               HAK CIPTA KODE                  //
//                                               //
///////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <WiFiUdp.h>


#define DHTPIN D2        // Pin untuk sensor DHT-11
#define DHTTYPE DHT11    // Tipe sensor DHT-11
#define MQ135PIN A0      // Pin untuk sensor MQ-135
#define LEDindicator D4  //Built in LED

DHT dht(DHTPIN, DHTTYPE);

// SSID dan Password WiFi
const char* ssid = "Rescom Jember";
const char* password = "kantorRESCOM@#123";

//URL Server
String URL_temperature = "http://is4ac.research-ai.my.id/public/temperture_data.php";
String URL_metana = "http://is4ac.research-ai.my.id/public/metana_data.php";
String URL_humidity = "http://is4ac.research-ai.my.id/public/humidity_data.php";
String URL_dioksida = "http://is4ac.research-ai.my.id/public/dioksida_data.php";

//IP Local
String ip_local = "192.168.100.3";

//URL Local
String URL_temperature_local = "http://" + ip_local + "/is4ac_local/temperature_data.php";
String URL_metana_local = "http://" + ip_local + "/is4ac_local/metana_data.php";
String URL_humidity_local = "http://" + ip_local + "/is4ac_local/humidity_data.php";
String URL_dioksida_local = "http://" + ip_local + "/is4ac_local/dioksida_data.php";

///////////////////////////////////////////////////
//
//          KONFIGURASI ALAT
//
//
//////////////////////////////////////////////////

int id_alat_iot = 1;              // Untuk identifikasi tiap alat
const int tipe_data = 1;          //real = 1, dummy = 2
const int metode_kirim_data = 1;  //Otomatis = 1, Manual = 2
const int config_server = 1;      //Public =1, Local =2

//Ketik 1 Pada Serial Untuk Trigger Kirim Data Manual
///////////////////////////////////////////////////

int t;
int h;
int mq135Value;
int dioksida;

unsigned long previousMillis = 0;  // variabel untuk menyimpan waktu terakhir
const long interval = 30000;       // interval waktu (dalam milidetik)

// Fungsi untuk menginisialisasi
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LEDindicator, OUTPUT);
  digitalWrite(LEDindicator, LOW);
  delay(5);
  Serial.println("Connecting to WiFi...");
  // Koneksi ke WiFi
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

// Fungsi loop utama
void loop() {
  //CEK SENSOR
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  delay(5);
  sendDataSensor();
  delay(5);
}

void tampilkan_konfigurasi() {
  Serial.println("Konfigurasi Alat Saat Ini");

  if (tipe_data == 1 || tipe_data == 2) {
    if (tipe_data == 1) {
      Serial.println("Data Menggunakan data Sensor");
    }
    if (tipe_data == 2) {
      Serial.println("Data Menggunakan data Dummy");
    }
  }

  if (metode_kirim_data == 1 || metode_kirim_data == 2) {
    if (metode_kirim_data == 1) {
      Serial.println("Data dikirim Secara Otomasi dengan interval waktu");
    }
    if (metode_kirim_data == 2) {
      Serial.println("Data dikirim secara manual menggunakan trigger");
    }
  }

  if (config_server == 1 || config_server == 2) {
    if (config_server == 1) {
      Serial.println("Menggunakan Server Public");
    }
    if (config_server == 2) {
      Serial.println("Menggunakan Server Local");
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
    dioksida = mq135Value + 10;
    delay(5);
  }
  if (tipe_data == 2) {
    h = random(20, 30);
    t = random(50, 40);
    mq135Value = random(10, 33);
    dioksida = random(33, 44);
    delay(5);
  }
  delay(10);
}

void sendDataSensor() {

  if (config_server == 1) {
    if (metode_kirim_data == 1) {
      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval) {
        // simpan waktu terakhir
        previousMillis = currentMillis;
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

    if (metode_kirim_data == 2) {
      if (Serial.available() > 0) {
        int data = Serial.parseInt();
        if (data == 1) {
          Serial.println("Proses kirim ke server");
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
        // simpan waktu terakhir
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
          Serial.println("Proses Kirim data Local");
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
