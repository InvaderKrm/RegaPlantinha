
#include <WiFi.h>
#include <ESPmDNS.h>

// Replace with your network credentials
const char* ssid = "kramm2@148";
const char* password = "5134992001";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


#define N_SENSORES 4
#define TEMPO_ESPERA 5000
#define TEMPO_DISPARO 1000
// Definição do pino analógico
const int analogPin[N_SENSORES] = {34,35,32,33};
int level[N_SENSORES] = {50,50,50,50};
// Definição do pino do LED
const int ledPin[N_SENSORES] = {5,4,2,15};
int tempo[N_SENSORES];
int espera[N_SENSORES];
int humidade[N_SENSORES];
String plantas[N_SENSORES] = {"Alface", "Cebolinha", "Morango", "Tempero Verde"};
String imagem[N_SENSORES] = {"2hvcmtx","Yz8B589","TUfOnKV","RRQoJCq"};

void leitor() {
  // Lê o valor do pino analógico
  for(int i=0;i<N_SENSORES;i++){
    if (espera[i]==0) {
      espera[i]=TEMPO_ESPERA;
      float temp = analogRead(analogPin[i]);
			temp = temp*100/4096;
			//converte em percentual
			humidade[i] = int(temp);
      if (humidade[i] > level[i]) tempo[i]=TEMPO_DISPARO;
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




void setup() {
	Serial.begin(115200);
	// Initialize the output variables as outputs
  for(int i=0;i<N_SENSORES;i++){
    pinMode(analogPin[i], INPUT_PULLUP);
    pinMode(ledPin[i], OUTPUT);
  }

	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	// Print local IP address and start web server
	Serial.println("");
	Serial.println("WiFi connected.");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	server.begin();

	if(!MDNS.begin("plantinha")) {
		Serial.println("Error starting mDNS");
		return;
	}
}

void minhaPagina(){
	WiFiClient client = server.available();   // Listen for incoming clients

	if (client) {                             // If a new client connects,
		currentTime = millis();
		previousTime = currentTime;
		Serial.println("New Client.");          // print a message out in the serial port
		String currentLine = "";                // make a String to hold incoming data from the client
		while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
			currentTime = millis();
			if (client.available()) {             // if there's bytes to read from the client,
				char c = client.read();             // read a byte, then
				Serial.write(c);                    // print it out the serial monitor
				header += c;
				if (c == '\n') {                    // if the byte is a newline character
					// if the current line is blank, you got two newline characters in a row.
					// that's the end of the client HTTP request, so send a response:
					if (currentLine.length() == 0) {
						// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
						// and a content-type so the client knows what's coming, then a blank line:

            client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println("Connection: close");
						client.println();
						
						// turns the GPIOs on and off
						for(int i=0;i<N_SENSORES;i++){
							if (header.indexOf("GET /"+String(i+1)+"/on") >= 0)				tempo[i]=TEMPO_DISPARO;
							else if (header.indexOf("GET /"+String(i+1)+"/off") >= 0)	tempo[i]=0;
						}
						
						
						// Display the HTML web page
						client.println("<!DOCTYPE html><html>");
						client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
						client.println("<link rel=\"icon\" href=\"data:,\">");
						// CSS to style the on/off buttons 
						// Feel free to change the background-color and font-size attributes to fit your preferences
						client.println("<style>");
						client.println(".card {background-color: rgb(77, 163, 106); width: 200px; height: 310px; padding: 25px; border-radius: 12px; float: left; margin: 10px;}");
            client.println("button {background-color: rgb(141, 206, 148); border-radius: 12px; color: white; padding: 9px 87px; font-size: 25px; cursor: pointer;}");
            client.println("p {font-family: Helvetica; font-size: 22px; margin-top: 1px; margin-bottom: 1px; color: rgb(255, 255, 255);}");
            client.println("img {width: 196px; border-radius: 8px;}");
            client.println("h1 {font-family: helvetica; margin-left: 10px;}");
            client.println("h3 {font-family: Helvetica; margin-bottom: 2px; margin-top: -3px; color: rgb(255, 255, 255); font-size: 23px;}");
						client.println("</style></head>");
						
						// Web Page Heading
						client.println("<body><h1>Regador de Plantinhas</h1>");
						
						// Display current state, and ON/OFF buttons for i
						for(int i=0;i<N_SENSORES;i++){
              client.println("<div class=\"card\">");
              client.println("<h3>"+plantas[i]+"</h3>");
              client.println("<img src=\"https://i.imgur.com/"+imagem[i]+".png\">");
              client.println("<p><b>"+String(humidade[i])+"% de umidade</b></p>");
							client.println("<a href=\"/"+String(i+1)+"/on\"><button class=\"button button"+String(i+1) +"\">&#9654;</button></a>");
              client.println("</div>");
						}
						client.println("</body></html>");


						// The HTTP response ends with another blank line
						client.println();
						// Break out of the while loop
						break;
					} else { // if you got a newline, then clear currentLine
						currentLine = "";
					}
				} else if (c != '\r') {  // if you got anything else but a carriage return character,
					currentLine += c;      // add it to the end of the currentLine
				}
			}
		}
		// Clear the header variable
		header = "";
		// Close the connection
		client.stop();
		Serial.println("Client disconnected.");
		Serial.println("");
	}
}



void loop() {
	minhaPagina();
	disparo();
	leitor();
	contador();
}