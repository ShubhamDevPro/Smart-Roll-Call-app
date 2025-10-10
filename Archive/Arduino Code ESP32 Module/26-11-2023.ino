#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_netif.h" // <-- ADD THIS NEW HEADER
#include <vector>

#define LED_BUILTIN 2

// For hotspot
const char* ssid = "Everyday I'm buffering";
const char* password = "area51project";
const int channel = 10;
const bool hide_SSID = false;
const int max_connection = 20;

// For connection to Home WiFi (for internet)
const char* wifi_network_ssid = "home_wifi";
const char* wifi_network_password = "password";

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

  // *** IMPORTANT FIX: Set mode to AP + Station to run both simultaneously ***
  WiFi.mode(WIFI_AP_STA);

  // Configure and start the Access Point
  WiFi.softAP(ssid, password, channel, hide_SSID, max_connection);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());

  // Connect to the home Wi-Fi network
  WiFi.begin(wifi_network_ssid, wifi_network_password);
  Serial.print("[*] Connecting to home WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[+] Connected to home WiFi");
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
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

// ======================================================================
// ========= THIS IS THE ENTIRELY REVISED FUNCTION =======================
// ======================================================================
void display_connected_devices() {
  wifi_sta_list_t wifi_sta_list;
  esp_netif_sta_list_t netif_sta_list; // <-- NEW: Use esp_netif_sta_list_t

  // Clear previous data
  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
  memset(&netif_sta_list, 0, sizeof(netif_sta_list));

  // Get the list of connected stations
  esp_wifi_ap_get_sta_list(&wifi_sta_list);
  esp_netif_get_sta_list(&wifi_sta_list, &netif_sta_list); // <-- NEW: Use esp_netif_get_sta_list

  if (netif_sta_list.num > 0) {
    bool newDeviceFound = false;
    for (int i = 0; i < netif_sta_list.num; i++) {
      esp_netif_sta_info_t station = netif_sta_list.sta[i]; // <-- NEW: Use esp_netif_sta_info_t

      if (!isDeviceConnected(station.mac) && connectedDevices.size() < max_connection) {
        if (!newDeviceFound) {
          Serial.println("-----------");
          newDeviceFound = true;
        }

        ConnectedDevice device;
        memcpy(device.mac, station.mac, 6);
        device.ip = station.ip; // IP is now directly accessible
        connectedDevices.push_back(device);

        Serial.print((String) "[+] New Device " + (i + 1) + " | MAC: ");
        Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5]);
        
        // Use the IP address from the new struct
        char ip_str[16];
        sprintf(ip_str, "%d.%d.%d.%d", IP2STR(&station.ip));
        Serial.println(" | IP: " + String(ip_str));
      }
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
