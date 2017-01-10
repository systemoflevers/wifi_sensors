#include <BME280I2C.h>
#include <Wire.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>

String str_mac;
int get_id() {
  byte mac[6];

  WiFi.macAddress(mac);

  str_mac += String(mac[5],HEX);
  str_mac += String(":");
  str_mac += String(mac[4],HEX);
  str_mac += String(":");
  str_mac += String(mac[3],HEX);
  str_mac += String(":");
  str_mac += String(mac[2],HEX);
  str_mac += String(":");
  str_mac += String(mac[1],HEX);
  str_mac += String(":");
  str_mac += String(mac[0],HEX);
  Serial.println();
  Serial.println("MAC: " + str_mac);

  return 0;
}

/**
 1 Temperature Oversampling Rate (tosr): uint8_t, default = 0x1
   values: 0x0 = Skipped, 0x1 = x1, 0x2 = x2, 0x3 = x4, 0x4 = x8, 0x5/other = x16
 2 Humidity Oversampling Rate (hosr): uint8_t, default = 0x1
   values: B000 = Skipped, B001 = x1, B010 = x2, B011 = x4, B100 = x8, B101/other = x16
 3 Pressure Oversampling Rate (posr): uint8_t, default = 0x1
   values: B000 = Skipped, B001 = x1, B010 = x2, B011 = x4, B100 = x8, B101/other = x16
 4 Mode: uint8_t, default = Normal
   values: Sleep = B00, Forced = B01 and B10, Normal = B11
 5 Standby Time (st): uint8_t, default = 1000ms
   values: 0x0 = 0.5ms, 0x1 = 62.5ms, 0x2 = 125ms, 0x3 = 250ms, 0x4 = 250ms, 0x5 = 1000ms, 0x6 = 10ms, 0x7 = 20ms
 6 Filter: uint8_t, default = None
   values: 0x0 = off, 0x1 = 2, 0x2 = 4, 0x3 = 8, 0x4/other = 16
 7 SPI Enable: bool, default = false
   values: true = enable, false = disable
 8 BME280 Address: uint8_t, default = 0x76
   values: any uint8_t
*/
BME280I2C bme(0x5, 0x5, 0x5, 0x3, 0x1, 0x4, false, 0x76);
ESP8266WiFiMulti wifiMulti;


void send(float temp, float hum, float pres) {
  String url = "Your URL";
  url = String(url + "s=" + str_mac);
  url = String(url + "&t=" + temp);
  url = String(url + "&h=" + hum);
  url = String(url + "&p=" + String(pres,10));

  Serial.print("url: ");
  Serial.println(url);
  Serial.println("[HTTP} URL: " + url);
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
}

void sense_and_send() {
  float temp(NAN), hum(NAN), pres(NAN);

  // unit: B000 = Pa, B001 = hPa, B010 = Hg, B011 = atm, B100 = bar,
  //   B101 = torr, B110 = N/m^2, B111 = psi
  uint8_t pressureUnit(7);
  bme.read(pres, temp, hum, true, pressureUnit);

  send(temp, hum, pres);
}

void setup(void) {
  Serial.begin(9600);

  get_id();

  while(!bme.begin()) {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }

  wifiMulti.addAP("YourAP"); // password too if needed

  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Not connected to WiFi.");
    delay(1000);
  }
}

void loop(void) {
  sense_and_send();
  delay(1*60000);
}
