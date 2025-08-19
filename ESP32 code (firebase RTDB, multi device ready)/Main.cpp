#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// ====== WIFI ======
#define WIFI_SSID "Your_WiFi_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"

// ====== FIREBASE ======
#define API_KEY "Your_Firebase_Web_API_Key"
#define DATABASE_URL "https://your-project-id-default-rtdb.asia-southeast1.firebasedatabase.app/" // end with '/'

// Change this device ID to make it unique in the database
#define DEVICE_ID "lamp01"

// Relay pin (ESP32) — replace if necessary
const int RELAY_PIN = 23; //Active LOW

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String currentState = "OFF";

void setRelay(const String& state) { 
if (state == "ON") { 
digitalWrite(RELAY_PIN, LOW); // ON (active LOW) 
} else { 
digitalWrite(RELAY_PIN, HIGH); // OFF 
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

// Anonymous sign-in (enable Anonymous in Firebase Auth) 
if (Firebase.signUp(&config, &auth, "", "")) { 
Serial.println("Firebase signUp OK"); 
} else { 
Serial.printf("signUp failed: %s\n", config.signer.signupError.message.c_str()); 
} 

Firebase.begin(&config, &auth); 
Firebase.reconnectWiFi(true);
}

void streamCallback(FirebaseStream data) { 
if (data.dataTypeEnum() == fb_esp_rtdb_data_type_string) { 
String val = data.stringData(); 
val.toUpperCase(); 
if (val == "ON" || val == "OFF") { 
currentState = val; 
setRelay(currentState); 
// Report actual status 
String statusPath = String("/devices/") + DEVICE_ID + "/status"; 
Firebase.RTDB.setString(&fbdo, statusPath, currentState); 
Serial.println("Updated state to: " + currentState); 
} 
}
}

void streamTimeoutCallback(bool timeout) { 
if (timeout) Serial.println("Stream timeout, reconnecting...");
}

void setup() { 
Serial.begin(115200); 
pinMode(RELAY_PIN, OUTPUT); 
digitalWrite(RELAY_PIN, HIGH); //default OFF 

connectWiFi(); 
setupFirebase(); 

// Initial synchronization 
String base = String("/devices/") + DEVICE_ID + "/"; 
String statePath = base + "state"; 
String namePath = base + "name"; 

// Make sure the node exists 
if (!Firebase.RTDB.getString(&fbdo, statePath)) { 
Firebase.RTDB.setString(&fbdo, statePath, currentState); 
} else { 
String s = fbdo.stringData(); 
s.toUpperCase(); 
currentState = (s == "ON" || s == "OFF") ? s : "OFF"; 
} 

// Set a default name if it doesn't already exist 
if (!Firebase.RTDB.getString(&fbdo, namePath)) { 
Firebase.RTDB.setString(&fbdo, namePath, "ESP32 Lamp"); 
} 

setRelay(currentState); 
Firebase.RTDB.setString(&fbdo, base + "state", currentState); 

// Real-time stream 
if (!Firebase.RTDB.beginStream(&fbdo, statePath)) { 
Serial.printf("Stream error: %s\n", fbdo.errorReason().c_str()); 
} 
Firebase.RTDB.setStreamCallback(&fbdo, streamCallback, streamTimeoutCallback);
}

void loop() { 
// Streams handled by the library
}
