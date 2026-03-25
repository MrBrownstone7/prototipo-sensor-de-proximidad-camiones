#include <WiFi.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>

const char* SSID = "Wokwi-GUEST";
const char* PASS = "";
unsigned long myChannelNumber = 3312062; 
const char* myWriteAPIKey = "CP1L05BI81OPWN77";     

WiFiClient client;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int trigPin = 5;
const int echoPin = 18;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  ThingSpeak.begin(client);
  lcd.print("Sistema Listo");
}

void loop() {
  // 1. Lectura del sensor ultrasónico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  // 2. Lógica de detección (Menos de 100cm = Camión presente)
  if (distance < 100) { 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Camion Detectado");
    lcd.setCursor(0, 1);
    lcd.print("Dist: "); lcd.print(distance); lcd.print(" cm");

    digitalWrite(2, HIGH); // Enciende LED Verde (Acceso)
    digitalWrite(4, LOW);  // Apaga LED Rojo

    // Envío a la nube (RF-03) 
    ThingSpeak.writeField(myChannelNumber, 1, 1, myWriteAPIKey); 
    
    // ThingSpeak necesita tiempo entre envíos, pero para que el LCD 
    // no se congele 15s, usaremos un delay más corto en la simulación
    delay(5000); 
  } 
  // 3. Lógica cuando NO hay camión (Más de 100cm)
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Esperando Camion");
    lcd.setCursor(0, 1);
    lcd.print("Acceso Libre");

    digitalWrite(2, LOW);  // Apaga LED Verde
    digitalWrite(4, HIGH); // Enciende LED Rojo (Standby/Pare)
    
    delay(500); // Refresco rápido para detectar ingresos nuevos
  }
}
