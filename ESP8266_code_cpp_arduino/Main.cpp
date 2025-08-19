#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// ====== WIFI ======
#define WIFI_SSID "Your_WiFi_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"

// ====== FIREBASE ======
// Get it from the Firebase Console (Project Settings > Service accounts > Database secrets no longer exists.
// Use Web API Key + Anonymous Auth or Custom Token.
// For a simple demo without auth: use setBSSLBufferSize & setCertNone and a Realtime DB REST stream.
// However, in this library, we use API Key + Anonymous Sign-In.)
#define API_KEY "Your_Firebase_Web_API_Key"
#define DATABASE_URL "https://your-project-id-default-rtdb.asia-southeast1.firebasedatabase.app/" // end with '/'

// Device identity in DB: /devices/<DEVICE_ID>/state
#define DEVICE_ID "lamp01"

// Relay pin (active LOW)
const int RELAY_PIN = D1;

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Keep local state
String currentState = "OFF";

void setRelay(const String& state) { 
if (state == "ON") { 
digitalWrite(RELAY_PIN, LOW); // active (active LOW) 
} else { 
digitalWrite(RELAY_PIN, HIGH); // dead 
}
}

void connectWiFi() { 
WiFi.mode(WIFI_STA); 
WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
Serial.print("Connecting WiFi"); 
while (WiFi.status() != WL_CONNECTED) { 
delay(500); 
Serial.print("."); 
} 
Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

void setupFirebase() { 
config.api_key = API_KEY; 
config.database_url = DATABASE_URL; 

// Anonymous sign-in (enable Anonymous auth in Firebase Authentication) 
if (Firebase.signUp(&config, &auth, "", "")) { 
Serial.println("Firebase signUp OK"); 
} else { 
Serial.printf("signUp failed, reason: %s\n", config.signer.signupError.message.c_str()); 
} 

Firebase.begin(&config, &auth); 
Firebase.reconnectWiFi(true);
}

void streamCallback(FirebaseStream data) { 
Serial.printf("Stream path: %s\n", data.streamPath().c_str()); 
Serial.printf("Event path : %s\n", data.dataPath().c_str()); 
Serial.printf("Type : %s\n", data.dataType().c_str()); 
Serial.printf("Value : %s\n", data.stringData().c_str()); 

String val = data.stringData(); 
if (val == "ON" || val == "OFF") { 
currentState = val; 
setRelay(currentState); 

// Optional: report actual status to DB 
String statusPath = String("/devices/") + DEVICE_ID + "/status"; 
Firebase.RTDB.setString(&fbdo, statusPath, currentState); 
}
}

void streamTimeoutCallback(bool timeout) { 
if (timeout) { 
Serial.println("Stream timeout, resuming..."); 
}
}

void setup() { 
Serial.begin(115200); 
pinMode(RELAY_PIN, OUTPUT); 
digitalWrite(RELAY_PIN, HIGH); //default OFF 

connectWiFi(); 
setupFirebase(); 

// Make sure the path state exists 
String statePath = String("/devices/") + DEVICE_ID + "/state"; 
if (!Firebase.RTDB.getString(&fbdo, statePath)) { 
Firebase.RTDB.setString(&fbdo, statePath, currentState); 
} else { 
// Sync relay with DB on boot 
currentState = fbdo.stringData(); 
if (currentState != "ON" && currentState != "OFF") currentState = "OFF"; 
setRelay(currentState); 
} 

// Start stream (real-time listener) 
if (!Firebase.RTDB.beginStream(&fbdo, statePath)) { 
Serial.printf("Stream began error, %s\n", fbdo.errorReason().c_str()); 
}
Firebase.RTDB.setStreamCallback(&fbdo, streamCallback, streamTimeoutCallback);
}

void loop() {
// Firebase ESP Client manages the stream in the background
// Additional heartbeats or reconnects can be placed here if needed
}
