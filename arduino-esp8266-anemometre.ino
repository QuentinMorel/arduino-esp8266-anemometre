#include "WiFiCreds.h"
#include "dataServer.h"
#include "ESP8266WiFi.h" 
#include <PubSubClient.h>

const byte AnemometrePin        = D7;    // PIN de connexion de l'anémomêtre.
unsigned int anemometreCnt      = 0;     // Initialisation du compteur.
unsigned long lastSendVent      = 0;     // Millis du dernier envoi (permet de récupérer l'intervale réel, et non la valeur de souhait).
unsigned long t_lastActionVent  = 0;     // enregistre le Time de la dernière intérogation des capteurs vent.
unsigned long t_lastRafaleVent  = 0;     // Enregistre le Time du dernier relevé de Rafale de vent.
unsigned int rafalecnt          = 0;     // Compteur pour le calcul des rafales de vent.
unsigned int anemometreOld      = 0;     // Mise en mémoire du relevé "anemometreCnt" pour calcul de Rafale.
#define INTERO_VENT 10                  // Valeur de l'intervale en secondes entre 2 relevés des capteurs Vent.
#define INTERO_RAF 2                  // Valeur de l'intervale en secondes entre 2 relevés des capteurs Vent pour les rafales.

char vitesseVentStr[10];
char vitesseRafaleStr[10];

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
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
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

void setup() {
  Serial.begin(115200);
  Serial.println("Initialisation...");
  setup_wifi();
  delay(2000);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(2000);
  
  // Initialisation du PIN et création de l'interruption.
  pinMode(AnemometrePin, INPUT_PULLUP);          // Montage PullUp avec Condensateur pour éviter l'éffet rebond.
  attachInterrupt(digitalPinToInterrupt(AnemometrePin), cntAnemometre, FALLING); // CHANGE: Déclenche de HIGH à LOW ou de LOW à HIGH - FALLING : HIGH à LOW - RISING : LOW à HIGH.

   // On initialise lastSend à maintenant.
   lastSendVent = millis();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // On reléve la rafale de vent des 5 dernières secondes.
  if (millis() - t_lastRafaleVent >= (INTERO_RAF * 1000)) {
    // On met à jour la valeur du dernier traitement de Rafale de vent à maintenant.
    t_lastRafaleVent = millis();
    
    // On a atteint l'interval souhaité, on exécute le traitement Vent.
    getRafale();
  }
  
  // On vérifie si l'intervale d'envoi des informations "Vent" sont atteintes. (120s)
  if (millis() - t_lastActionVent >= (INTERO_VENT * 1000)) {
    // On met à jour la valeur du dernier traitement de l'anémometre à maintenant.
    t_lastActionVent = millis();
    
    // On a atteint l'interval souhaité, on exécute le traitement Vent.
    getSendVitesseVent();
  }
}

void getSendVitesseVent() {
  // On effectue le calcul de la vitesse du vent.
  // Serial.println("Execution de la fonction getSendVitesseVent().");
  // On initialise lastSendVent à maintenant.
  int temps_sec = (millis() - lastSendVent) / 1000;
  lastSendVent = millis();
  // Serial.print("Temps pour le calcul = "); Serial.print(temps_sec); Serial.println(" sec");
  // Serial.print("Nombre de déclenchement = "); Serial.println(anemometreCnt);

  // On calcul la vitesse du vent.
  float vitesseVent = (((float)anemometreCnt / 2) / (float)temps_sec) * 2.4;    // Vitesse du vent en km/h = (Nbre de tour / temps de comptage en sec) * 2,4
  vitesseVent       = round(vitesseVent * 10) / 10;                             // On tranforme la vitesse du vent à 1 décimale.

  // On calcul la vitesse de la rafale.
  float vitesseRafale = (((float)rafalecnt / 2) / INTERO_RAF) * 2.4;                     // Vitesse du vent en km/h = (Nbre de tour / temps de comptage en sec) * 2,4
  vitesseRafale       = round(vitesseRafale * 10) / 10;                         // On tranforme la vitesse du vent à 1 décimale.

  // On réinitialise les compteurs.
  anemometreCnt     = 0;                                          // Envoi des données, On réinitialise le compteur de vent à 0.
  anemometreOld     = 0;                                          // Envoi des données, On réinitialise le compteur de mémoire à 0.
  rafalecnt         = 0;                                          // Envoi des données, On réinitialise le compteur de Rafale à 0.

  // On affiche la vitesse du vent.
  Serial.print("Vitesse du vent = "); Serial.println(vitesseVent,1); 
  Serial.print("Rafale du vent = "); Serial.println(vitesseRafale,1); 

  // send mqtt message
  dtostrf(vitesseVent, 1, 1, vitesseVentStr);
  dtostrf(vitesseRafale, 1, 1, vitesseRafaleStr);
  client.publish("esp32/wind_sensor", vitesseVentStr, true);
  client.publish("esp32/wind_sensor_rafale", vitesseRafaleStr, true);
}

void getRafale() {
  // Relevé et comparaison du relevé précédent.
  // Serial.println("Execution de la fonction getRafale().");
  // On calcul le nombre d'impulsion sur les 5 dernières secondes.
  int compteur = anemometreCnt - anemometreOld;
  // On vérifie si la rafale est supérieure à la précédente.
  // Serial.print(compteur); Serial.print(" = "); Serial.print(anemometreCnt); Serial.print(" - "); Serial.println(anemometreOld); 
  // On stock la nouvelle valeur comme étant l'ancienne pour le prochain traitement.
  anemometreOld = anemometreCnt;
  // On vérifie si la rafale est supérieure à la précédente.
  if (compteur > rafalecnt) {
    // La rafale est supérieure, on enregistre l'information.
    rafalecnt = compteur;
    // Serial.print("Nouvelle valeur de rafale : "); Serial.println(rafalecnt);
  }
}