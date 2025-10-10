# 📊 System Architecture & Flow Diagrams

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     Firebase Firestore Cloud                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐  │
│  │   Batches    │  │  Schedules   │  │  Attendance Records  │  │
│  │              │  │              │  │                      │  │
│  │  - CSE-A     │  │  - Thursday  │  │  - Student ID        │  │
│  │  - CSE-B     │  │    13:00-14:00│  │  - Schedule ID      │  │
│  │  - Students  │  │  - Friday    │  │  - Date & Time      │  │
│  │  - MAC Addr  │  │    10:00-11:00│  │  - Marked by ESP32  │  │
│  └──────────────┘  └──────────────┘  └──────────────────────┘  │
└────────────────────────────┬────────────────────────────────────┘
                             │
                    ┌────────┴────────┐
                    │   Internet      │
                    │   (Home WiFi)   │
                    └────────┬────────┘
                             │
                    ┌────────┴────────┐
                    │   NTP Server    │
                    │  (Time Sync)    │
                    └────────┬────────┘
                             │
            ┌────────────────┴────────────────┐
            │         ESP32 Device             │
            │                                  │
            │  ┌──────────────────────────┐  │
            │  │   WiFi Station Mode      │  │
            │  │  (Connected to Internet)  │  │
            │  └──────────────────────────┘  │
            │                                  │
            │  ┌──────────────────────────┐  │
            │  │  WiFi Access Point Mode   │  │
            │  │   (Student Hotspot)       │  │
            │  │  "Everyday I'm buffering" │  │
            │  └──────────────────────────┘  │
            │                                  │
            │  ┌──────────────────────────┐  │
            │  │  Time Management System   │  │
            │  │  - Syncs every hour       │  │
            │  │  - GMT+5:30 (IST)        │  │
            │  └──────────────────────────┘  │
            │                                  │
            │  ┌──────────────────────────┐  │
            │  │   Schedule Manager        │  │
            │  │  - Refreshes every 5 min  │  │
            │  │  - Filters by day/time    │  │
            │  └──────────────────────────┘  │
            │                                  │
            │  ┌──────────────────────────┐  │
            │  │  Device Monitor           │  │
            │  │  - Checks every 5 seconds │  │
            │  │  - Tracks MAC addresses   │  │
            │  └──────────────────────────┘  │
            └──────────────────────────────────┘
                             │
            ┌────────────────┴────────────────┐
            │         │         │         │    │
       ┌────┴────┐ ┌─┴──┐ ┌───┴──┐ ┌────┴───┐
       │Student 1│ │ S2 │ │  S3  │ │  S4    │
       │Device   │ │Dev │ │Device│ │ Device │
       │(Phone)  │ │    │ │      │ │        │
       └─────────┘ └────┘ └──────┘ └────────┘
         Connected to ESP32 Hotspot
```

---

## 🔄 Main Flow Diagram

```
START
  │
  ├─► Initialize System
  │   ├─ Set CPU to 80MHz
  │   ├─ Enable power saving
  │   └─ Configure LED
  │
  ├─► Setup WiFi
  │   ├─ Create Hotspot (AP Mode)
  │   │   └─ SSID: "Everyday I'm buffering"
  │   └─ Connect to Home WiFi (STA Mode)
  │       └─ For internet access
  │
  ├─► Sync Time from NTP
  │   ├─ Connect to pool.ntp.org
  │   ├─ Set timezone to GMT+5:30
  │   └─ Get current date & time
  │
  ├─► Load Today's Schedules
  │   ├─ Get current day (e.g., "Thursday")
  │   ├─ Fetch all batches from Firestore
  │   ├─ For each batch:
  │   │   └─ Get schedules matching today
  │   └─ Store in memory
  │
  └─► Enter Main Loop
      │
      ├─► LED Heartbeat (blink)
      │
      ├─► Check WiFi Connection
      │   └─ If disconnected → Reconnect
      │
      ├─► Check Time (every hour)
      │   └─ Resync from NTP
      │
      ├─► Refresh Schedules (every 5 min)
      │   └─ Reload from Firestore
      │
      └─► Monitor Devices (every 5 sec)
          ├─ Get list of connected devices
          ├─ For each NEW device:
          │   └─ Process Attendance →
          │
          └─ Clean up disconnected devices
```

---

## 📱 Attendance Processing Flow

```
NEW DEVICE CONNECTED
  │
  ├─► Get Device Info
  │   ├─ MAC Address: AA:BB:CC:DD:EE:FF
  │   └─ IP Address: 192.168.4.2
  │
  ├─► Check Current Time
  │   └─ Example: Thursday, 13:15
  │
  ├─► Find Active Schedule
  │   ├─ Loop through today's schedules
  │   ├─ Check if current time is within any schedule
  │   └─ Result:
  │       ├─ FOUND: Schedule "13:00-14:00" for Batch "CSE-A"
  │       └─ NOT FOUND: Exit (no attendance marking)
  │
  ├─► Search Student by MAC
  │   ├─ Query Firestore:
  │   │   users/{userId}/batches/CSE-A/students
  │   ├─ Filter by macAddress == "AA:BB:CC:DD:EE:FF"
  │   └─ Result:
  │       ├─ FOUND: Student "John Doe" (ID: abc123)
  │       └─ NOT FOUND: Exit (student not registered)
  │
  ├─► Check Duplicate
  │   ├─ Query attendance_records collection
  │   ├─ Filter by:
  │   │   ├─ studentId == "abc123"
  │   │   ├─ scheduleId == "schedule789"
  │   │   └─ date == today
  │   └─ Result:
  │       ├─ EXISTS: Exit (already marked)
  │       └─ NOT EXISTS: Continue
  │
  ├─► Mark Attendance
  │   ├─ Create new document in attendance_records:
  │   │   {
  │   │     studentId: "abc123",
  │   │     batchId: "CSE-A",
  │   │     scheduleId: "schedule789",
  │   │     date: "2025-10-09T13:15:00Z",
  │   │     isPresent: true,
  │   │     markedBy: "ESP32",
  │   │     markedAt: "2025-10-09T13:15:00Z"
  │   │   }
  │   └─ Send POST request to Firestore
  │
  └─► Success!
      └─ Mark device as processed
```

---

## 🕐 Time-Based Decision Tree

```
                    Current Time & Day
                           │
                 ┌─────────┴─────────┐
                 │                   │
            Is it during        No class
            a scheduled      ┌─ scheduled
            class time?      │
                 │           │
            ┌────┴────┐      │
            │         │      │
           YES       NO      │
            │         │      │
            │         └──────┴─► DO NOTHING
            │                    (Device connected
            │                     but ignored)
            ▼
      Get Schedule Info
      ┌─────────────┐
      │ Schedule ID │
      │ Batch ID    │
      │ Start: 13:00│
      │ End: 14:00  │
      └──────┬──────┘
             │
             ▼
    Search for Student
      in this Batch
             │
        ┌────┴────┐
        │         │
      FOUND    NOT FOUND
        │         │
        │         └─► DO NOTHING
        │             (Device not registered)
        ▼
   Mark Attendance
        │
        └─► SUCCESS!

```

---

## 📊 Data Flow Example

### Scenario: Thursday, 13:15 PM

```
┌───────────────────────────────────────────────────────────┐
│ Step 1: Student connects phone to ESP32 hotspot          │
└───────────────────────────────────────────────────────────┘
                         │
                         ▼
┌───────────────────────────────────────────────────────────┐
│ ESP32 detects new device                                  │
│ MAC: AA:BB:CC:DD:EE:FF                                   │
│ IP: 192.168.4.2                                          │
└───────────────────────────────────────────────────────────┘
                         │
                         ▼
┌───────────────────────────────────────────────────────────┐
│ ESP32 checks current time & day                          │
│ Time: 13:15 (1:15 PM)                                    │
│ Day: Thursday                                             │
└───────────────────────────────────────────────────────────┘
                         │
                         ▼
┌───────────────────────────────────────────────────────────┐
│ ESP32 checks loaded schedules for Thursday               │
│                                                           │
│ Schedule 1: 13:00-14:00 (CSE-A) ✅ MATCH                 │
│ Schedule 2: 15:00-16:00 (CSE-B) ❌ Not yet               │
└───────────────────────────────────────────────────────────┘
                         │
                         ▼
┌───────────────────────────────────────────────────────────┐
│ ESP32 searches for student in Batch CSE-A                │
│                                                           │
│ Query: students WHERE macAddress == "AA:BB:CC:DD:EE:FF" │
└───────────────────────────────────────────────────────────┘
                         │
                         ▼
┌───────────────────────────────────────────────────────────┐
│ Student found!                                            │
│ Name: John Doe                                            │
│ Enroll: 2024001                                           │
│ Student ID: abc123                                        │
└───────────────────────────────────────────────────────────┘
                         │
                         ▼
┌───────────────────────────────────────────────────────────┐
│ ESP32 checks for duplicate attendance                     │
│                                                           │
│ Query: attendance_records WHERE                           │
│   studentId == "abc123" AND                              │
│   scheduleId == "schedule789" AND                        │
│   date == "2025-10-09"                                   │
│                                                           │
│ Result: No existing record ✅                            │
└───────────────────────────────────────────────────────────┘
                         │
                         ▼
┌───────────────────────────────────────────────────────────┐
│ ESP32 creates attendance record in Firestore             │
│                                                           │
│ POST to: attendance_records/                             │
│ {                                                         │
│   studentId: "abc123",                                   │
│   batchId: "CSE-A",                                      │
│   scheduleId: "schedule789",                             │
│   date: "2025-10-09T13:15:00Z",                         │
│   isPresent: true,                                       │
│   markedBy: "ESP32",                                     │
│   markedAt: "2025-10-09T13:15:00Z"                      │
│ }                                                         │
└───────────────────────────────────────────────────────────┘
                         │
                         ▼
┌───────────────────────────────────────────────────────────┐
│ ✅ ATTENDANCE MARKED SUCCESSFULLY!                        │
│                                                           │
│ Visible in Flutter app immediately                        │
│ - Teacher dashboard shows "Present"                       │
│ - Student can see their attendance                        │
└───────────────────────────────────────────────────────────┘
```

---

## 🔍 Scenario Comparison

### ✅ Scenario A: Successful Attendance

| Factor | Status |
|--------|--------|
| Current Time | 13:15 (Thursday) |
| Scheduled Class | 13:00-14:00 ✅ |
| MAC Address | Registered ✅ |
| Already Marked | No ✅ |
| **Result** | **Attendance Marked** ✅ |

---

### ❌ Scenario B: No Class Scheduled

| Factor | Status |
|--------|--------|
| Current Time | 16:30 (Thursday) |
| Scheduled Class | None ❌ |
| MAC Address | Registered ✅ |
| Already Marked | N/A |
| **Result** | **No Action** ❌ |

---

### ❌ Scenario C: Unregistered Device

| Factor | Status |
|--------|--------|
| Current Time | 13:15 (Thursday) |
| Scheduled Class | 13:00-14:00 ✅ |
| MAC Address | NOT Registered ❌ |
| Already Marked | N/A |
| **Result** | **No Action** ❌ |

---

### ⚠️ Scenario D: Duplicate Attempt

| Factor | Status |
|--------|--------|
| Current Time | 13:45 (Thursday) |
| Scheduled Class | 13:00-14:00 ✅ |
| MAC Address | Registered ✅ |
| Already Marked | Yes ⚠️ |
| **Result** | **Ignored (Already Marked)** ⚠️ |

---

## 🔄 Schedule Refresh Cycle

```
Time: 00:00 (Midnight)
  │
  ├─► ESP32 continues running
  │
  ├─► Next schedule check (any time)
  │   └─► ESP32 notices day changed
  │       └─► Loads new day's schedules
  │
  └─► New day's classes are now active

Example:
  Thursday 23:59 → Has Thursday schedules
  Friday  00:01 → Automatically loads Friday schedules
```

---

## 📈 Performance Metrics

```
┌─────────────────────────────────────────┐
│ Device Check Interval: 5 seconds        │
│ Schedule Refresh: 5 minutes             │
│ Time Sync: 1 hour                       │
│ Max Devices: 4 simultaneous             │
│ Response Time: < 2 seconds per device   │
│ Power Consumption: ~120-180mA           │
│ CPU Frequency: 80 MHz                   │
│ WiFi TX Power: 15 dBm                   │
└─────────────────────────────────────────┘
```

---

## 🎯 Key Design Decisions

### Why check schedules from Firestore?
- **Real-time updates**: Teachers can modify schedules without reprogramming ESP32
- **Flexibility**: Supports multiple classes per day
- **Accuracy**: Always uses the latest schedule data

### Why use MAC addresses?
- **Automatic**: No need for students to manually check in
- **Unique**: Each device has a unique MAC address
- **Persistent**: MAC address doesn't change

### Why check time-based schedules?
- **Prevents abuse**: Can't mark attendance outside class hours
- **Accurate**: Ensures attendance is only for actual class time
- **Fair**: All students must be present during class

### Why use dual WiFi mode?
- **Internet access**: Needed for Firestore and time sync
- **Student connectivity**: Hotspot for device detection
- **Simultaneous**: ESP32 supports both modes at once

---

This visual guide should help you understand how the entire system works together! 🚀
