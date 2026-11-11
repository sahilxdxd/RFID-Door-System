
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <math.h>

// ==========================================
// 1. HARDWARE PIN CONFIGURATION
// ==========================================

#define SS_PIN     21
#define RST_PIN    22
#define SCK_PIN    18
#define MISO_PIN   19
#define MOSI_PIN   23

#define LED_PIN    4
#define SERVO_PIN  13

// ==========================================
// 2. WIFI ACCESS POINT
// ==========================================

// Change this password before using the project.
const char* ssid = "RFID-Door-System";
const char* password = "ChangeThisPassword";

// ==========================================
// 3. HARDWARE OBJECTS
// ==========================================

MFRC522 rfid(SS_PIN, RST_PIN);
WebServer server(80);
Servo doorServo;

// ==========================================
// 4. AUTHORIZED RFID CARD
// ==========================================

// Replace with your own card UID.
// Example UID from the project: C1 B4 2B 07

byte authorizedUID[] = {
  0xC1, 0xB4, 0x2B, 0x07
};

// ==========================================
// 5. DOOR CONFIGURATION
// ==========================================

const int CLOSED_ANGLE = 0;
const int OPEN_ANGLE = 90;

unsigned long openDuration = 5000;

int speedLevel = 100;

// Movement timing
const unsigned long SLOWEST_MOVE_MS = 3500;
const unsigned long FASTEST_GRADUAL_MS = 800;
const unsigned long DIRECT_MOVE_ESTIMATE_MS = 700;

// ==========================================
// 6. DOOR STATES
// ==========================================

enum DoorState {
  DOOR_CLOSED,
  DOOR_OPENING,
  DOOR_OPEN,
  DOOR_CLOSING
};

DoorState doorState = DOOR_CLOSED;

// Servo tracking variables
float currentAngle = CLOSED_ANGLE;
float motionStartAngle = CLOSED_ANGLE;

int targetAngle = CLOSED_ANGLE;
int lastWrittenAngle = CLOSED_ANGLE;

unsigned long motionStartedAt = 0;
unsigned long motionDuration = 0;
unsigned long lastServoUpdate = 0;
unsigned long doorOpenedAt = 0;

bool moveActive = false;
bool gradualMovement = false;

// ==========================================
// 7. DASHBOARD VARIABLES
// ==========================================

String lastUID = "No scan yet";
String lastResult = "WAITING";
String lastMember = "None";

int grantedCount = 0;
int deniedCount = 0;

String historyItems[5] = {
  "No activity yet", "", "", "", ""
};

// ==========================================
// 8. GET MOVEMENT DURATION
// ==========================================

unsigned long getMovementDuration() {

  if (speedLevel >= 100) {
    return DIRECT_MOVE_ESTIMATE_MS;
  }

  return (unsigned long)map(
    speedLevel,
    1,
    99,
    SLOWEST_MOVE_MS,
    FASTEST_GRADUAL_MS
  );
}

// ==========================================
// 9. GET DOOR STATUS
// ==========================================

String getDoorStatus() {

  switch (doorState) {

    case DOOR_CLOSED:
      return "CLOSED";

    case DOOR_OPENING:
      return "OPENING";

    case DOOR_OPEN:
      return "OPEN";

    case DOOR_CLOSING:
      return "CLOSING";
  }

  return "UNKNOWN";
}

// ==========================================
// 10. JSON ESCAPING
// ==========================================

String escapeJSON(String value) {

  value.replace("\\", "\\\\");
  value.replace("\"", "\\\"");
  value.replace("\n", "\\n");
  value.replace("\r", "");

  return value;
}

// ==========================================
// 11. ADD SCAN HISTORY
// ==========================================

void addHistory(String message) {

  for (int i = 4; i > 0; i--) {
    historyItems[i] = historyItems[i - 1];
  }

  historyItems[0] = message;
}

// ==========================================
// 12. READ RFID UID
// ==========================================

String readUID() {

  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      uid += "0";
    }

    uid += String(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      uid += " ";
    }
  }

  uid.toUpperCase();

  return uid;
}

// ==========================================
// 13. CHECK AUTHORIZED CARD
// ==========================================

bool checkAuthorized() {

  if (rfid.uid.size != sizeof(authorizedUID)) {
    return false;
  }

  for (byte i = 0; i < sizeof(authorizedUID); i++) {

    if (rfid.uid.uidByte[i] != authorizedUID[i]) {
      return false;
    }
  }

  return true;
}

// ==========================================
// 14. START SERVO MOVEMENT
// ==========================================

void startDoorMovement(int target, bool opening) {

  motionStartAngle = currentAngle;

  targetAngle = target;

  motionStartedAt = millis();
  lastServoUpdate = motionStartedAt;

  motionDuration = getMovementDuration();

  if (speedLevel >= 100) {

    // Direct target command
    gradualMovement = false;

    doorServo.write(targetAngle);

    // Software tracking only, not physical feedback
    currentAngle = targetAngle;
    lastWrittenAngle = targetAngle;

  } else {

    // Gradual movement
    gradualMovement = true;
  }

  moveActive = true;

  doorState = opening ? DOOR_OPENING : DOOR_CLOSING;

  if (opening) {
    Serial.println("Door opening...");
  } else {
    Serial.println("Door closing...");
  }
}

// ==========================================
// 15. OPEN DOOR
// ==========================================

void openDoor() {

  digitalWrite(LED_PIN, HIGH);

  if (doorState == DOOR_OPEN && !moveActive) {

    doorOpenedAt = millis();

    Serial.println("Door already open. Timer restarted.");

    return;
  }

  if (doorState == DOOR_OPENING &&
      moveActive &&
      targetAngle == OPEN_ANGLE) {

    return;
  }

  startDoorMovement(OPEN_ANGLE, true);

  Serial.println("ACCESS GRANTED!");
}

// ==========================================
// 16. UPDATE DOOR
// ==========================================

void updateDoor() {

  unsigned long now = millis();

  if (moveActive) {

    unsigned long elapsed = now - motionStartedAt;

    // Gradual servo movement
    if (gradualMovement &&
        (now - lastServoUpdate >= 20 ||
         elapsed >= motionDuration)) {

      lastServoUpdate = now;

      float progress = min(
        1.0f,
        (float)elapsed / (float)motionDuration
      );

      float angle = motionStartAngle +
        (targetAngle - motionStartAngle) * progress;

      currentAngle = angle;

      int commandAngle = (int)roundf(angle);

      if (commandAngle != lastWrittenAngle) {

        doorServo.write(commandAngle);

        lastWrittenAngle = commandAngle;
      }
    }

    // Finish movement
    if (elapsed >= motionDuration) {

      if (gradualMovement &&
          lastWrittenAngle != targetAngle) {

        doorServo.write(targetAngle);

        lastWrittenAngle = targetAngle;
      }

      currentAngle = targetAngle;

      moveActive = false;

      if (targetAngle == OPEN_ANGLE) {

        doorState = DOOR_OPEN;

        doorOpenedAt = millis();

        Serial.println("Door fully open.");

      } else {

        doorState = DOOR_CLOSED;

        digitalWrite(LED_PIN, LOW);

        Serial.println("Door fully closed.");
        Serial.println("LED OFF.");
      }
    }
  }

  // Automatic closing
  if (doorState == DOOR_OPEN &&
      !moveActive &&
      now - doorOpenedAt >= openDuration) {

    Serial.println("Automatic door closing.");

    startDoorMovement(CLOSED_ANGLE, false);
  }
}

// ==========================================
// 17. WEBSITE DASHBOARD
// ==========================================

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1">

<title>RFID Smart Door Dashboard</title>

<style>

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  padding: 20px;
  font-family: Arial, sans-serif;
  background: #edf2f7;
  color: #172b4d;
}

.container {
  max-width: 950px;
  margin: auto;
}

.header {
  background: #142e50;
  color: white;
  padding: 25px;
  border-radius: 15px;
}

.status {
  color: #a7f3d0;
  font-size: 13px;
}

.grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 15px;
  margin-top: 20px;
}

.card, .panel {
  background: white;
  padding: 20px;
  border-radius: 13px;
  box-shadow: 0 3px 12px #0000000d;
}

.label {
  color: #64748b;
  font-size: 13px;
}

.value {
  font-size: 25px;
  font-weight: bold;
  margin-top: 8px;
}

.green {
  color: #16a34a;
}

.red {
  color: #dc2626;
}

.panel {
  margin-top: 18px;
}

.setting {
  margin: 22px 0;
}

input[type=range] {
  width: 100%;
  margin: 15px 0;
  accent-color: #2563eb;
}

.speed-labels {
  display: flex;
  justify-content: space-between;
  color: #64748b;
  font-size: 12px;
}

button {
  background: #1d4ed8;
  color: white;
  border: none;
  border-radius: 8px;
  padding: 13px 20px;
  cursor: pointer;
  font-size: 14px;
}

.history {
  padding: 12px;
  margin-top: 9px;
  background: #f1f5f9;
  border-radius: 8px;
  font-size: 13px;
}

.footer {
  text-align: center;
  color: #64748b;
  margin: 25px 0;
  font-size: 12px;
}

@media(max-width: 550px) {
  .grid {
    grid-template-columns: 1fr;
  }
}

</style>

</head>

<body>

<div class="container">

<div class="header">

<h2>RFID Smart Door System</h2>

<p>ESP32 Wi-Fi Control Dashboard</p>

<div class="status" id="connection">
Connecting to ESP32...
</div>

</div>

<div class="grid">

<div class="card">
<div class="label">Door Status</div>
<div class="value" id="door">CLOSED</div>
</div>

<div class="card">
<div class="label">LED Status</div>
<div class="value" id="led">OFF</div>
</div>

<div class="card">
<div class="label">Authorized Scans</div>
<div class="value green" id="granted">0</div>
</div>

<div class="card">
<div class="label">Denied Scans</div>
<div class="value red" id="denied">0</div>
</div>

</div>

<div class="panel">

<h3>Latest RFID Scan</h3>

<p><b>Card UID:</b>
<span id="uid">--</span></p>

<p><b>Member:</b>
<span id="member">None</span></p>

<p><b>Access Result:</b>
<span id="result">WAITING</span></p>

</div>

<div class="panel">

<h3>Door Configuration</h3>

<div class="setting">

<label>
Door Open Duration:
<b id="durationText">5 seconds</b>
</label>

<input
type="range"
id="duration"
min="1"
max="20"
value="5"
oninput="document.getElementById('durationText').innerText=this.value+' seconds'"
>

</div>

<div class="setting">

<label>
Servo Movement Speed:
<b id="speedText">100%</b>
</label>

<input
type="range"
id="speed"
min="1"
max="100"
value="100"
oninput="document.getElementById('speedText').innerText=this.value+'%'"
>

<div class="speed-labels">
<span>Slowest</span>
<span>Fastest</span>
</div>

<p style="font-size:12px;color:#64748b">
100% = direct target command.
Lower values = gradual movement.
</p>

</div>

<button onclick="saveSettings()">
Apply Settings
</button>

<p id="saveMessage" style="color:#16a34a"></p>

</div>

<div class="panel">

<h3>Recent RFID Activity</h3>

<div id="history"></div>

</div>

<div class="footer">

RFID-Based Automatic Door System

<br>

Powered by ESP32 Wi-Fi

</div>

</div>

<script>

let settingsLoaded = false;

async function updateDashboard() {

  try {

    const response = await fetch('/status');

    const data = await response.json();

    document.getElementById('connection').innerText =
      '● ESP32 CONNECTED';

    document.getElementById('door').innerText =
      data.door;

    document.getElementById('led').innerText =
      data.led;

    document.getElementById('granted').innerText =
      data.granted;

    document.getElementById('denied').innerText =
      data.denied;

    document.getElementById('uid').innerText =
      data.uid;

    document.getElementById('member').innerText =
      data.member;

    const result = document.getElementById('result');

    result.innerText = data.result;

    result.className =
      data.result === 'ACCESS GRANTED'
      ? 'green'
      : data.result === 'ACCESS DENIED'
      ? 'red' : '';

    if (!settingsLoaded) {

      document.getElementById('duration').value =
        data.duration;

      document.getElementById('speed').value =
        data.speed;

      document.getElementById('durationText').innerText =
        data.duration + ' seconds';

      document.getElementById('speedText').innerText =
        data.speed + '%';

      settingsLoaded = true;
    }

    const historyBox =
      document.getElementById('history');

    historyBox.replaceChildren();

    data.history.forEach(item => {

      const div = document.createElement('div');

      div.className = 'history';
      div.textContent = item;

      historyBox.appendChild(div);
    });

  } catch(error) {

    document.getElementById('connection').innerText =
      '● ESP32 DISCONNECTED';
  }
}

async function saveSettings() {

  const duration =
    document.getElementById('duration').value;

  const speed =
    document.getElementById('speed').value;

  try {

    const response = await fetch(
      '/settings?duration=' + duration +
      '&speed=' + speed
    );

    document.getElementById('saveMessage').innerText =
      await response.text();

  } catch(error) {

    document.getElementById('saveMessage').innerText =
      'Unable to save settings.';
  }
}

updateDashboard();

setInterval(updateDashboard, 1000);

</script>

</body>
</html>

)rawliteral";

// ==========================================
// 18. WEB SERVER ROUTES
// ==========================================

void startWebsite() {

  server.on("/", HTTP_GET, []() {

    server.send_P(200, "text/html", webpage);

  });

  // Dashboard status
  server.on("/status", HTTP_GET, []() {

    String json = "{";

    json += "\"uid\":\"" + escapeJSON(lastUID) + "\",";
    json += "\"member\":\"" + escapeJSON(lastMember) + "\",";
    json += "\"result\":\"" + escapeJSON(lastResult) + "\",";

    json += "\"granted\":" + String(grantedCount) + ",";
    json += "\"denied\":" + String(deniedCount) + ",";

    json += "\"door\":\"" + getDoorStatus() + "\",";

    json += "\"led\":\"" +
            String(digitalRead(LED_PIN) ? "ON" : "OFF") + "\",";

    json += "\"duration\":" +
            String(openDuration / 1000) + ",";

    json += "\"speed\":" + String(speedLevel) + ",";

    json += "\"history\":[";

    for (int i = 0; i < 5; i++) {

      json += "\"" + escapeJSON(historyItems[i]) + "\"";

      if (i < 4) {
        json += ",";
      }
    }

    json += "]}";

    server.send(200, "application/json", json);

  });

  // Dashboard settings
  server.on("/settings", HTTP_GET, []() {

    if (server.hasArg("duration")) {

      int seconds = constrain(
        server.arg("duration").toInt(),
        1,
        20
      );

      openDuration = (unsigned long)seconds * 1000UL;
    }

    if (server.hasArg("speed")) {

      speedLevel = constrain(
        server.arg("speed").toInt(),
        1,
        100
      );
    }

    server.send(
      200,
      "text/plain",
      "Settings successfully applied!"
    );

    Serial.println("Dashboard settings updated.");

    Serial.print("Open duration: ");
    Serial.print(openDuration / 1000);
    Serial.println(" seconds");

    Serial.print("Servo speed: ");
    Serial.print(speedLevel);
    Serial.println("%");
  });

  server.begin();

  Serial.println("Web server started!");
}

// ==========================================
// 19. RFID SCANNING
// ==========================================

void processRFID() {

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  lastUID = readUID();

  Serial.println("\n------------------------------");

  Serial.print("Card UID: ");
  Serial.println(lastUID);

  if (checkAuthorized()) {

    lastMember = "Authorized User";
    lastResult = "ACCESS GRANTED";

    grantedCount++;

    addHistory(
      lastUID + " - ACCESS GRANTED"
    );

    Serial.println("ACCESS GRANTED!");

    openDoor();

  } else {

    lastMember = "Unknown User";
    lastResult = "ACCESS DENIED";

    deniedCount++;

    addHistory(
      lastUID + " - ACCESS DENIED"
    );

    Serial.println("ACCESS DENIED!");

  }

  // Finish RFID communication so the next
  // presentation can be read again.
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  Serial.println("------------------------------");
}

// ==========================================
// 20. SETUP
// ==========================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  SPI.begin(
    SCK_PIN,
    MISO_PIN,
    MOSI_PIN,
    SS_PIN
  );

  rfid.PCD_Init();

  doorServo.setPeriodHertz(50);

  doorServo.attach(
    SERVO_PIN,
    500,
    2400
  );

  doorServo.write(CLOSED_ANGLE);

  currentAngle = CLOSED_ANGLE;
  lastWrittenAngle = CLOSED_ANGLE;

  // Start Wi-Fi access point
  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  Serial.println("\n================================");
  Serial.println("RFID SMART DOOR SYSTEM");
  Serial.println("================================");

  Serial.print("Wi-Fi Name: ");
  Serial.println(ssid);

  Serial.print("Wi-Fi Password: ");
  Serial.println(password);

  Serial.print("Website: http://");
  Serial.println(WiFi.softAPIP());

  startWebsite();

  Serial.println("\nRFID Reader Ready!");
  Serial.println("Dashboard Ready!");
  Serial.println("System Ready!");
}

// ==========================================
// 21. MAIN LOOP
// ==========================================

void loop() {

  server.handleClient();

  updateDoor();

  processRFID();
}
