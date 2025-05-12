#include <WiFiNINA.h>
#include <Servo.h>

Servo steering;
int angle[3] = {70, 100, 130};
int anglestep = 1;
bool stepA = false;
bool stepD = false;

int enable2 = 3;
int potVal;
int in3 = 6;
int in4 = 5;

char ssid[] = "Tidskapseln";     //  your network SSID (name) between the " "
char pass[] = "Gollum sover";  // your network password between the " "
int keyIndex = 0;               // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;    //connection status
WiFiServer server(80);          //server socket

WiFiClient client = server.available();

int speed = 0;
bool Wpress = false;
bool Spress = false;
bool backw = false; 

void setup(){

  steering.attach(9);
  pinMode(enable2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  Serial.begin(9600);

  while (!Serial)
    ;

  enable_WiFi();
  connect_WiFi();

  server.begin();
  printWifiStatus();
  Serial.println("HejHej");

}

void loop(){

  client = server.available();

  if (client) {
    printWEB();
  }

  //Serial.println(potVal);
  //Serial.println(speed);

  if (Spress == false && Wpress == false) {
    speed = 0;
  } else {
    speed = 255;
  }

  analogWrite(enable2, speed); // Any value between 0 and 255
  
  if (backw) {
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  } else {
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }

  if (stepA) {
    if (anglestep > 0){
      anglestep -= 1;
    }
  }
  stepA= false;
  if (stepD) {
    if (anglestep < 2){
      anglestep += 1;
    }
  }
  stepD = false;
  
  steering.write(angle[anglestep]);
  delay(100);

}




void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
}

void printWEB() {

  if (client) {                    // if you get a client,
    Serial.println("new client");  // print a message out the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected()) {   // loop while the client's connected
      if (client.available()) {    // if there's bytes to read from the client,
        char c = client.read();    // read a byte, then
        Serial.write(c);           // print it out the serial monitor
        if (c == '\n') {           // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {

            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            //create the buttons
            //client.print("Click <a href=\"/H\">here</a> turn the LED on<br>");
            //client.print("Click <a href=\"/L\">here</a> turn the LED off<br><br>");
            //client.print("<input type="range" min="1" max="100" value="50"");
            
            client.print("<html><body>");
            client.print("<h2>Press 'W' to drive forward, 'S' to drive backward, 'A' to steer right, 'D' to steer left</h2>");

            client.print("<script>");
            client.print("document.addEventListener('keydown', function(event) {");
            client.print("  if (event.key === 'W' || event.key === 'w') {");
            client.print("    window.location.href = '/W';");
            client.print("  }");
            client.print("  if (event.key === 'S' || event.key === 's') {");
            client.print("    window.location.href = '/S';");
            client.print("  }");
            client.print("  if (event.key === 'A' || event.key === 'a') {");
            client.print("    window.location.href = '/A';");
            client.print("  }");
            client.print("  if (event.key === 'D' || event.key === 'd') {");
            client.print("    window.location.href = '/D';");
            client.print("  }");
            client.print("});");
            client.print("</script>");

            client.print("</body></html>");

            int randomReading = analogRead(A1);
            client.print("Random reading from analog pin: ");
            client.print(randomReading);

            client.println("Speed: ");
            client.print(speed);
            client.println("Wpress: ");
            client.print(Wpress);
            client.println("Spress: ");
            client.print(Spress);
            client.println("Direction: ");
            client.print(angle[anglestep]);
            client.println("Anglestep: ");
            client.print(anglestep);





            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        if (currentLine.endsWith("GET /W")) {
          Wpress = !Wpress;
          backw = false;
        }
        if (currentLine.endsWith("GET /S")) {
          Spress = !Spress;
          backw = true;
        }
        if (currentLine.endsWith("GET /A")) {
          stepA = true;
        }
        if (currentLine.endsWith("GET /D")) {
          stepD = true;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}