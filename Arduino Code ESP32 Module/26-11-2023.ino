#include <WiFi.h>
#include "esp_wifi.h"
#include <vector>

#define LED_BUILTIN 2  // Define LED_BUILTIN for ESP32

// For hotspot
const char* ssid = "Everyday I'm buffering";
const char* password = "area51project";
const int channel = 10;
const bool hide_SSID = false;
const int max_connection = 20;

// For connection to Home WiFi (for internet)
const char* wifi_network_ssid = "Bitto";
const char* wifi_network_password = "9278348561";

struct ConnectedDevice {
  uint8_t mac[6];
  ip4_addr_t ip;
};

std::vector<ConnectedDevice> connectedDevices;

// To run the functions with different time delays
unsigned long lastDisplayTime = 0;
const int displayInterval = 5000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password, channel, hide_SSID, max_connection);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());

  WiFi.begin(wifi_network_ssid, wifi_network_password);
  while (WiFi.status() != WL_CONNECTED) delay(100);
}

bool isDeviceConnected(const uint8_t* mac) {
  for (const auto& device : connectedDevices) {
    if (memcmp(device.mac, mac, 6) == 0) {
      return true; // Device is already in the list
    }
  }
  return false; // Device not found in the list
}

void check_if_esp32_is_connected_to_College_WiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

void display_connected_devices() {
  wifi_sta_list_t wifi_sta_list;
  tcpip_adapter_sta_list_t adapter_sta_list;
  esp_wifi_ap_get_sta_list(&wifi_sta_list);
  tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

  if (adapter_sta_list.num > 0)
    Serial.println("-----------");

  for (uint8_t i = 0; i < adapter_sta_list.num; i++) {
    tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
    if (!isDeviceConnected(station.mac) && connectedDevices.size() < max_connection) {
      ConnectedDevice device;
      memcpy(device.mac, station.mac, 6);
      device.ip = *reinterpret_cast<ip4_addr_t*>(&station.ip);
      connectedDevices.push_back(device);

      Serial.print((String) "[+] Device " + i + " | MAC : ");
      Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5]);
      Serial.println((String) " | IP " + ip4addr_ntoa(reinterpret_cast<const ip4_addr_t*>(&station.ip)));
    }
  }
}

void loop() {
  check_if_esp32_is_connected_to_College_WiFi();

  unsigned long currentTime = millis();
  if (currentTime - lastDisplayTime >= displayInterval) {
    display_connected_devices();
    lastDisplayTime = currentTime;
  }
}
