## Purpose: This is a mobile application for turning on and off your house lights. When you're away, you sometimes forget to turn on some of the lights, especially if you're alone or there's no one else around. The other occupants might be away, so you can't ask someone to turn on the lights. Don't worry, this mobile application is the solution. With this application, you can turn on and off the lights at any time. 

We'll create two parts: 
• IoT Device Code (ESP8266/ESP32) → which controls the lights. 
• Mobile App (e.g., Flutter) → which sends commands to the device over the internet (via HTTP or MQTT).

For this example, I'll create a simple version:
• Flutter mobile app → with ON and OFF buttons. • ESP8266 → has a small web server to receive ON/OFF commands.

 1️⃣ ESP8266 Code (English) 📌 Result: The ESP8266 will have an IP address, for example, 192.168.1.100. If you open http://192.168.1.100/ON → the light turns on. If you open http://192.168.1.100/OFF → the light turns off. 
2️⃣ Flutter Mobile App (English) pubspec.yaml → add the HTTP package:
lib/main.dart

How It Works
• Upload the ESP8266 code → note the IP address from the Serial Monitor.
• Replace deviceIP in the Flutter code with the ESP8266 IP address.
• Run the Flutter app on your phone (make sure your phone and ESP8266 are on the same Wi-Fi network or use port forwarding for access from outside the home).
• Press the ON/OFF button in the app → the lamp will respond. 

The version that can be controlled from outside the home network (global internet) without the hassle of setting up a router uses the Firebase Realtime Database or MQTT broker.

We'll create the global internet version without fiddling with the router, using the Firebase Realtime Database as an "intermediary" (cloud). 

The architecture is as follows: Flutter app (mobile) ⟷ Firebase (cloud) ⟷ ESP8266 (home) → Relay → Lamp

Below are the complete setup steps, ESP8266 code, and Flutter code.

1) Set up Firebase (just once)
• Open the Firebase Console and create a project (e.g., lamp-controller).
• Add a Realtime Database → select a location → Start in locked mode (we'll set the rules later). • Add a Web App (for Flutter) → copy the config (apiKey, projectId, etc.).
• In the Realtime Database, create a starting node: /devices/<YOUR_DEVICE_ID>/state = "OFF" (replace <YOUR_DEVICE_ID>, e.g., lamp01) 

Minimum rules for testing (public—for testing only, not for production): 
{ "rules": { ".read": true, ".write": true } }
For production, enable Auth and restrict write/read permissions to logged-in users.

2) ESP8266 Code (C++ / Arduino) Required libraries (Library Manager):
• Firebase ESP Client (by Mobizt)
• ESP8266WiFi

Pin connections:
• Relay IN → D1
• Relay VCC → 3V3, GND → GND

Note:
• Enable Anonymous Authentication in Firebase > Authentication > Sign-in method.
• Change API_KEY and DATABASE_URL according to your project.
• For production security, use Auth (Email/Password, Custom Token, etc.) and tighten the rules.

3) Flutter Mobile App (ON/OFF control) pubspec.yaml 

android/app/google-services.json & ios/Runner/GoogleService-Info.plist
• Download from the Firebase Console (Project Settings > Your apps > Flutter iOS/Android).
• Also add the Firebase Gradle plugin according to the Firebase (FlutterFire) guidelines.

lib/firebase_options.dart
• Auto-generate with: flutter pub add firebase_core firebase_database dart pub global activate flutterfire_cli flutterfire configure This generates a firebase_options.dart containing the app configuration. lib/main.dart 

Quick Testing 
• Upload the ESP8266 code (check the Serial Monitor to ensure Wi-Fi and Firebase are connected). 
• Run the Flutter app on your phone. 
• Try switching ON/OFF → the value in the DB changes → the ESP receives the stream and turns the relay on/off → the light turns on/off. 
• Unplug/unplug the ESP: the app will automatically sync with the latest status in the DB. 

Security & Production (important!) 
• DO NOT use public rules for production. Use Authentication (Anonymous/Email/Google Sign-In) and restrict the path: { "rules": { "devices": { "$deviceId": { ".read": "auth != null", ".write": "auth != null" } } } } 
• Hide credentials (keep the API key public in the app, but set strict rules). 
• Use watchdog/retry on the ESP for auto-reconnect. 

Alternative: MQTT (HiveMQ/Cloud MQTT) If you want very low latency and minimal traffic, you can use MQTT (public or self-hosted broker). The Flutter app publishes to the home/lamp01/cmd topic, the ESP subscribes, and drives the relay. (I can also create an MQTT version if you want.

Here, I've provided everything: a clear diagram/wiring, an ESP32 version, and a Flutter mobile app that supports multi-device (multiple lamps) with the Firebase Realtime Database.

1) Wiring (Diagram & Table) Purpose: The ESP (ESP8266/ESP32) controls the Relay → The Relay disconnects/connects the AC Lamp. ⚠️ Electrical safety 
Indonesia
Inggris
2.794 / 5.000
k: When controlling AC Mains (220V/110V), ensure safe cable routing, use appropriately rated relays, keep the casing closed, and do not touch the AC circuit while it is on.

a) Summary Schematic (ASCII) [ESP8266 / ESP32] [Relay Module] [AC Lamp] 3V3 ---------------------------> VCC GND ---------------------------> GND AC LIVE -----[COM] Relay [NO]----- Lamp ----- AC NEUTRAL D1 (ESP8266) / GPIO23 (ESP32) --> IN • Typical relay modes: Active LOW LOW → ON (NO contact closed) HIGH → OFF (NO contact open)

b) Pin Table (two variants) Components ESP8266 (NodeMCU) ESP32 (DevKit) Relay Module VCC3 V3 V3 VCC GND GND GND Control D1 GPIO23 IN AC Load —— COM & NO For testing without AC, replace the AC lamp with an LED + resistor on the breadboard (driven from 5V) via transistor/relay).

2) ESP32 Code (Firebase RTDB, Multi-device-ready) Use libraries: 
• ESP32 core (Arduino) 
• Firebase ESP Client (by Mobizt)
Data structure in Firebase:

3) Flutter App (Multi-device) This app: 
• Reads a list of devices from /devices.
• Displays a list with names and an ON/OFF switch.
• Tapping the switch → writes the state: "ON"/"OFF"; the ESP updates the state. 

pubspec.yaml
lib/main.dart

4) Firebase Realtime Database Rules Testing (easy, not secure) — while in development: { "rules": { ".read": true, ".write": true } } Production (more secure, with Auth): Enable Anonymous or Email/Google Sign-In. { "rules": { "devices": { "$deviceId": { ".read": "auth != null", ".write": "auth != null", "state": { ".validate": "newData.val() == 'ON' || newData.val() == 'OFF'" }, "status": { // status only updated by the device? Can be restricted via custom claims / separate path ".write": "auth != null", ".validate": "newData.val() == 'ON' || newData.val() == 'OFF'" } } } } }

5) How to Run (Short)
• Firebase
• Create a project + Realtime Database.
• Add an app (Android/iOS), download the config file (google-services.json / plist).
• (Optional) Enable Anonymous Auth.
• Create a node /devices/lamp01/{name:"Living Room", state:"OFF", status:"OFF"}.
• ESP32
• Install the Firebase ESP Client library.
• Fill in the API_KEY, DATABASE_URL, DEVICE_ID, SSID/Password.
• Upload → view the Serial Monitor (IP, connected...).
• Flutter App
• flutterfire configure → generate firebase_options.dart.
• Run flutter run.
• View the device list, toggle the ON/OFF switch.

6) Optional Extensions
• Group/Room: the rooms/{roomId}/devices/{deviceId} structure for grouping.
• Schedule: the schedules/ node that the ESP monitors for ON/OFF timers.
• MQTT: Replace Firebase with an MQTT broker for lower latency.
• Over-the-air (OTA): Add OTA updates for ESPs. 
