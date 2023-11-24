/*
#include <WiFi.h>

const char *ssid = "Everyday Im buffering"; // Set your WiFi hotspot SSID
const char *password = "area51project"; // Set your WiFi hotspot password
int channel = 1; // Set your desired WiFi channel

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  // Set up the ESP32 as a WiFi access point
  WiFi.softAPConfig(WiFi.softAPIP(), WiFi.gatewayIP(), WiFi.subnetMask());
  WiFi.softAP(ssid, password, channel);
  Serial.println("ESP32 WiFi Hotspot Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(500);                     
  digitalWrite(LED_BUILTIN, LOW);   
  delay(500);   
}
*/
/*
#include <WiFi.h>

const char* ssid     = "upesyAP";
const char* password = "super_strong_password";

void setup()
{
    Serial.begin(115200);
    Serial.println("\n[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
}

void loop(){}
*/

#include <WiFi.h>
#include "esp_wifi.h"

const char* ssid           = "Everyday I'm buffering";               
const char* password       = "area51project";   
const int   channel        = 10;                        
const bool  hide_SSID      = false;                     
const int   max_connection = 20;       

const char* wifi_network_ssid     = "shubham_dev";
const char* wifi_network_password = "9278348561";                  

void display_connected_devices()
{
    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    if (adapter_sta_list.num > 0)
        Serial.println("-----------");
    for (uint8_t i = 0; i < adapter_sta_list.num; i++)
    {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        Serial.print((String)"[+] Device " + i + " | MAC : ");
        Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5]);
        // Change the type of the argument to const ip4_addr_t*
        Serial.println((String) " | IP " + ip4addr_ntoa((const ip4_addr_t*)&(station.ip)));
    }
    if(WiFi.status() == WL_CONNECTED){digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    //delay(500);
}
}

void setup()
{
    //FOR LED
    pinMode(LED_BUILTIN, OUTPUT);
    // Hotspot Code Below
    Serial.begin(115200);
    Serial.println("\n[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password, channel, hide_SSID, max_connection);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
    
    // to Connect to WiFi
    WiFi.begin(wifi_network_ssid, wifi_network_password);
    while (WiFi.status() != WL_CONNECTED) delay(100);
}

void loop()
{
    display_connected_devices();
    delay(5000);
}
