# ✅ ESP32 Testing Checklist

## Pre-Upload Checklist

Before uploading the code to ESP32, verify:

- [ ] **Firebase User ID** is updated in line 29
- [ ] **WiFi SSID** and **Password** are correct (line 38-39)
- [ ] **Hotspot SSID** and **Password** are set (line 32-33)
- [ ] **ArduinoJson library** is installed
- [ ] **ESP32 board package** is installed in Arduino IDE
- [ ] **Correct board** is selected (Tools → Board → ESP32 Dev Module)
- [ ] **Correct port** is selected (Tools → Port)

---

## Post-Upload Verification

After uploading, check Serial Monitor (115200 baud):

### 1. System Initialization
```
[*] ESP32 Upload Successful!
[*] Initializing SmartRollCall System...
```
- [ ] System starts without errors
- [ ] Power optimizations applied
- [ ] CPU frequency shows 80 MHz

### 2. WiFi Setup
```
[✓] Hotspot created successfully!
[✓] Connected to home WiFi
```
- [ ] Hotspot is created (check with phone WiFi list)
- [ ] Connected to home WiFi for internet
- [ ] Both IP addresses are displayed

### 3. Time Synchronization
```
[✓] Time synchronized successfully
    Current time: Thursday, October 09, 2025 13:15:00
```
- [ ] Time is synced from NTP server
- [ ] Current time is correct
- [ ] Day of week is correct
- [ ] Time zone is correct (GMT+5:30 for India)

### 4. Schedule Loading
```
[✓] Loaded 2 schedule(s) for today
  === Today's Class Schedule ===
  1. 13:00 - 14:00 (Batch: CSE-A)
  2. 15:00 - 16:00 (Batch: CSE-B)
```
- [ ] Schedules are loaded from Firestore
- [ ] Today's schedules are displayed
- [ ] Batch IDs are correct
- [ ] Times match your Firestore data

### 5. System Ready
```
[✓] System Ready - Monitoring Devices
```
- [ ] No error messages
- [ ] LED is blinking (heartbeat)
- [ ] System is ready to detect devices

---

## Functional Testing

### Test 1: Device Connection Detection

**Steps:**
1. Connect your phone to ESP32 hotspot
2. Check Serial Monitor

**Expected Output:**
```
========================================
[NEW DEVICE] MAC: AA:BB:CC:DD:EE:FF | IP: 192.168.4.2
========================================
```

**Verification:**
- [ ] Device is detected
- [ ] MAC address is displayed
- [ ] IP address is assigned

---

### Test 2: No Class in Session

**Scenario:** Connect when NO class is scheduled

**Expected Output:**
```
[NEW DEVICE] MAC: AA:BB:CC:DD:EE:FF | IP: 192.168.4.2
[!] No class currently in session - Attendance not marked
```

**Verification:**
- [ ] Device is detected
- [ ] System checks for active class
- [ ] NO attendance is marked
- [ ] Correct message is displayed

---

### Test 3: Class in Session - Student Found

**Scenario:** Connect during a scheduled class with registered MAC

**Setup:**
1. Ensure there's a class scheduled RIGHT NOW
2. Ensure your MAC address is registered in that batch
3. Connect to ESP32 hotspot

**Expected Output:**
```
[NEW DEVICE] MAC: AA:BB:CC:DD:EE:FF | IP: 192.168.4.2
[*] Class in session: 13:00 - 14:00 (Batch: CSE-A)
[*] Searching for student with MAC AA:BB:CC:DD:EE:FF in batch CSE-A...
[✓] Student found: John Doe (2024001)
[*] Marking attendance in Firestore...
[✓] Attendance marked successfully!
```

**Verification:**
- [ ] Class is detected
- [ ] Student is found by MAC
- [ ] Attendance is marked in Firestore
- [ ] Check Flutter app - attendance should appear

---

### Test 4: Class in Session - Student NOT Found

**Scenario:** Connect during class with unregistered MAC

**Expected Output:**
```
[NEW DEVICE] MAC: BB:CC:DD:EE:FF:00 | IP: 192.168.4.3
[*] Class in session: 13:00 - 14:00 (Batch: CSE-A)
[*] Searching for student with MAC BB:CC:DD:EE:FF:00 in batch CSE-A...
[!] No student found with MAC BB:CC:DD:EE:FF:00 in batch CSE-A
```

**Verification:**
- [ ] Class is detected
- [ ] Student search is attempted
- [ ] "Not found" message displayed
- [ ] NO attendance is marked

---

### Test 5: Duplicate Prevention

**Scenario:** Same device connects twice during same class

**Expected Output:**
First connection:
```
[✓] Attendance marked successfully!
```

Second connection:
```
[!] Attendance already marked for this student today
```

**Verification:**
- [ ] First connection marks attendance
- [ ] Second connection is prevented
- [ ] Only ONE attendance record in Firestore

---

### Test 6: Multiple Students

**Scenario:** Multiple students connect during class

**Expected Output:**
```
[NEW DEVICE] MAC: AA:BB:CC:DD:EE:FF | IP: 192.168.4.2
[✓] Attendance marked successfully!

[NEW DEVICE] MAC: 11:22:33:44:55:66 | IP: 192.168.4.3
[✓] Attendance marked successfully!

[NEW DEVICE] MAC: 99:88:77:66:55:44 | IP: 192.168.4.4
[✓] Attendance marked successfully!
```

**Verification:**
- [ ] All devices are detected
- [ ] Each student's attendance is marked separately
- [ ] All records appear in Firestore
- [ ] All records appear in Flutter app

---

## Firebase Verification

After testing, verify in Firebase Console:

### 1. Check Firestore Database

Navigate to: `attendance_records` collection

**Verify:**
- [ ] New documents are created
- [ ] `studentId` is correct
- [ ] `batchId` is correct
- [ ] `scheduleId` is correct
- [ ] `date` is today's date
- [ ] `isPresent` is `true`
- [ ] `markedBy` is "ESP32"
- [ ] `markedAt` timestamp is correct

### 2. Check Flutter App

**Verify:**
- [ ] Attendance appears in teacher dashboard
- [ ] Attendance appears in student view
- [ ] Correct date is shown
- [ ] Correct schedule is linked
- [ ] Student name is displayed correctly

---

## Performance Testing

### CPU and Memory
- [ ] ESP32 runs stable for at least 30 minutes
- [ ] No memory leaks (monitor free heap)
- [ ] LED blinks consistently (heartbeat)
- [ ] No unexpected resets

### Network Stability
- [ ] Hotspot remains stable
- [ ] Internet connection stays active
- [ ] Devices can connect and disconnect smoothly
- [ ] No WiFi drops

### Timing Accuracy
- [ ] Time stays synchronized
- [ ] Schedule checks are accurate
- [ ] Time-based marking works correctly

---

## Common Issues and Solutions

### Issue: "Time not synchronized"
**Solution:**
- Check internet connection
- Wait 1-2 minutes for NTP sync
- Verify NTP server is accessible

### Issue: "No schedules loaded"
**Solution:**
- Verify Firebase User ID is correct
- Check that schedules exist in Firestore
- Ensure today has scheduled classes
- Check Firestore security rules

### Issue: "Student not found"
**Solution:**
- Verify MAC address in Firestore (must be uppercase)
- Check student is in the correct batch
- Ensure `macAddress` field exists in student document

### Issue: "Failed to mark attendance"
**Solution:**
- Check internet connection
- Verify Firestore security rules
- Check Firebase API key
- Look for HTTP error codes in Serial Monitor

### Issue: Device connects but nothing happens
**Solution:**
- Check if there's a class scheduled right now
- Verify current time is correct
- Check if schedule times match

---

## Final Checklist

Before deployment:

- [ ] All tests passed
- [ ] Time is accurate
- [ ] Schedules are loading correctly
- [ ] Attendance marking works
- [ ] Firebase records are created
- [ ] Flutter app shows attendance
- [ ] System runs stable for 1+ hour
- [ ] Power supply is adequate
- [ ] ESP32 is securely mounted
- [ ] Hotspot is accessible to students

---

## Monitoring Commands

### View System Status
Check Serial Monitor for:
- Current time
- Active schedules
- Connected devices
- Attendance marking status

### Force Schedule Refresh
Press the RESET button on ESP32 to reload schedules

### Check Memory Usage
Add this to Serial Monitor periodically:
```cpp
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
```

---

## Production Deployment

When ready for production:

- [ ] Document the Firebase User ID (keep it secure)
- [ ] Note the hotspot SSID and password
- [ ] Set up reliable power supply
- [ ] Position ESP32 for good WiFi coverage
- [ ] Inform students of hotspot SSID
- [ ] Test with real students
- [ ] Monitor for first few days
- [ ] Keep backup ESP32 configured

---

## Support Documentation

Keep these files handy:
- `README.md` - Complete setup guide
- `FIND_USER_ID.md` - How to get Firebase User ID
- `TESTING_CHECKLIST.md` - This file
- Serial Monitor logs for troubleshooting

---

**Testing Date**: _____________  
**Tested By**: _____________  
**Result**: ☐ Pass  ☐ Fail  ☐ Needs Review  

**Notes:**
_____________________________________________
_____________________________________________
_____________________________________________
