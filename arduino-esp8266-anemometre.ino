#include "WiFiCreds.h"
#include "dataServer.h"
#include "ESP8266WiFi.h" 
#include <PubSubClient.h>

// Power Pin: pour alimenter les capteurs uniquement pour la prise de mesure
#define powerPinAnemometre D8
#define powerPinRain D6

// Suivi du voltage des la pile
#define analogPin A0 
int battery = 0;
float batteryPct = 0;
char batteryPctStr[15];

// Anemometre
#define anemometrePin D7    // PIN de connexion de l'anémomêtre.
unsigned int timeMeasureWind = 3; // Valeur de l'intervale en secondes entre 2 relevés des capteurs Vent.
unsigned int anemometreCnt = 0;     // Initialisation du compteur.
unsigned long lastSendVent = 0;     // Millis du dernier envoi (permet de récupérer l'intervale réel, et non la valeur de souhait).
char vitesseVentStr[10];
char tmpStr[10];

// Détecteur de pluie
#define rainPin D5
char rainStatusStr[10];

// Temps de deep sleep entre les mesures
int sleepTimeS = 30; // wait (seconds)

ICACHE_RAM_ATTR void cntAnemometre() {
  anemometreCnt++;
}

WiFiClient espClient;
PubSubClient client(espClient);

// Connect WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connexion OK ");
  Serial.print("=> Addresse IP : ");
  Serial.print(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      // Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/wind_sensor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 
      delay(5000);
    }
  }
}

void readSendBatteryState() {
  if (!client.connected()) {
    reconnect();
  }
  battery = analogRead(analogPin);
  batteryPct = ((battery-820.0)/(1024.0-820.0)*(100.0-0.0)); // interpolation linéaire 1v = 1024 (100%), 0.8v = 820 (0%)
  Serial.print("batterie = "); Serial.println(battery,1);
  Serial.print("% batterie = "); Serial.println(batteryPct,1);
  dtostrf(batteryPct, 1, 0, batteryPctStr);
  client.publish("esp32/pct_batterie", batteryPctStr, true);

  dtostrf(battery, 1, 0, tmpStr);
  client.publish("esp32/tmp", tmpStr, true);
}

//  This function returns the sensor output
void readSendRainSensor() {
  if (!client.connected()) {
    reconnect();
  }
	digitalWrite(powerPinRain, HIGH);	// Turn the sensor ON
	delay(10);
	int rainStatus = digitalRead(rainPin);	// Read the sensor output
	digitalWrite(powerPinRain, LOW);		// Turn the sensor OFF
	Serial.print("Digital Output: ");
	Serial.println(rainStatus);

	// Determine le statut
	if (rainStatus) {
    String("0").toCharArray(rainStatusStr, 15);
		Serial.println("Status: Clear");
    client.publish("esp32/rain_status_str", "Pas de pluie", true);
	} else {
		Serial.println("Status: It's raining");
    client.publish("esp32/rain_status_str", "Pluie", true);
    String("1").toCharArray(rainStatusStr, 15);
	}
  client.publish("esp32/rain_status", rainStatusStr, true);
}

void readSendAnemometer() {
  digitalWrite(powerPinAnemometre, HIGH);	// Turn the sensor ON
	delay(10);

  // Initialisation du PIN et création de l'interruption.
  pinMode(anemometrePin, INPUT_PULLUP);          // Montage PullUp avec Condensateur pour éviter l'éffet rebond.
  attachInterrupt(digitalPinToInterrupt(anemometrePin), cntAnemometre, FALLING); // CHANGE: Déclenche de HIGH à LOW ou de LOW à HIGH - FALLING : HIGH à LOW - RISING : LOW à HIGH.

  lastSendVent = millis();

  delay(timeMeasureWind * 1000);

  int temps_sec = (millis() - lastSendVent) / 1000;
  
  Serial.print("Temps pour le calcul = "); Serial.print(temps_sec); Serial.println(" sec");

  // On calcul la vitesse du vent.
  float vitesseVent = (((float)anemometreCnt) / (float)temps_sec) * 1.75 / 20 * 3.6;    // Vitesse du vent en km/h = (Nbre de tour / temps de comptage en sec) * 2,4
  vitesseVent = round(vitesseVent * 10) / 10;                             // On tranforme la vitesse du vent à 1 décimale.

  digitalWrite(powerPinAnemometre, LOW);	// Turn the sensor OFF
  
  // On réinitialise les compteurs.
  anemometreCnt = 0;                                          // Envoi des données, On réinitialise le compteur de vent à 0.
  
  
  // send mqtt message
  dtostrf(vitesseVent, 1, 0, vitesseVentStr);
  // On affiche la vitesse du vent.
  Serial.print("Vitesse du vent = "); Serial.println(vitesseVent,1);

  if (!client.connected()) {
    reconnect();
  }
  client.publish("esp32/wind_sensor", vitesseVentStr, true);
  
}

void setup() {
  Serial.println("\n\nWake up");
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);
  delay(2000);

  Serial.begin(115200);

  // Alimentation des capteurs anémomètre et pluie sur OFF
  pinMode(powerPinRain, OUTPUT);
	digitalWrite(powerPinRain, LOW);

  pinMode(powerPinAnemometre, OUTPUT);
	digitalWrite(powerPinAnemometre, LOW);

  // Initialisation Wifi et MQTT
  Serial.println("Initialisation...");
  setup_wifi();
  delay(2000);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(2000);
 
  // Initialisation du PIN et création de l'interruption.
  pinMode(anemometrePin, INPUT_PULLUP);          // Montage PullUp avec Condensateur pour éviter l'éffet rebond.
  attachInterrupt(digitalPinToInterrupt(anemometrePin), cntAnemometre, FALLING); // CHANGE: Déclenche de HIGH à LOW ou de LOW à HIGH - FALLING : HIGH à LOW - RISING : LOW à HIGH.

  ///// Batterie: lecture et envoi sur serveur MQTT de l'état de charge
  readSendBatteryState();
  delay(2000);
  
  ////// Anémomètre: lecture et envoi sur serveur MQTT
  readSendAnemometer();
  delay(2000);
  
  ////// Détecteur de pluie: lecture et envoi sur serveur MQTT
  readSendRainSensor();
  delay(2000);

     
  if (!client.connected()) {
    reconnect();
  }
  
  for(int i=0; i<20; i++)
  {
    client.loop();  //Ensure we've sent & received everything
    delay(100);
  }
  
  /* Close MQTT client cleanly */
  client.disconnect();
  WiFi.disconnect( true );
  
  Serial.printf("Sleep for %d seconds\n\n", sleepTimeS);

  delay(2000);

  // convert to microseconds
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop() {
}

