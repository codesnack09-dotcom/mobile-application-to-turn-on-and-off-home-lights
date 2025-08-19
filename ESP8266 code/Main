#include <ESP8266WiFi.h>

const char* ssid = "Your_WiFi_Name";
const char* password = "Your_WiFi_Password";

WiFiServer server(80);
int relayPin = D1;

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Lamp OFF initially

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/ON") != -1) {
    digitalWrite(relayPin, LOW);  // Turn ON
    Serial.println("Lamp ON");
  }
  if (request.indexOf("/OFF") != -1) {
    digitalWrite(relayPin, HIGH); // Turn OFF
    Serial.println("Lamp OFF");
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<html><body><h1>Lamp Controlled</h1></body></html>");
  client.stop();
}
