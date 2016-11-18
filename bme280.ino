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


BME280I2C bme;
ESP8266WiFiMulti wifiMulti;


void send(float temp, float hum, float pres) {
  String url = "/forms/d/e/1FAIpQLSdPAwU5wi7i_ZZMSlCb5Tdjq1O-l8PX9i3l4WFr88XEPP6HRA/formResponse?";
  url = String(url + "entry.1967258850=" + str_mac);
  url = String(url + "&entry.668574052=" + temp);
  url = String(url + "&entry.1595499712=" + hum);
  url = String(url + "&entry.1253295275=" + pres);

  Serial.print("url: ");
  Serial.println(url);

  String domain = "docs.google.com";
  if (!https.connect(domain.c_str(), 443)) {
    Serial.println("connection failed");
    return;
  }
  https.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + domain + "\r\n" +
              "User-Agent: DHTSensorESP8266\r\n" +
              "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (https.connected()) {
    String line = https.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      Serial.println(line);
      break;
    }
  }
  Serial.println("closing connection");
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

  wifiMulti.addAP("GoogleGuest"); // password too if needed
  wifiMulti.addAP("jessnet", "jesselovesjessica");
  wifiMulti.addAP("jessnet_tv_2", "jesselovesjessica");
  wifiMulti.addAP("jessnet-slow", "jesselovesjessica");

  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Not connected to WiFi.");
    delay(1000);
  }
}

void loop(void) {
  sense_and_send();
  delay(1*60000);
}
