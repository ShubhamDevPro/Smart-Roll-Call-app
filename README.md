# ESP32 Smart Roll Call - Automatic Attendance System

## 📋 Overview

This ESP32 system automatically marks student attendance based on:
- **MAC Address Detection**: When a student's device connects to the ESP32 hotspot
- **Class Schedule**: Only marks attendance when there's an active class in session
- **Time Verification**: Checks current time against Firestore schedule database
- **Automatic Sync**: Directly writes to Firestore database

## 🎯 Features

✅ **Smart Scheduling**
- Fetches class schedules from Firestore
- Only marks attendance during scheduled class times
- Supports multiple classes per day
- Auto-refreshes schedule every 5 minutes

✅ **Device Detection**
- Detects new devices connecting to hotspot
- Identifies students by MAC address
- Prevents duplicate attendance marking
- Tracks connected/disconnected devices

✅ **Time Management**
- Syncs with NTP server for accurate time
- Supports Indian Standard Time (GMT+5:30)
- Auto-resyncs every hour
- Displays current day and time

✅ **Power Optimization**
- CPU frequency reduced to 80MHz
- WiFi power save mode enabled
- Reduced TX power (15dBm)
- Optimized for 24/7 operation

## 🔧 Hardware Requirements

- **ESP32 Dev Board** (any variant with WiFi)
- **USB Cable** for programming
- **Power Supply** (5V USB or battery pack)

## ⚙️ Configuration Setup

### **IMPORTANT: First Time Setup**

Before uploading the code to your ESP32, you MUST configure your credentials:

1. **Copy the example configuration file:**
   ```bash
   cp include/config.example.h include/config.h
   ```

2. **Edit `include/config.h`** with your actual credentials:
   - Firebase Project ID
   - Firebase API Key
   - Firebase User ID (get from Firebase Console → Authentication)
   - WiFi SSID and Password (your home WiFi for internet)
   - Hotspot SSID and Password (for students to connect)

3. **NEVER commit `include/config.h`** to Git - it's already in `.gitignore`

4. **Upload to ESP32** after configuration

> 🔒 **Security Note:** The `config.h` file contains sensitive credentials and is excluded from Git. Only `config.example.h` (with placeholder values) is committed to the repository.

## 📦 Required Libraries

Install these libraries via Arduino IDE Library Manager:

1. **ArduinoJson** (by Benoit Blanchon)
   - Version: 6.x or higher
   - Used for parsing Firestore JSON responses

2. Built-in ESP32 libraries (included with ESP32 board package):
   - WiFi
   - HTTPClient
   - esp_wifi
   - time

## 🚀 Installation Steps

### Step 1: Install Arduino IDE and ESP32 Support

1. Download and install [Arduino IDE](https://www.arduino.cc/en/software)
2. Open Arduino IDE → File → Preferences
3. Add ESP32 board URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to Tools → Board → Boards Manager
5. Search for "ESP32" and install "esp32 by Espressif Systems"

### Step 2: Install Required Libraries

1. Open Arduino IDE → Tools → Manage Libraries
2. Search and install **ArduinoJson** (version 6.x)

### Step 3: Get Your Firebase User ID

**IMPORTANT**: You need to find your Firebase User ID. Here's how:

#### Method 1: From Flutter App
1. Open your Flutter app
2. Login with your account
3. Check the terminal/debug console for the user ID
4. Or add this code temporarily in your Flutter app:
   ```dart
   print("User ID: ${FirebaseAuth.instance.currentUser?.uid}");
   ```

#### Method 2: From Firebase Console
1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Select your project: `smart-roll-call-76a46`
3. Go to Authentication → Users
4. Click on your user account
5. Copy the "User UID"

### Step 4: Configure the ESP32 Code

1. Open `ESP32_SmartRollCall_Scheduler.ino` in Arduino IDE
2. Update these configuration values (around line 27-40):

```cpp
// IMPORTANT: Replace with your actual Firebase User ID
const char* FIREBASE_USER_ID = "YOUR_USER_ID_HERE";

// Update WiFi credentials if needed
const char* AP_SSID = "Everyday I'm buffering";
const char* AP_PASSWORD = "Pass123";

const char* WIFI_SSID = "Shubham";
const char* WIFI_PASSWORD = "Pass123";
```

### Step 5: Upload to ESP32

1. Connect ESP32 to computer via USB
2. Select Board: Tools → Board → ESP32 Dev Module
3. Select Port: Tools → Port → (your ESP32 port)
4. Click Upload button (→)
5. Wait for "Done uploading" message

### Step 6: Monitor Serial Output

1. Open Serial Monitor: Tools → Serial Monitor
2. Set baud rate to **115200**
3. You should see:
   ```
   ========================================
   ESP32 Smart Roll Call System v2.0
   ========================================
   [*] Initializing...
   [✓] System Ready - Monitoring Devices
   ```

## 📱 How It Works

### System Flow:

```
1. ESP32 starts up
   ↓
2. Connects to home WiFi (for internet)
   ↓
3. Creates hotspot for students
   ↓
4. Syncs time from NTP server
   ↓
5. Fetches today's class schedules from Firestore
   ↓
6. Monitors for connecting devices
   ↓
7. When new device connects:
   - Get current time (e.g., 13:15)
   - Check if there's a class right now (e.g., 13:00-14:00 on Thursday)
   - If YES: Find student by MAC address
   - If found: Mark attendance in Firestore
   - If NO class: Do nothing
   ↓
8. Repeat monitoring
```

### Example Scenario:

**Current Time**: Thursday, 13:15 (1:15 PM)  
**Schedules in Firestore**:
- Thursday 13:00-14:00 (Batch: CSE-A) ✅ **ACTIVE**
- Thursday 15:00-16:00 (Batch: CSE-B) ❌ Not active yet

**Student connects with MAC**: `AA:BB:CC:DD:EE:FF`

**ESP32 Actions**:
1. ✅ Class in session (13:00-14:00)
2. ✅ Search for student with this MAC in Batch CSE-A
3. ✅ Student found: "John Doe" (Enroll: 2024001)
4. ✅ Mark attendance in Firestore
5. ✅ Done!

**If no class is scheduled**: ESP32 detects the device but does NOT mark attendance.

## 🗄️ Database Structure

The ESP32 interacts with these Firestore collections:

### 1. Batches Collection
```
users/{userId}/batches/{batchId}
```

### 2. Schedules Collection
```
users/{userId}/batches/{batchId}/schedules/{scheduleId}
  - dayOfWeek: "Thursday"
  - startTime: "13:00"
  - endTime: "14:00"
  - isActive: true
```

### 3. Students Collection
```
users/{userId}/batches/{batchId}/students/{studentId}
  - name: "John Doe"
  - enrollNumber: "2024001"
  - macAddress: "AA:BB:CC:DD:EE:FF"
```

### 4. Attendance Records Collection
```
attendance_records/{recordId}
  - studentId: "abc123"
  - batchId: "batch456"
  - scheduleId: "schedule789"
  - date: Timestamp
  - isPresent: true
  - markedBy: "ESP32"
  - markedAt: Timestamp
```

## 🔍 Serial Monitor Output Examples

### Successful Attendance Marking:
```
========================================
[NEW DEVICE] MAC: AA:BB:CC:DD:EE:FF | IP: 192.168.4.2
========================================
[*] Class in session: 13:00 - 14:00 (Batch: CSE-A)
[*] Searching for student with MAC AA:BB:CC:DD:EE:FF in batch CSE-A...
[✓] Student found: John Doe (2024001)
[*] Marking attendance in Firestore...
[✓] Attendance marked successfully!
```

### No Class in Session:
```
========================================
[NEW DEVICE] MAC: AA:BB:CC:DD:EE:FF | IP: 192.168.4.2
========================================
[!] No class currently in session - Attendance not marked
```

### Student Not Found:
```
========================================
[NEW DEVICE] MAC: BB:CC:DD:EE:FF:AA | IP: 192.168.4.3
========================================
[*] Class in session: 13:00 - 14:00 (Batch: CSE-A)
[*] Searching for student with MAC BB:CC:DD:EE:FF:AA in batch CSE-A...
[!] No student found with MAC BB:CC:DD:EE:FF:AA in batch CSE-A
```

## ⚙️ Configuration Options

### Timing Configuration (line 43-46)
```cpp
const unsigned long DEVICE_CHECK_INTERVAL = 5000;    // Check devices every 5 seconds
const unsigned long SCHEDULE_REFRESH_INTERVAL = 300000; // Refresh schedules every 5 minutes
const unsigned long TIME_SYNC_INTERVAL = 3600000;    // Sync time every hour
```

### Time Zone Configuration (line 35)
```cpp
const long GMT_OFFSET_SEC = 19800;  // GMT+5:30 for India (5.5 hours * 3600)
```

For different time zones:
- GMT+0: `0`
- GMT+1: `3600`
- GMT-5: `-18000`
- GMT+8: `28800`

## 🐛 Troubleshooting

### Problem: ESP32 not connecting to WiFi
**Solution**: 
- Check WiFi credentials
- Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Move ESP32 closer to router

### Problem: Time not syncing
**Solution**:
- Check internet connection
- Verify NTP server is accessible
- Wait a few minutes for initial sync

### Problem: Schedules not loading
**Solution**:
- Verify `FIREBASE_USER_ID` is correct
- Check Firebase project ID
- Ensure Firestore security rules allow read access
- Check Serial Monitor for error codes

### Problem: Student not found by MAC address
**Solution**:
- Verify MAC address is stored in Firestore (uppercase)
- Ensure student is in the correct batch
- Check that student document has `macAddress` field

### Problem: Attendance not marking
**Solution**:
- Verify there's an active class at current time
- Check Firebase security rules allow write access
- Ensure `attendance_records` collection exists
- Check Serial Monitor for error messages

## 📊 Power Consumption

- **Active mode**: ~80mA @ 80MHz
- **With WiFi**: ~120-180mA
- **Daily consumption**: ~2.9-4.3 Wh (24h operation)
- **Recommended**: 5V/2A power supply or 10000mAh power bank

## 🔒 Security Notes

1. **WiFi Security**: The hotspot uses WPA2 password protection
2. **API Key**: The Firebase API key is embedded in code (acceptable for ESP32 as it only writes attendance)
3. **Firestore Rules**: Ensure proper security rules are set in Firebase
4. **MAC Address Privacy**: MAC addresses are considered sensitive data

## 🔄 Updates and Maintenance

### Regular Checks:
- ✅ Monitor Serial output for errors
- ✅ Check power supply stability
- ✅ Verify time accuracy
- ✅ Update WiFi credentials if changed

### Periodic Updates:
- 🔄 Refresh Firebase API key if rotated
- 🔄 Update NTP server if needed
- 🔄 Adjust time zone for daylight savings

## 📝 Firebase Security Rules

Add these rules to your Firestore to allow ESP32 access:

```javascript
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    // Allow ESP32 to read schedules and students
    match /users/{userId}/batches/{batch}/schedules/{schedule} {
      allow read: if true;
    }
    
    match /users/{userId}/batches/{batch}/students/{student} {
      allow read: if true;
    }
    
    // Allow ESP32 to write attendance records
    match /attendance_records/{record} {
      allow write: if true;
      allow read: if true;
    }
  }
}
```

**Note**: These rules allow public access. In production, implement proper authentication.

## 🎓 Student Setup Instructions

### For Students to Get Their MAC Address:

**Android:**
1. Settings → About Phone → Status → Wi-Fi MAC Address

**iPhone:**
1. Settings → General → About → Wi-Fi Address

**Windows:**
1. Open Command Prompt
2. Type: `ipconfig /all`
3. Look for "Physical Address"

**Mac:**
1. System Preferences → Network → Wi-Fi → Advanced → Hardware
2. Look for "MAC Address"

Then students should provide this to the teacher for registration in the app.

## 📞 Support

If you encounter issues:
1. Check Serial Monitor output
2. Verify all configuration values
3. Ensure Firebase credentials are correct
4. Check Firestore security rules
5. Review the troubleshooting section

## 📄 License

This code is part of the Smart Roll Call system and follows the same license as the main project.

---

**Version**: 2.0  
**Last Updated**: October 2025  
**Compatibility**: ESP32 (all variants), Arduino IDE 2.x
