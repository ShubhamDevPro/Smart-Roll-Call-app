#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_netif.h" 
#include <vector>
#include "esp_pm.h"
#include "esp32/pm.h"

#define LED_BUILTIN 2

// For hotspot
const char* ssid = "Everyday I'm buffering";
const char* password = "area51project";
const int channel = 10;
const bool hide_SSID = false;
const int max_connection = 4;

// For connection to Home WiFi (for internet)
const char* wifi_network_ssid = "Bitto";
const char* wifi_network_password = "9278348561";

struct ConnectedDevice {
  uint8_t mac[6];
  esp_ip4_addr_t ip;
};

std::vector<ConnectedDevice> connectedDevices;

// To run the functions with different time delays
unsigned long lastDisplayTime = 0;
const int displayInterval = 5000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  
  Serial.println("\n[*] ESP32 Upload Successful!");
  Serial.println("[*] Initializing SmartRollCall System...");
  Serial.println("[*] Applying Power Optimizations...");
  
  // ============ POWER OPTIMIZATION SETTINGS ============
  
  // 1. Reduce CPU frequency to 80MHz (from default 240MHz)
  setCpuFrequencyMhz(80);
  Serial.print("[+] CPU Frequency reduced to: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");
  
  // 2. Enable WiFi Power Save Mode (MODEM sleep)
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  Serial.println("[+] WiFi Power Save Mode enabled");
  
  // 3. Reduce WiFi TX power (default is 20dBm, reducing to 15dBm)
  esp_wifi_set_max_tx_power(60); // 60 = 15dBm (value is in 0.25dBm units)
  Serial.println("[+] WiFi TX power reduced to 15dBm");
  
  Serial.println("[*] Creating AP");

  // *** IMPORTANT FIX: Set mode to AP + Station to run both simultaneously ***
  WiFi.mode(WIFI_AP_STA);

  // Configure and start the Access Point
  bool apStarted = WiFi.softAP(ssid, password, channel, hide_SSID, max_connection);
  
  if (apStarted) {
    Serial.println("[+] ✓ Hotspot created successfully!");
    Serial.print("[+] SSID: ");
    Serial.println(ssid);
    Serial.print("[+] AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("[+] Max Connections: ");
    Serial.println(max_connection);
  } else {
    Serial.println("[!] ✗ Failed to create hotspot!");
  }

  // Connect to the home Wi-Fi network
  Serial.print("[*] Connecting to home WiFi");
  WiFi.begin(wifi_network_ssid, wifi_network_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[+] ✓ Connected to home WiFi");
  Serial.print("[+] Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("[*] System ready - Monitoring for connected devices...");
  Serial.println("=================================================");
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
  // Function kept for potential future use
  // LED blinking removed - now handled in main loop
}

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
  // Permanent LED blink to indicate ESP32 is working
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);  // LED on for 500ms
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);  // LED off for 500ms
  
  check_if_esp32_is_connected_to_College_WiFi();

  unsigned long currentTime = millis();
  if (currentTime - lastDisplayTime >= displayInterval) {
    display_connected_devices();
    lastDisplayTime = currentTime;
  }
}