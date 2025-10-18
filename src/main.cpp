#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "esp_wifi.h"
#include "esp_netif.h"
#include <vector>
#include "esp_pm.h"
#include "config.h" // Include configuration file with credentials

#define LED_BUILTIN 2

// ==================== STRUCTURES ====================

struct ConnectedDevice
{
  uint8_t mac[6];
  esp_ip4_addr_t ip;
  bool sentToServer;
};

struct PendingMacAddress
{
  String macAddress;
  unsigned long timestamp;
};

// ==================== GLOBAL VARIABLES ====================

std::vector<ConnectedDevice> connectedDevices;
std::vector<PendingMacAddress> pendingQueue; // Queue for MAC addresses to send

unsigned long lastDeviceCheck = 0;
unsigned long lastTimeSync = 0;
bool timeInitialized = false;

// ==================== FORWARD DECLARATIONS ====================

void applyPowerOptimizations();
void setupWiFi();
void initializeTime();
void printCurrentTime();
void printConfiguration();
bool isDeviceConnected(const uint8_t *mac);
String macToString(const uint8_t *mac);
void checkForNewDevices();
void cleanupDisconnectedDevices(esp_netif_sta_list_t *currentList);
void sendMacToServer(String macAddress);

// ==================== SETUP ====================

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("ESP32 Attendance System - MAC Only Mode");
  Serial.println("========================================");

  printConfiguration();
  applyPowerOptimizations();
  setupWiFi();

  Serial.println("[✓] System Ready - Monitoring Devices\n");
}

void applyPowerOptimizations()
{
  setCpuFrequencyMhz(80);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  esp_wifi_set_max_tx_power(60);
  Serial.println("[*] Power optimizations applied");
}

void setupWiFi()
{
  WiFi.mode(WIFI_AP_STA);

  if (WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, HIDE_SSID, MAX_CONNECTIONS))
  {
    Serial.printf("[✓] Hotspot: %s | IP: %s\n", AP_SSID, WiFi.softAPIP().toString().c_str());
  }
  else
  {
    Serial.println("[✗] Hotspot failed!");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 20)
  {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.printf("[✓] Internet: %s | IP: %s\n", WIFI_SSID, WiFi.localIP().toString().c_str());
  }
  else
  {
    Serial.println("[✗] Internet connection failed!");
  }
}

void printConfiguration()
{
  Serial.println("========================================");
  Serial.printf("Server: %s\n", SERVER_URL);
  Serial.println("========================================");
}

bool isDeviceConnected(const uint8_t *mac)
{
  for (const auto &device : connectedDevices)
  {
    if (memcmp(device.mac, mac, 6) == 0)
      return true;
  }
  return false;
}

String macToString(const uint8_t *mac)
{
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

void checkForNewDevices()
{
  wifi_sta_list_t wifi_sta_list;
  esp_netif_sta_list_t netif_sta_list;
  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
  memset(&netif_sta_list, 0, sizeof(netif_sta_list));

  esp_wifi_ap_get_sta_list(&wifi_sta_list);
  esp_netif_get_sta_list(&wifi_sta_list, &netif_sta_list);

  if (netif_sta_list.num > 0)
  {
    for (int i = 0; i < netif_sta_list.num; i++)
    {
      esp_netif_sta_info_t station = netif_sta_list.sta[i];

      if (!isDeviceConnected(station.mac))
      {
        ConnectedDevice device;
        memcpy(device.mac, station.mac, 6);
        device.ip = station.ip;
        device.sentToServer = false;
        connectedDevices.push_back(device);

        String macAddress = macToString(station.mac);
        Serial.printf("\n[NEW] %s\n", macAddress.c_str());

        PendingMacAddress pending;
        pending.macAddress = macAddress;
        pending.timestamp = millis();
        pendingQueue.push_back(pending);

        Serial.printf("[QUEUE] Size: %d\n", pendingQueue.size());
      }
    }
  }

  cleanupDisconnectedDevices(&netif_sta_list);
}

void cleanupDisconnectedDevices(esp_netif_sta_list_t *currentList)
{
  for (int i = connectedDevices.size() - 1; i >= 0; i--)
  {
    bool stillConnected = false;
    for (int j = 0; j < currentList->num; j++)
      if (memcmp(connectedDevices[i].mac, currentList->sta[j].mac, 6) == 0)
      {
        stillConnected = true;
        break;
      }

    if (!stillConnected)
    {
      Serial.printf("[DISC] %s\n", macToString(connectedDevices[i].mac).c_str());
      connectedDevices.erase(connectedDevices.begin() + i);
    }
  }
}

// ==================== SERVER COMMUNICATION ====================

void sendMacToServer(String macAddress)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[✗] No WiFi");
    return;
  }

  DynamicJsonDocument doc(128);
  doc["macAddress"] = macAddress;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  HTTPClient http;
  WiFiClient client;

  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  int httpCode = http.POST(jsonPayload);

  if (httpCode == 200)
  {
    Serial.printf("[✓] %s sent\n", macAddress.c_str());
    for (auto &device : connectedDevices)
      if (macToString(device.mac) == macAddress)
      {
        device.sentToServer = true;
        break;
      }
  }
  else
    Serial.printf("[✗] HTTP %d\n", httpCode);

  http.end();
}

void processPendingQueue()
{
  if (pendingQueue.empty() || WiFi.status() != WL_CONNECTED)
    return;

  PendingMacAddress pending = pendingQueue.front();
  sendMacToServer(pending.macAddress);
  pendingQueue.erase(pendingQueue.begin());

  if (!pendingQueue.empty())
    Serial.printf("[QUEUE] %d remaining\n", pendingQueue.size());
}

void loop()
{
  unsigned long currentMillis = millis();

  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[!] Reconnecting WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);
    return;
  }

  if (currentMillis - lastDeviceCheck >= DEVICE_CHECK_INTERVAL)
  {
    checkForNewDevices();
    lastDeviceCheck = currentMillis;
  }

  processPendingQueue();
  delay(100);
}
