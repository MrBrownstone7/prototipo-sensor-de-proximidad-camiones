#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>

// Pines RFID
#define SS_PIN  5
#define RST_PIN 15 // Cambiado para evitar conflictos

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configuración Red y Nube
const char* SSID = "Wokwi-GUEST";
const char* PASS = "";
unsigned long myChannelNumber = 3312062; // Pon tu ID de ThingSpeak
const char* myWriteAPIKey = "CP1L05BI81OPWN77";     // Pon tu API Key

WiFiClient client;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  
  lcd.init();
  lcd.backlight();
  
  pinMode(2, OUTPUT); // LED Verde
  pinMode(4, OUTPUT); // LED Rojo
  
  digitalWrite(4, HIGH); // Rojo encendido (Standby)
  
  lcd.print("Iniciando IoT...");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  
  ThingSpeak.begin(client);
  lcd.clear();
  lcd.print("Sistema Listo");
  delay(2000);
  lcd.clear();
  lcd.print("Acerque Tag RFID");
}

void loop() {
  // Resetear estado si no hay tarjeta
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Leer el ID de la tarjeta
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  lcd.clear();
  lcd.setCursor(0, 0);

  // Lógica de "Base de Datos" (RE-01)
  String patente = "";
  int idCamion = 0;

  if (uid == "11223344") { // ID de ejemplo
    patente = "ABCD-12";
    idCamion = 1;
  } else {
    patente = "XYZW-99"; // Cualquier otro tag
    idCamion = 2;
  }

  // Interfaz de Usuario (RF-02)
  lcd.print("Camion: " + patente);
  lcd.setCursor(0, 1);
  lcd.print("Registrando...");
  
  // Feedback Visual
  digitalWrite(4, LOW);  // Apaga rojo
  digitalWrite(2, HIGH); // Prende verde

  // Envío a ThingSpeak (RF-03)
  ThingSpeak.setField(1, 1);        // Presencia detectada
  ThingSpeak.setField(2, idCamion); // ID de la patente
  int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(response == 200) {
    lcd.setCursor(0, 1);
    lcd.print("Dato en la Nube ");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Error en Nube   ");
  }

  delay(5000); // Espera para que el usuario lea el LCD
  
  // Volver a estado de espera
  digitalWrite(2, LOW);
  digitalWrite(4, HIGH);
  lcd.clear();
  lcd.print("Acerque Tag RFID");
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
