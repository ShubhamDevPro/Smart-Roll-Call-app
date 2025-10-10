/*
 * =====================================================
 * Configuration File Template - EXAMPLE
 * =====================================================
 * 
 * This is a template configuration file with placeholder values.
 * 
 * SETUP INSTRUCTIONS:
 * 1. Copy this file to config.h:
 *    cp include/config.example.h include/config.h
 * 
 * 2. Edit config.h and replace ALL placeholder values with your actual credentials
 * 
 * 3. Upload to ESP32
 * 
 * IMPORTANT: 
 * - This file (config.example.h) is committed to Git as a template
 * - config.h is NOT committed to Git (listed in .gitignore)
 * - Never commit your actual credentials to Git!
 * 
 * =====================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== FIREBASE CONFIGURATION ====================

// Firebase Project ID 
// Get from: Firebase Console -> Project Settings
#define FIREBASE_PROJECT_ID "your-project-id-here"

// Firebase API Key
// Get from: Firebase Console -> Project Settings -> Web API Key
#define FIREBASE_API_KEY "your-firebase-api-key-here"

// Firebase User ID
// Get from: Firebase Console -> Authentication -> Users -> User UID
// Or from Flutter: FirebaseAuth.instance.currentUser?.uid
#define FIREBASE_USER_ID "your-firebase-user-id-here"

// ==================== HOTSPOT CONFIGURATION ====================

// ESP32 Hotspot SSID (the name students will see)
#define AP_SSID "YourHotspotName"

// ESP32 Hotspot Password (minimum 8 characters)
#define AP_PASSWORD "your-hotspot-password"

// Hotspot channel (1-13)
#define AP_CHANNEL 10

// Hide SSID (true = hidden, false = visible)
#define HIDE_SSID false

// Maximum number of simultaneous connections
#define MAX_CONNECTIONS 4

// ==================== HOME WIFI CONFIGURATION ====================

// Your home WiFi SSID (for internet access)
#define WIFI_SSID "YourHomeWiFiName"

// Your home WiFi password
#define WIFI_PASSWORD "your-wifi-password"

// ==================== NTP CONFIGURATION ====================

// NTP Server for time synchronization
#define NTP_SERVER "pool.ntp.org"

// GMT Offset in seconds (adjust for your timezone)
// Examples:
//   GMT+5:30 (India): 19800
//   GMT+0:00 (UTC):   0
//   GMT-5:00 (EST):   -18000
#define GMT_OFFSET_SEC 0

// Daylight saving time offset (usually 0 if not applicable)
#define DAYLIGHT_OFFSET_SEC 0

// ==================== TIMING CONFIGURATION ====================

// How often to check for new devices (milliseconds)
#define DEVICE_CHECK_INTERVAL 5000

// How often to refresh class schedules (milliseconds)
#define SCHEDULE_REFRESH_INTERVAL 300000

// How often to sync time from NTP server (milliseconds)
#define TIME_SYNC_INTERVAL 3600000

#endif // CONFIG_H
