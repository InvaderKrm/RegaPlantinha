// Importe as bibliotecas necessárias
#include <Arduino.h>
#define N_SENSORES 4
#define TEMPO_ESPERA 5000
#define TEMPO_DISPARO 1000
// Definição do pino analógico
const int analogPin[N_SENSORES] = {34,35,32,33};
int level[N_SENSORES] = {4096/2,4096/2,4096/2,4096/2};
// Definição do pino do LED
const int ledPin[N_SENSORES] = {5,4,2,15};
int tempo[N_SENSORES];
int espera[N_SENSORES];

void leitor() {
    int analogValue;
  // Lê o valor do pino analógico
  for(int i=0;i<N_SENSORES;i++){
    if (espera[i]==0) {
      espera[i]=TEMPO_ESPERA;
      analogValue = analogRead(analogPin[i]);
      if (analogValue > level[i]) tempo[i]=TEMPO_DISPARO;
    }
  }
}

void contador() {
  for(int i=0;i<N_SENSORES;i++) {
   if (tempo[i]) tempo[i]--;
   if (espera[i]) espera[i]--;
  }
  delay (1);
}

void disparo() {
  for(int i=0;i<N_SENSORES;i++) {
    if (tempo[i]) digitalWrite(ledPin[i], HIGH);
    else digitalWrite(ledPin[i], LOW);
  }
}

// Função de inicialização
void setup() {
  // Inicializa a serial
  Serial.begin(115200);
  for(int i=0;i<N_SENSORES;i++){
    pinMode(analogPin[i], INPUT);
    pinMode(ledPin[i], OUTPUT);
  }
}

// Função de loop
void loop() {
  disparo();
  leitor();
  contador();
}