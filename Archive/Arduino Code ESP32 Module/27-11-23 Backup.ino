#include <WiFi.h>
#include "esp_wifi.h"
#include <vector>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
/*
#include <WiFiClient.h>
#include <unordered_map>
#include <map>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <iterator>*/

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

/*
std::unordered_map<String, String> StudentRollNo;
std::unordered_map<String, String> StudentName;
*/
map<string, string> StudentRollNo;
map<string, string> StudentName;

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

/*
  // Inserting key-value pairs (Students List)
  StudentRollNo["76:2F:0E:C6:C3:5D"] = "01919011722";
  StudentName["76:2F:0E:C6:C3:5D"] = "Shubham Dev";
  //myMap["Bob"] = 30;
  //myMap["Charlie"] = 22;
  unordered_map<String, String> StudentRollNo = { {"76:2F:0E:C6:C3:5D", "01919011722"}, {"Mac2", "RollNo2"} };
unordered_map<String, String> StudentName = { {"76:2F:0E:C6:C3:5D", "Shubham Dev"}, {"Mac2", "Name2"} };

 // Insert some values into the map
    StudentRollNo["76:2F:0E:C6:C3:5D"] = "01919011722";
    StudentName["76:2F:0E:C6:C3:5D"] = "Shubham Dev";
    ["three"] = "Third";

#include <string.h>

// Define a structure to represent student information
struct Student {
    char macAddress[18];  // MAC address as a string
    char rollNo[12];      // Student roll number
    char name[50];        // Student name
};

int main() {
    // Create an array of structures to store student information
    struct Student students[1];  // You can adjust the array size based on your needs

    // Assign values to the structure
    strcpy(students[0].macAddress, "76:2F:0E:C6:C3:5D");
    strcpy(students[0].rollNo, "01919011722");
    strcpy(students[0].name, "Shubham Dev");

    // Access and print values
    printf("MAC Address: %s\n", students[0].macAddress);
    printf("Roll Number: %s\n", students[0].rollNo);
    printf("Student Name: %s\n", students[0].name);

    return 0;
}
*/

}

bool isDeviceConnected(const uint8_t* mac) {
  for (const auto& device : connectedDevices) {
    if (memcmp(device.mac, mac, 6) == 0) {
      return true;  // Device is already in the list
    }
  }
  return false;  // Device not found in the list
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




      // storing station.mac inside a variable
      char macAddressString[18];  // Allocate a char array to store the formatted MAC address (including null terminator)

      // Use sprintf to format the MAC address into a string
      sprintf(macAddressString, "%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5]);

      // Now, macAddressString contains the formatted MAC address as a string
      //Serial.println(macAddressString);



      // Declare a variable to store the IP address
ip4_addr_t ipAddress;

// Copy the IP address from station.ip to ipAddress
memcpy(&ipAddress, &station.ip, sizeof(ip4_addr_t));

// Now, ipAddress contains the IP address in the ip4_addr_t format

// If you want to convert it to a human-readable string, you can use ip4addr_ntoa
char ipAddressString[16]; // Allocate a char array to store the formatted IP address (including null terminator)
strcpy(ipAddressString, ip4addr_ntoa(&ipAddress));


      HTTPClient http;
      http.begin("https://maker.ifttt.com/trigger/Student/with/key/pv9O3xwW5wIdIa-IDDRb3GKVjoxbdZQ2NPHRd769fRb");  //Specify request destination
      http.addHeader("Content-Type", "application/json");                                                          //Specify content-type header

      StaticJsonDocument<200> jsonBuffer;
      JsonObject root = jsonBuffer.to<JsonObject>();
      root["value1"] = macAddressString;
      /*
      map<string, string>::iterator it1 = StudentName.find("76:2F:0E:C6:C3:5D");
      map<string, string>::iterator it2 = StudentRollNo.find("76:2F:0E:C6:C3:5D");
      */
      root["value2"] =  it1->second;
      root["value3"] =  it2->second;
      String json;
      serializeJson(root, json);

      int httpResponseCode = http.POST(json);  //Send the actual POST request

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      //Free resources
      http.end();
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
