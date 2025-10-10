# 🆔 How to Find Your Firebase User ID

## Quick Methods

### Method 1: From Your Flutter App (Easiest)

1. Open your Flutter app project in VS Code
2. Go to `lib/services/firestore_service.dart`
3. Look for this line (around line 15):
   ```dart
   String? get userId => _auth.currentUser?.uid;
   ```

4. Add this temporary code in any screen where you're logged in:
   ```dart
   @override
   void initState() {
     super.initState();
     print("🔑 USER ID: ${FirebaseAuth.instance.currentUser?.uid}");
   }
   ```

5. Run your Flutter app and check the Debug Console
6. Copy the User ID that appears after "🔑 USER ID:"

---

### Method 2: From Firebase Console

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Click on your project: **smart-roll-call-76a46**
3. In the left sidebar, click **Authentication**
4. Click on the **Users** tab
5. Find your email/account in the list
6. Click on your user row
7. Copy the **User UID** (it looks like: `abc123XYZ456def789...`)

---

### Method 3: From Firestore Database

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Click on your project: **smart-roll-call-76a46**
3. In the left sidebar, click **Firestore Database**
4. Look at the **users** collection
5. Click on any document in the users collection
6. The document ID is your User ID!

---

### Method 4: Using Browser DevTools

1. Login to your Flutter web app
2. Open Browser DevTools (F12)
3. Go to **Console** tab
4. Type this command:
   ```javascript
   firebase.auth().currentUser.uid
   ```
5. Press Enter
6. Copy the displayed User ID

---

## What Does a User ID Look Like?

A Firebase User ID typically looks like:
```
hR3Kp8xYz2M4nVqW1cD5fG7jL9pT
```

Or:
```
a1b2c3d4-e5f6-7890-ghij-klmnopqrstuv
```

- It's usually **28 characters long** (alphanumeric)
- Or formatted with dashes (UUID style)
- It's **case-sensitive**

---

## Where to Put the User ID

Once you have your User ID, update it in the ESP32 code:

**File**: `ESP32_SmartRollCall_Scheduler.ino`  
**Line**: ~29

```cpp
// BEFORE:
const char* FIREBASE_USER_ID = "YOUR_USER_ID_HERE";

// AFTER (example):
const char* FIREBASE_USER_ID = "hR3Kp8xYz2M4nVqW1cD5fG7jL9pT";
```

---

## Verification

To verify you have the correct User ID:

1. In your Flutter app, go to Firestore Service
2. The userId is used in queries like:
   ```dart
   _firestore.collection('users').doc(userId).collection('batches')
   ```

3. This should match the structure in Firebase:
   ```
   users/
     └── {YOUR_USER_ID}/
           └── batches/
                 └── ...
   ```

---

## ⚠️ Important Notes

- **Keep it secure**: Don't share your User ID publicly
- **One per account**: Each Firebase user has a unique ID
- **Permanent**: The User ID doesn't change
- **Case-sensitive**: Copy it exactly as shown

---

## Need Help?

If you're still having trouble finding your User ID:

1. Make sure you're logged into your Flutter app
2. Check if you have any data in Firestore
3. Try Method 1 (printing in Flutter app) - it's the most reliable
4. Ensure you're using the correct Firebase project

---

## Quick Copy-Paste Code for Flutter

Add this anywhere in your Flutter app (temporarily) to print the User ID:

```dart
import 'package:firebase_auth/firebase_auth.dart';

// Add this in any StatefulWidget's initState or build method:
void printUserId() {
  final userId = FirebaseAuth.instance.currentUser?.uid;
  print('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
  print('🔑 YOUR USER ID: $userId');
  print('Copy this ID to ESP32 code!');
  print('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
}
```

Then call `printUserId()` and check your debug console!
