/*
 * =====================================================
 * ESP32 Smart Roll Call - Automatic Attendance System
 * =====================================================
 * 
 * Features:
 * - Creates WiFi hotspot for students to connect
 * - Connects to home WiFi for internet access
 * - Syncs time from NTP server
 * - Queries Firestore for class schedules
 * - Automatically marks attendance based on MAC address
 * - Only marks attendance if there's a scheduled class
 * - Power optimized for long-term operation
 * 
 * SETUP INSTRUCTIONS:
 * 1. Configure WiFi credentials (WIFI_SSID and WIFI_PASSWORD)
 * 2. Configure hotspot settings (AP_SSID and AP_PASSWORD)
 * 3. IMPORTANT: Set FIREBASE_USER_ID to your actual Firebase Auth User ID
 *    
 *    How to find your Firebase User ID:
 *    Method 1 (Firebase Console):
 *      - Open Firebase Console (console.firebase.google.com)
 *      - Go to Authentication -> Users
 *      - Find your logged-in user
 *      - Copy the "User UID" column value
 *    
 *    Method 2 (Flutter App):
 *      - In your Flutter app, after login, print:
 *        FirebaseAuth.instance.currentUser?.uid
 *      - Copy the printed UID
 *    
 *    The User ID looks like: "a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6"
 * 
 * 4. Upload this code to ESP32
 * 5. Monitor Serial output to verify schedule loading
 * 
 * Troubleshooting:
 * - If "No classes scheduled" appears but you have schedules:
 *   -> Check FIREBASE_USER_ID is correct
 *   -> Verify schedules exist in Firestore for today's day of week
 *   -> Check schedules are marked as "isActive: true"
 * - If students not found:
 *   -> Ensure students have registered MAC addresses in Flutter app
 * 
 * Author: Smart Roll Call System
 * Date: October 2025
 * =====================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "esp_wifi.h"
#include "esp_netif.h"
#include <vector>
#include "esp_pm.h"

#define LED_BUILTIN 2

// ==================== CONFIGURATION ====================

/*
 * FIREBASE CONFIGURATION
 * 
 * The ESP32 needs to query your Firestore database to:
 * 1. Load class schedules for today
 * 2. Find students by MAC address
 * 3. Mark attendance records
 * 
 * Your Firestore structure is:
 * users/{userId}/batches/{batchId}/
 *   - schedules/{scheduleId}      <- Class time schedules
 *   - students/{studentId}         <- Student info with MAC addresses
 * attendance_records/{recordId}    <- Where attendance is stored
 * 
 * The FIREBASE_USER_ID must match the userId in your Firestore path!
 */

// Firebase Configuration
const char* FIREBASE_PROJECT_ID = "smart-roll-call-76a46";
const char* FIREBASE_API_KEY = "AIzaSyB_MD4YFnfHyuXIUFMBCPl9M6-u1OIYK5g";

// ⚠️ CRITICAL: Replace "YOUR_USER_ID_HERE" with your actual Firebase Auth User ID
// 
// How to find your User ID:
// Method 1 - Firebase Console:
//   1. Go to https://console.firebase.google.com
//   2. Select your project "smart-roll-call-76a46"
//   3. Navigate to Authentication -> Users
//   4. Copy the "User UID" (looks like: a1b2c3d4e5f6g7h8...)
//
// Method 2 - Flutter App Debug:
//   1. In your Flutter app, add this after login:
//      print('User ID: ${FirebaseAuth.instance.currentUser?.uid}');
//   2. Run the app and copy the printed UID from console
//
// Method 3 - Check Firestore:
//   1. Go to Firestore Database in Firebase Console
//   2. Look under the "users" collection
//   3. The document ID is your User ID
//
const char* FIREBASE_USER_ID = "gIi2otSEGRU8KVbuNt5lO85ckPs2";

// Hotspot Configuration
const char* AP_SSID = "Everyday I'm buffering";
const char* AP_PASSWORD = "area51project";
const int AP_CHANNEL = 10;
const bool HIDE_SSID = false;
const int MAX_CONNECTIONS = 4;

// Home WiFi Configuration (for internet access)
const char* WIFI_SSID = "Bitto";
const char* WIFI_PASSWORD = "9278348561";

// NTP Server Configuration for time sync
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 19800;  // GMT+5:30 for India (5.5 hours * 3600)
const int DAYLIGHT_OFFSET_SEC = 0;

// Timing Configuration
const unsigned long DEVICE_CHECK_INTERVAL = 5000;    // Check for new devices every 5 seconds
const unsigned long SCHEDULE_REFRESH_INTERVAL = 300000; // Refresh schedules every 5 minutes
const unsigned long TIME_SYNC_INTERVAL = 3600000;    // Sync time every hour

// ==================== STRUCTURES ====================

struct ConnectedDevice {
  uint8_t mac[6];
  esp_ip4_addr_t ip;
  bool attendanceMarked;
};

struct ClassSchedule {
  String scheduleId;
  String batchId;
  String dayOfWeek;
  String startTime;
  String endTime;
  bool isActive;
};

// ==================== GLOBAL VARIABLES ====================

std::vector<ConnectedDevice> connectedDevices;
std::vector<ClassSchedule> todaysSchedules;

unsigned long lastDeviceCheck = 0;
unsigned long lastScheduleRefresh = 0;
unsigned long lastTimeSync = 0;
bool timeInitialized = false;
bool schedulesLoaded = false;

// ==================== FORWARD DECLARATIONS ====================

void applyPowerOptimizations();
void setupWiFi();
void initializeTime();
void printCurrentTime();
String getCurrentDayName();
String getCurrentTime24();
void printConfiguration();
void loadTodaysSchedules();
void loadSchedulesForBatch(String batchId, String dayOfWeek);
void printTodaysSchedules();
ClassSchedule* getCurrentSchedule();
bool isDeviceConnected(const uint8_t* mac);
String macToString(const uint8_t* mac);
void checkForNewDevices();
void cleanupDisconnectedDevices(esp_netif_sta_list_t* currentList);
void processDeviceAttendance(String macAddress);
String findStudentByMacAddress(String macAddress, String batchId);
bool markAttendance(String studentId, ClassSchedule* schedule, String macAddress);

// ==================== SETUP ====================

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("ESP32 Smart Roll Call System v2.0");
  Serial.println("========================================");
  Serial.println("[*] Initializing...");
  
  // Print configuration for debugging
  printConfiguration();
  
  // Apply power optimizations
  applyPowerOptimizations();
  
  // Setup WiFi (both AP and Station mode)
  setupWiFi();
  
  // Initialize time from NTP server
  initializeTime();
  
  // Load today's class schedules
  loadTodaysSchedules();
  
  Serial.println("\n========================================");
  Serial.println("[✓] System Ready - Monitoring Devices");
  Serial.println("========================================\n");
}

// ==================== POWER OPTIMIZATION ====================

void applyPowerOptimizations() {
  Serial.println("[*] Applying power optimizations...");
  
  // Reduce CPU frequency to 80MHz
  setCpuFrequencyMhz(80);
  Serial.printf("  [+] CPU frequency: %d MHz\n", getCpuFrequencyMhz());
  
  // Enable WiFi power save mode
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  Serial.println("  [+] WiFi power save enabled");
  
  // Reduce WiFi TX power to 15dBm
  esp_wifi_set_max_tx_power(60);
  Serial.println("  [+] WiFi TX power: 15dBm");
}

// ==================== WIFI SETUP ====================

void setupWiFi() {
  Serial.println("\n[*] Setting up WiFi...");
  
  // Set WiFi mode to AP + Station
  WiFi.mode(WIFI_AP_STA);
  
  // Start Access Point
  Serial.println("  [*] Creating hotspot...");
  bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, HIDE_SSID, MAX_CONNECTIONS);
  
  if (apStarted) {
    Serial.println("  [✓] Hotspot created successfully!");
    Serial.printf("    SSID: %s\n", AP_SSID);
    Serial.printf("    IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.printf("    Max connections: %d\n", MAX_CONNECTIONS);
  } else {
    Serial.println("  [✗] Failed to create hotspot!");
  }
  
  // Connect to home WiFi
  Serial.printf("  [*] Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n  [✓] Connected to home WiFi");
    Serial.printf("    Station IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n  [✗] Failed to connect to home WiFi!");
  }
}

// ==================== TIME MANAGEMENT ====================

void initializeTime() {
  Serial.println("\n[*] Initializing time from NTP server...");
  
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 10) {
    Serial.print("  .");
    delay(1000);
    attempts++;
  }
  
  if (getLocalTime(&timeinfo)) {
    timeInitialized = true;
    Serial.println("\n  [✓] Time synchronized successfully");
    printCurrentTime();
  } else {
    Serial.println("\n  [✗] Failed to obtain time from NTP server");
  }
}

void printCurrentTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "    Current time: %A, %B %d, %Y %H:%M:%S", &timeinfo);
    Serial.println(timeStr);
  }
}

void printConfiguration() {
  // Print configuration status to help with debugging
  Serial.println("\n========================================");
  Serial.println("CONFIGURATION STATUS");
  Serial.println("========================================");
  Serial.printf("Firebase Project: %s\n", FIREBASE_PROJECT_ID);
  Serial.printf("Firebase User ID: %s\n", FIREBASE_USER_ID);
  
  if (String(FIREBASE_USER_ID) == "YOUR_USER_ID_HERE") {
    Serial.println("⚠️  WARNING: User ID not configured!");
    Serial.println("    This will prevent loading schedules.");
  } else {
    Serial.println("✓  User ID configured");
  }
  
  Serial.printf("\nHotspot SSID: %s\n", AP_SSID);
  Serial.printf("Home WiFi: %s\n", WIFI_SSID);
  Serial.printf("Timezone: GMT+5:30 (India)\n");
  Serial.println("========================================\n");
}

String getCurrentDayName() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "";
  
  const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  return String(days[timeinfo.tm_wday]);
}

String getCurrentTime24() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "";
  
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  return String(timeStr);
}

// ==================== SCHEDULE MANAGEMENT ====================

void loadTodaysSchedules() {
  Serial.println("\n[*] Loading today's class schedules...");
  
  // Validation checks
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("  [✗] No internet connection - cannot load schedules");
    return;
  }
  
  if (!timeInitialized) {
    Serial.println("  [✗] Time not initialized - cannot determine current day");
    return;
  }
  
  // Check if USER_ID is configured
  if (String(FIREBASE_USER_ID) == "YOUR_USER_ID_HERE") {
    Serial.println("  [✗] FIREBASE_USER_ID not configured!");
    Serial.println("  [!] Please update FIREBASE_USER_ID in the code with your actual Firebase Auth User ID");
    Serial.println("  [!] Find it in: Firebase Console -> Authentication -> Users -> User UID");
    return;
  }
  
  String currentDay = getCurrentDayName();
  Serial.printf("  [*] Today is: %s\n", currentDay.c_str());
  
  // Clear existing schedules before loading new ones
  todaysSchedules.clear();
  
  // Construct URL to get all batches for the current user
  String batchesUrl = "https://firestore.googleapis.com/v1/projects/" + 
                      String(FIREBASE_PROJECT_ID) + 
                      "/databases/(default)/documents/users/" + 
                      String(FIREBASE_USER_ID) + "/batches?key=" + 
                      String(FIREBASE_API_KEY);
  
  Serial.println("  [*] Fetching batches from Firestore...");
  
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure(); // Skip SSL verification (use proper certificate in production)
  
  http.begin(client, batchesUrl);
  http.setTimeout(10000); // 10 second timeout
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.printf("  [✗] JSON parse error: %s\n", error.c_str());
      http.end();
      return;
    }
    
    // Check if the response contains documents
    if (!doc.containsKey("documents")) {
      Serial.println("  [!] No batches found for this user");
      Serial.println("  [!] Possible reasons:");
      Serial.println("      - FIREBASE_USER_ID is incorrect");
      Serial.println("      - No batches created in the Flutter app yet");
      Serial.println("      - User ID doesn't match any data in Firestore");
      http.end();
      return;
    }
    
    JsonArray batches = doc["documents"].as<JsonArray>();
    Serial.printf("  [✓] Found %d batch(es) for this user\n", batches.size());
    
    // Iterate through each batch and load its schedules
    for (JsonObject batch : batches) {
      String batchPath = batch["name"].as<String>();
      String batchId = batchPath.substring(batchPath.lastIndexOf('/') + 1);
      
      // Get batch name for better logging
      String batchName = "Unknown";
      if (batch["fields"].containsKey("batchName")) {
        batchName = batch["fields"]["batchName"]["stringValue"].as<String>();
      }
      
      Serial.printf("  [*] Checking batch: %s (ID: %s)\n", 
                    batchName.c_str(), batchId.c_str());
      
      // Get schedules for this batch
      loadSchedulesForBatch(batchId, currentDay);
    }
    
    Serial.printf("\n  [✓] Loaded %d schedule(s) for today (%s)\n", 
                  todaysSchedules.size(), currentDay.c_str());
    printTodaysSchedules();
    schedulesLoaded = true;
    
  } else if (httpCode == 404) {
    Serial.println("  [✗] User not found (HTTP 404)");
    Serial.println("  [!] Check if FIREBASE_USER_ID is correct");
  } else if (httpCode == 401 || httpCode == 403) {
    Serial.printf("  [✗] Authentication/Permission error (HTTP %d)\n", httpCode);
    Serial.println("  [!] Check Firestore security rules");
    Serial.println("  [!] Check if API key is valid");
  } else if (httpCode < 0) {
    Serial.printf("  [✗] Connection error (code: %d)\n", httpCode);
    Serial.println("  [!] Check internet connection");
  } else {
    Serial.printf("  [✗] Failed to fetch batches (HTTP %d)\n", httpCode);
    String response = http.getString();
    Serial.println("  [✗] Response: " + response);
  }
  
  http.end();
}

void loadSchedulesForBatch(String batchId, String dayOfWeek) {
  // Construct URL to fetch schedules subcollection for this batch
  String schedulesUrl = "https://firestore.googleapis.com/v1/projects/" + 
                        String(FIREBASE_PROJECT_ID) + 
                        "/databases/(default)/documents/users/" + 
                        String(FIREBASE_USER_ID) + "/batches/" + 
                        batchId + "/schedules?key=" + 
                        String(FIREBASE_API_KEY);
  
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure(); // Skip SSL certificate validation (use proper cert in production)
  
  http.begin(client, schedulesUrl);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    // Debug: Print raw response if no documents found (uncomment for debugging)
    // Serial.println("  [DEBUG] Raw schedules response: " + payload);
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.printf("  [✗] JSON parse error for batch %s: %s\n", 
                    batchId.c_str(), error.c_str());
      http.end();
      return;
    }
    
    // Check if documents array exists in response
    if (!doc.containsKey("documents")) {
      Serial.printf("  [!] No schedules found for batch %s\n", batchId.c_str());
      http.end();
      return;
    }
    
    JsonArray schedules = doc["documents"].as<JsonArray>();
    Serial.printf("  [*] Found %d total schedule(s) in batch %s\n", 
                  schedules.size(), batchId.c_str());
    
    for (JsonObject schedule : schedules) {
      String schedulePath = schedule["name"].as<String>();
      String scheduleId = schedulePath.substring(schedulePath.lastIndexOf('/') + 1);
      
      JsonObject fields = schedule["fields"];
      
      // Safety check: ensure required fields exist
      if (!fields.containsKey("dayOfWeek") || 
          !fields.containsKey("startTime") || 
          !fields.containsKey("endTime")) {
        Serial.printf("  [!] Schedule %s missing required fields, skipping\n", 
                      scheduleId.c_str());
        continue;
      }
      
      String schedDay = fields["dayOfWeek"]["stringValue"].as<String>();
      bool isActive = fields.containsKey("isActive") ? 
                      fields["isActive"]["booleanValue"].as<bool>() : true;
      
      Serial.printf("    - Schedule: %s (Day: %s, Active: %s)\n", 
                    scheduleId.c_str(), 
                    schedDay.c_str(), 
                    isActive ? "Yes" : "No");
      
      // Only add if it's for today and is active
      if (schedDay == dayOfWeek && isActive) {
        ClassSchedule classSchedule;
        classSchedule.scheduleId = scheduleId;
        classSchedule.batchId = batchId;
        classSchedule.dayOfWeek = schedDay;
        classSchedule.startTime = fields["startTime"]["stringValue"].as<String>();
        classSchedule.endTime = fields["endTime"]["stringValue"].as<String>();
        classSchedule.isActive = isActive;
        
        todaysSchedules.push_back(classSchedule);
        Serial.printf("    ✓ Added: %s - %s\n", 
                      classSchedule.startTime.c_str(), 
                      classSchedule.endTime.c_str());
      }
    }
  } else if (httpCode == 404) {
    // 404 means the schedules subcollection doesn't exist for this batch
    Serial.printf("  [!] No schedules subcollection for batch %s (HTTP 404)\n", 
                  batchId.c_str());
  } else {
    Serial.printf("  [✗] Failed to fetch schedules for batch %s (HTTP %d)\n", 
                  batchId.c_str(), httpCode);
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("  [✗] Response: " + response);
    }
  }
  
  http.end();
}

void printTodaysSchedules() {
  if (todaysSchedules.size() == 0) {
    Serial.println("\n  ========================================");
    Serial.println("  [!] No classes scheduled for today");
    Serial.println("  ========================================");
    Serial.println("  Possible reasons:");
    Serial.println("  - No schedules match today's day of week");
    Serial.println("  - Schedules exist but are marked inactive");
    Serial.println("  - Schedules not yet created in Flutter app");
    Serial.println("  ========================================\n");
    return;
  }
  
  Serial.println("\n  ========================================");
  Serial.println("  === Today's Class Schedule ===");
  Serial.println("  ========================================");
  for (int i = 0; i < todaysSchedules.size(); i++) {
    ClassSchedule& schedule = todaysSchedules[i];
    Serial.printf("  %d. %s - %s\n", 
                  i + 1, 
                  schedule.startTime.c_str(), 
                  schedule.endTime.c_str());
    Serial.printf("     Batch: %s\n", schedule.batchId.c_str());
    Serial.printf("     Schedule ID: %s\n", schedule.scheduleId.c_str());
  }
  Serial.println("  ========================================\n");
}

ClassSchedule* getCurrentSchedule() {
  String currentTime = getCurrentTime24();
  
  // Check each schedule to see if current time falls within its time range
  for (int i = 0; i < todaysSchedules.size(); i++) {
    ClassSchedule& schedule = todaysSchedules[i];
    
    // String comparison works for time in HH:MM format (e.g., "19:00" <= "20:04" <= "23:00")
    if (currentTime >= schedule.startTime && currentTime <= schedule.endTime) {
      return &schedule;
    }
  }
  
  // No class currently in session
  return nullptr;
}

// ==================== DEVICE MANAGEMENT ====================

bool isDeviceConnected(const uint8_t* mac) {
  for (const auto& device : connectedDevices) {
    if (memcmp(device.mac, mac, 6) == 0) {
      return true;
    }
  }
  return false;
}

String macToString(const uint8_t* mac) {
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

void checkForNewDevices() {
  wifi_sta_list_t wifi_sta_list;
  esp_netif_sta_list_t netif_sta_list;

  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
  memset(&netif_sta_list, 0, sizeof(netif_sta_list));

  esp_wifi_ap_get_sta_list(&wifi_sta_list);
  esp_netif_get_sta_list(&wifi_sta_list, &netif_sta_list);

  if (netif_sta_list.num > 0) {
    for (int i = 0; i < netif_sta_list.num; i++) {
      esp_netif_sta_info_t station = netif_sta_list.sta[i];

      if (!isDeviceConnected(station.mac)) {
        // New device detected
        ConnectedDevice device;
        memcpy(device.mac, station.mac, 6);
        device.ip = station.ip;
        device.attendanceMarked = false;
        connectedDevices.push_back(device);

        String macAddress = macToString(station.mac);
        char ip_str[16];
        sprintf(ip_str, "%d.%d.%d.%d", IP2STR(&station.ip));
        
        Serial.println("\n========================================");
        Serial.printf("[NEW DEVICE] MAC: %s | IP: %s\n", macAddress.c_str(), ip_str);
        Serial.println("========================================");
        
        // Try to mark attendance for this device
        processDeviceAttendance(macAddress);
      }
    }
  }
  
  // Clean up disconnected devices
  cleanupDisconnectedDevices(&netif_sta_list);
}

void cleanupDisconnectedDevices(esp_netif_sta_list_t* currentList) {
  for (int i = connectedDevices.size() - 1; i >= 0; i--) {
    bool stillConnected = false;
    
    for (int j = 0; j < currentList->num; j++) {
      if (memcmp(connectedDevices[i].mac, currentList->sta[j].mac, 6) == 0) {
        stillConnected = true;
        break;
      }
    }
    
    if (!stillConnected) {
      String macAddress = macToString(connectedDevices[i].mac);
      Serial.printf("[DISCONNECTED] Device: %s\n", macAddress.c_str());
      connectedDevices.erase(connectedDevices.begin() + i);
    }
  }
}

// ==================== ATTENDANCE PROCESSING ====================

void processDeviceAttendance(String macAddress) {
  // Check if there's a class in session
  ClassSchedule* currentSchedule = getCurrentSchedule();
  
  if (currentSchedule == nullptr) {
    Serial.println("[!] No class currently in session - Attendance not marked");
    return;
  }
  
  Serial.printf("[*] Class in session: %s - %s (Batch: %s)\n", 
                currentSchedule->startTime.c_str(), 
                currentSchedule->endTime.c_str(),
                currentSchedule->batchId.c_str());
  
  // Find student by MAC address in the current batch
  String studentId = findStudentByMacAddress(macAddress, currentSchedule->batchId);
  
  if (studentId.isEmpty()) {
    Serial.printf("[!] No student found with MAC %s in batch %s\n", 
                  macAddress.c_str(), 
                  currentSchedule->batchId.c_str());
    return;
  }
  
  // Mark attendance
  bool success = markAttendance(studentId, currentSchedule, macAddress);
  
  if (success) {
    Serial.println("[✓] Attendance marked successfully!");
    
    // Mark device as processed
    for (auto& device : connectedDevices) {
      if (macToString(device.mac) == macAddress) {
        device.attendanceMarked = true;
        break;
      }
    }
  } else {
    Serial.println("[✗] Failed to mark attendance");
  }
}

String findStudentByMacAddress(String macAddress, String batchId) {
  Serial.printf("[*] Searching for student with MAC %s in batch %s...\n", 
                macAddress.c_str(), batchId.c_str());
  
  // Construct URL to fetch all students in this batch
  String studentsUrl = "https://firestore.googleapis.com/v1/projects/" + 
                       String(FIREBASE_PROJECT_ID) + 
                       "/databases/(default)/documents/users/" + 
                       String(FIREBASE_USER_ID) + "/batches/" + 
                       batchId + "/students?key=" + 
                       String(FIREBASE_API_KEY);
  
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();
  
  http.begin(client, studentsUrl);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.printf("[✗] JSON parse error: %s\n", error.c_str());
      http.end();
      return "";
    }
    
    // Check if documents exist
    if (!doc.containsKey("documents")) {
      Serial.println("[!] No students found in this batch");
      http.end();
      return "";
    }
    
    JsonArray students = doc["documents"].as<JsonArray>();
    Serial.printf("[*] Checking %d student(s) in batch...\n", students.size());
    
    // Search for student with matching MAC address
    for (JsonObject student : students) {
      JsonObject fields = student["fields"];
      
      // Check if student has a MAC address registered
      if (fields.containsKey("macAddress")) {
        String studentMac = fields["macAddress"]["stringValue"].as<String>();
        
        // Case-insensitive MAC address comparison
        if (studentMac.equalsIgnoreCase(macAddress)) {
          String studentPath = student["name"].as<String>();
          String studentId = studentPath.substring(studentPath.lastIndexOf('/') + 1);
          String studentName = fields["name"]["stringValue"].as<String>();
          String enrollNumber = fields["enrollNumber"]["stringValue"].as<String>();
          
          Serial.printf("[✓] Student found: %s (%s)\n", 
                        studentName.c_str(), enrollNumber.c_str());
          http.end();
          return studentId;
        }
      }
    }
    
    // No matching student found
    Serial.println("[!] No student with matching MAC address found");
    Serial.println("[!] Make sure student has registered their device MAC in the Flutter app");
    
  } else if (httpCode == 404) {
    Serial.println("[✗] Students collection not found (HTTP 404)");
  } else {
    Serial.printf("[✗] Failed to fetch students (HTTP %d)\n", httpCode);
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("[✗] Response: " + response);
    }
  }
  
  http.end();
  return "";
}

bool markAttendance(String studentId, ClassSchedule* schedule, String macAddress) {
  Serial.println("[*] Marking attendance in Firestore...");
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[✗] Failed to get current time");
    return false;
  }
  
  // Create date string (YYYY-MM-DD format)
  char dateStr[11];
  sprintf(dateStr, "%04d-%02d-%02d", 
          timeinfo.tm_year + 1900, 
          timeinfo.tm_mon + 1, 
          timeinfo.tm_mday);
  
  // Create ISO 8601 timestamp for Firestore
  char timestampStr[30];
  sprintf(timestampStr, "%04d-%02d-%02dT%02d:%02d:%02dZ", 
          timeinfo.tm_year + 1900, 
          timeinfo.tm_mon + 1, 
          timeinfo.tm_mday,
          timeinfo.tm_hour, 
          timeinfo.tm_min, 
          timeinfo.tm_sec);
  
  Serial.printf("  Date: %s\n", dateStr);
  Serial.printf("  Timestamp: %s\n", timestampStr);
  
  // Step 1: Check if attendance already exists for this student+schedule+date
  // This prevents duplicate attendance records
  String checkUrl = "https://firestore.googleapis.com/v1/projects/" + 
                    String(FIREBASE_PROJECT_ID) + 
                    "/databases/(default)/documents/attendance_records?key=" + 
                    String(FIREBASE_API_KEY);
  
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();
  
  Serial.println("  [*] Checking for existing attendance record...");
  http.begin(client, checkUrl);
  int checkCode = http.GET();
  
  if (checkCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument checkDoc(8192);
    DeserializationError error = deserializeJson(checkDoc, payload);
    
    if (!error && checkDoc.containsKey("documents")) {
      JsonArray records = checkDoc["documents"].as<JsonArray>();
      
      // Check if attendance already marked for this student, schedule, and date
      for (JsonObject record : records) {
        if (!record.containsKey("fields")) continue;
        
        JsonObject fields = record["fields"];
        
        // Safely extract field values
        String recStudentId = fields.containsKey("studentId") ? 
                              fields["studentId"]["stringValue"].as<String>() : "";
        String recScheduleId = fields.containsKey("scheduleId") ? 
                               fields["scheduleId"]["stringValue"].as<String>() : "";
        String recDate = fields.containsKey("date") ? 
                        fields["date"]["timestampValue"].as<String>() : "";
        
        // Check for match
        if (recStudentId == studentId && 
            recScheduleId == schedule->scheduleId && 
            recDate.startsWith(String(dateStr))) {
          Serial.println("  [!] Attendance already marked for this class session");
          http.end();
          return true; // Already marked, consider it success
        }
      }
    }
  } else if (checkCode != 404) {
    // Only warn if it's not a 404 (empty collection is fine)
    Serial.printf("  [!] Could not check existing records (HTTP %d)\n", checkCode);
  }
  http.end();
  
  // Step 2: Create new attendance record
  Serial.println("  [*] Creating new attendance record...");
  String attendanceUrl = "https://firestore.googleapis.com/v1/projects/" + 
                         String(FIREBASE_PROJECT_ID) + 
                         "/databases/(default)/documents/attendance_records?key=" + 
                         String(FIREBASE_API_KEY);
  
  // Create JSON document following Firestore REST API format
  DynamicJsonDocument attendanceDoc(2048);
  attendanceDoc["fields"]["studentId"]["stringValue"] = studentId;
  attendanceDoc["fields"]["batchId"]["stringValue"] = schedule->batchId;
  attendanceDoc["fields"]["scheduleId"]["stringValue"] = schedule->scheduleId;
  attendanceDoc["fields"]["date"]["timestampValue"] = timestampStr;
  attendanceDoc["fields"]["isPresent"]["booleanValue"] = true;
  attendanceDoc["fields"]["markedBy"]["stringValue"] = "ESP32";
  attendanceDoc["fields"]["markedAt"]["timestampValue"] = timestampStr;
  
  String attendanceJson;
  serializeJson(attendanceDoc, attendanceJson);
  
  // Debug: Print JSON being sent (uncomment for debugging)
  // Serial.println("  [DEBUG] JSON: " + attendanceJson);
  
  http.begin(client, attendanceUrl);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(attendanceJson);
  
  bool success = (httpCode == 200 || httpCode == 201);
  
  if (success) {
    Serial.println("  [✓] Attendance marked successfully in Firestore");
  } else {
    Serial.printf("  [✗] Failed to mark attendance (HTTP %d)\n", httpCode);
    String response = http.getString();
    Serial.println("  [✗] Error response: " + response);
    
    if (httpCode == 403) {
      Serial.println("  [!] Permission denied - check Firestore security rules");
    } else if (httpCode == 401) {
      Serial.println("  [!] Authentication failed - check API key");
    }
  }
  
  http.end();
  return success;
}

// ==================== MAIN LOOP ====================

void loop() {
  unsigned long currentMillis = millis();
  
  // LED heartbeat
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[!] WiFi disconnected, reconnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);
    return;
  }
  
  // Sync time periodically
  if (currentMillis - lastTimeSync >= TIME_SYNC_INTERVAL || !timeInitialized) {
    initializeTime();
    lastTimeSync = currentMillis;
  }
  
  // Refresh schedules periodically
  if (currentMillis - lastScheduleRefresh >= SCHEDULE_REFRESH_INTERVAL || !schedulesLoaded) {
    loadTodaysSchedules();
    lastScheduleRefresh = currentMillis;
  }
  
  // Check for new devices
  if (currentMillis - lastDeviceCheck >= DEVICE_CHECK_INTERVAL) {
    checkForNewDevices();
    lastDeviceCheck = currentMillis;
  }
  
  delay(500);
}
