/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>

#include <SoftwareSerial.h>

SoftwareSerial rgbSerial(10, 11); // RX, TX

// Replace with your network credentials
const char* ssid = "WLAN-578260";
const char* password = "32272095940271487760";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;

byte redValue = 0, greenValue = 100, blueValue = 100;
bool hasChangedColors = false;

//------ Serial

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  // Connect to Wi-Fi network with SSID and password
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
}

void loop(){
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
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
            }


          if(header.indexOf("color-") >=0){
            int firstIndexOfColor(header.indexOf("color-"));
            int lastIndexOfColor(header.indexOf("-color"));
            String colorUrl = header.substring(firstIndexOfColor + 6, lastIndexOfColor);
            extractColorValues(colorUrl);
            Serial.println("ColorUrl: " + colorUrl);
            hasChangedColors = true;
          }else{
            hasChangedColors = false;
          }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");

            String customStyle = ".color-display {\n"            
                                  "width: 100px;\n"
                                  "height: 100px;\n"
                                  "margin: auto;\n"
                                  "border: 1px solid #000;\n"
                                  "}\n";
            client.println(customStyle);
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Alexanders LED Controller!</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

//-------------> Hier kommt mein shit rein

            String submitColorScript = "<script>\n"
            "function redirect(){\n"
            "var red = document.getElementById(\"red\").value;\n"
            "var green = document.getElementById(\"green\").value;\n"
            "var blue = document.getElementById(\"blue\").value;\n"
            "var currentUrl = window.location.href;\n"
            "location.href = '/color-r' + red + 'g' + green + 'b' + blue + '-color';\n"
            "}\n"


            "document.addEventListener(\"DOMContentLoaded\", function() {\n"
            "const redSlider = document.getElementById(\"red\");\n"
            "const greenSlider = document.getElementById(\"green\");\n"
            "const blueSlider = document.getElementById(\"blue\");\n"
            "const colorDisplay = document.querySelector(\".color-display\");\n"
  
            "function updateColor() {\n"
              "const redValue = redSlider.value / 100;\n"
              "const greenValue = greenSlider.value / 100;\n"
              "const blueValue = blueSlider.value / 100;\n"
  
              "const color = `rgb(${redValue * 255}, ${greenValue * 255}, ${blueValue * 255})`;\n"
              "colorDisplay.style.backgroundColor = color;\n"
            "}\n"
  
            "redSlider.addEventListener(\"input\", updateColor);\n"
            "greenSlider.addEventListener(\"input\", updateColor);\n"
            "blueSlider.addEventListener(\"input\", updateColor);\n"
  
           "// Initial update\n"
            "updateColor();\n"
          "});\n"
            "</script> \n";

            client.println(submitColorScript);

            String redSlider = "<div class=\"slidecontainerRed\">" 
                    "Rot:"
                    "<input type=\"range\" min=\"0\" max=\"100\" value=\""+ String(redValue) + "\" class=\"slider\" id=\"red\">" "%" 
                    "</div>";
            client.println(redSlider);
              
            String greenSlider = "<div class=\"slidecontainerGreen\">" 
                    "Green:"
                    "<input type=\"range\" min=\"0\" max=\"100\" value=\""+ String(greenValue) + "\" class=\"slider\" id=\"green\">" "%" 
                    "</div>";
            client.println(greenSlider);

                                   
            String blueSlider = "<div class=\"slidecontainerBlue\">" 
                    "Blau:"
                    "<input type=\"range\" min=\"0\" max=\"100\" value=\""+ String(blueValue) + "\" class=\"slider\" id=\"blue\">" "%" 
                    "</div>";
            client.println(blueSlider);

            String submitButton = "<button type=\"button\" onclick = \"redirect()\"> OK </button>";
            client.println(submitButton);


            client.println("<br\\>");
            client.println("<div class=\"color-display\"></div>");

            
            
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


if(hasChangedColors){
  setRedLight(redValue);
  setGreenLight(greenValue);
  setBlueLight(blueValue);
  }
}



void extractColorValues(String input) {
  // Durchlaufe den String und extrahiere die Werte nach r, g und b
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == 'r') {
      redValue = input.substring(i + 1).toInt();
    } else if (input.charAt(i) == 'g') {
      greenValue = input.substring(i + 1).toInt();
    } else if (input.charAt(i) == 'b') {
      blueValue = input.substring(i + 1).toInt();
    }
  }

  // Gib die extrahierten Werte aus
  Serial.print("Rot: ");
  Serial.println(redValue);
  Serial.print("Grün: ");
  Serial.println(greenValue);
  Serial.print("Blau: ");
  Serial.println(blueValue);
}

void sendData(byte data1, byte data2, byte data3) {
  byte checksum = data1 + data2 + data3; // Berechne die Checksumme
  rgbSerial.write(0x1F); // Startbyte
  rgbSerial.write(data1);
  rgbSerial.write(data2);
  rgbSerial.write(data3);
  rgbSerial.write(checksum); // Checksumme senden
}

void setAutoAdjustLight() {
  sendData(0x00, 0x00, 0x1F); // Beispiel: Automatische Anpassung der Helligkeit
}

void setRedLight(byte luminance) {
  sendData(0x1, luminance, 0x21); // Beispiel: Rote Lampe, 1% Helligkeit
}

void setGreenLight(byte luminance) {
  sendData(0x02, luminance, 0x22); // Beispiel: Grüne Lampe, Helligkeit variiert
}

void setBlueLight(byte luminance) {
  sendData(0x03, luminance, 0x23); // Beispiel: Blaue Lampe, Helligkeit variiert
}
