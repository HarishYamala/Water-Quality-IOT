#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <HardwareSerial.h>  // Use HardwareSerial instead of SoftwareSerial

// Define pins for sensors
#define TDS_PIN 34        // TDS sensor analog pin
#define TURBIDITY_PIN 35  // Turbidity sensor analog pin
#define ORP_PIN 36        // ORP sensor analog pin
#define PH_PIN 39         // pH sensor analog pin

// GSM module connections (using HardwareSerial)
HardwareSerial gsmSerial(1); // Use hardware serial 1 (you can also use 0 or 2)

// ThingSpeak credentials
const char *ssid = "Redmi 13C 5G";  // Wi-Fi SSID
const char *password = "harii2k2";  // Wi-Fi password
unsigned long myChannelNumber = 2789086;  // ThingSpeak Channel Number
const char *myWriteAPIKey = "R5MHOJUT0D1Q3ZLP";  // ThingSpeak Write API Key

// LCD settings
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD address 0x27, 16 columns, 2 rows

// WiFi and ThingSpeak variables
WiFiClient client;

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  gsmSerial.begin(9600, SERIAL_8N1, 16, 17);  // Start GSM serial (RX on GPIO16, TX on GPIO17)

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setBacklight(1);
  lcd.setCursor(0, 0);
  lcd.print("Water Quality");

  // Connect to Wi-Fi
  WiFi.begin("Redmi 13C 5G", "harii2k2");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
  delay(1000);
}

void loop() {
  // Read sensor values
  int tdsValue = analogRead(TDS_PIN);
  int turbidityValue = analogRead(TURBIDITY_PIN);
  int orpValue = analogRead(ORP_PIN);
  int phValue = analogRead(PH_PIN);

  // Map the sensor readings to their respective ranges
  float tds = map(tdsValue, 0, 4095, 0, 1000);  // TDS range can vary
  float turbidity = map(turbidityValue, 0, 4095, 0, 1000);  // Adjust ranges
  float orp = map(orpValue, 0, 4095, -500, 500);  // Adjust range for ORP
  float ph = map(phValue, 0, 4095, 0, 14);  // pH range 0-14
  
  // Display values on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TDS: " + String(tds) + "ppm");
  lcd.setCursor(0, 1);
  lcd.print("pH: " + String(ph));

  // Send data to ThingSpeak
  ThingSpeak.setField(1, tds);
  ThingSpeak.setField(2, turbidity);
  ThingSpeak.setField(3, orp);
  ThingSpeak.setField(4, ph);
  ThingSpeak.writeFields(2789086, "R5MHOJUT0D1Q3ZLP");

  // Send alert via GSM if values are out of range
  if (tds > 500 || turbidity > 500 || ph < 6 || ph > 8) {
    sendAlert(tds, turbidity, orp, ph);
  }

  delay(20000);  // Wait 20 seconds before next reading
}

// Function to send SMS alert using GSM module
void sendAlert(float tds, float turbidity, float orp, float ph) {
  gsmSerial.println("AT+CMGF=1");  // Set SMS format to text mode
  delay(1000);
  gsmSerial.println("AT+CMGS=\"+1234567890\"");  // Replace with your phone number
  delay(1000);
  gsmSerial.println("Alert: Water quality parameters out of range!");
  gsmSerial.println("TDS: " + String(tds) + " ppm");
  gsmSerial.println("Turbidity: " + String(turbidity));
  gsmSerial.println("pH: " + String(ph));
  gsmSerial.println("ORP: " + String(orp));
  delay(1000);
  gsmSerial.write(26);  // Send the message
  delay(5000);  // Wait for the message to be sent
}
