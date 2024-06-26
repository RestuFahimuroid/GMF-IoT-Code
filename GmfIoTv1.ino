  #include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <WiFiUdp.h>
#include <NTPClient.h>


#define DHTPIN D2      // Pin untuk sensor DHT-11
#define DHTTYPE DHT11  // Tipe sensor DHT-11
#define MQ135PIN A0    // Pin untuk sensor MQ-135
#define LEDindicator D4

DHT dht(DHTPIN, DHTTYPE);

// SSID dan Password WiFi
const char* ssid = "KSI-STUDENT";
const char* password = "12344321";

//String URL = "http://192.168.100.160/projectksi/kirimdatasen2.php";
String URL = "http://is4ac.research-ai.my.id/public/gasdetection.php";

// Tentukan NTP Server yang ingin digunakan dan timezone offset
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;  // Waktu Indonesia Barat (WIB)
const int daylightOffset_sec = 0;

// Inisialisasi WiFi UDP dan NTPClient
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, 25200);

// Fungsi untuk menginisialisasi
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LEDindicator, OUTPUT);
  digitalWrite(LEDindicator, LOW);
  Serial.println("Connecting to WiFi...");
  // Koneksi ke WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

// Fungsi loop utama
void loop() {
  // Update waktu dari NTP server
  timeClient.update();

  // Dapatkan waktu saat ini dalam format timestamp
  unsigned long epochTime = timeClient.getEpochTime();

  //Baca data sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int mq135Value = analogRead(MQ135PIN);
  int ammoniaValue = mq135Value + 10;

  //ID ALAT
  String id_alat1 = "SU_1";
  String id_alat2 = "KN_1";
  String id_alat3 = "KA_5";
  String id_alat4 = "AM_3";


  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  //Kirim data ke server

  sendData(id_alat1, t);
  sendData(id_alat2, h);
  delay(10000);
  sendData(id_alat3, mq135Value);
  delay(10000);
  sendData(id_alat4, ammoniaValue);
  delay(10000);

    // String postData = (String) "temperature=" + t + "&humidity=" + h + "&co2=" + mq135Value + "&amonia=" + ammoniaValue;
    // WiFiClient client;
    // HTTPClient http;
    // http.begin(client, URL);
    // http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // auto httpCode = http.POST(postData);
    // String payload = http.getString();

    // Serial.println(postData);
    // Serial.println(payload);

    // blinkled();
    // http.end();
}

void sendData(String idalat, float nilaisensor) {
  // Update waktu dari NTP server
  timeClient.update();
  WiFiClient client;
  HTTPClient http;
  http.begin(client, URL);
  http.addHeader("Content-Type", "application/json");

  // Dapatkan waktu saat ini dalam format timestamp
  unsigned long epochTime = timeClient.getEpochTime();
  Serial.println(String(epochTime));
  
  String postData = (String) "id_alat=" + idalat + "&nilai=" + nilaisensor + String(epochTime);
  auto httpCode = http.POST(postData);
  String payload = http.getString();
  int httpResponseCode = http.POST(postData);

  Serial.println(postData);
  Serial.println(payload);

  if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

  blinkled();
  http.end();
}

void blinkled() {
  digitalWrite(LEDindicator, HIGH);
  delay(300);
  digitalWrite(LEDindicator, LOW);
  delay(10);
}