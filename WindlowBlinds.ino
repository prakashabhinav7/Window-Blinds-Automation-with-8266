#include <ESP8266WiFi.h>
#include <Servo.h>

/*
 *  The server sets a GPIO pin depending on the request
 *  server_ip is the IP address of the ESP8266 module
 */
 
Servo servo;
int servoPin = D7; // GPIO13
int analogIn = A0; // GPIO13
int servoState = LOW;

const char* ssid = "Skynet";
const char* password = "dinkeywheat"; 

WiFiServer server(80);
 
void setup() 
{
  Serial.begin(115200);
  delay(10);
  servoState = LOW; // Variable inits
  pinMode(servoPin, OUTPUT); //Init LED output
  servo.attach(servoPin); //attach servo to output
  servo.write(80);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the server IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}
 
void loop() 
{
  // Test to check if a new client has connected
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
 
  // Wait for the client to send data
  Serial.println("new client");
  while(!client.available())
  {
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Find which commmand was sent(in case there was a command sent
  chkCmd(request);

  
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  // client.println("Refresh: 5"); //auto-refresh page every 5 seconds
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  //Update the page about the current status of the blinds
  client.print("Blinds are now: ");
  if(servoState == 95) 
  {
    client.print("OPEN");
  } 
  else if(servoState == 65) 
  {
    client.print("CLOSE");
  } 
  else 
  {
    client.print("OFF");
  }
  client.println("<br><br>");
  client.println("<a href=\"?cmd=OPEN_BLINDS\"><button>OPEN</button></a>");
  client.println("<a href=\"?cmd=CLOSE_BLINDS\"><button>CLOSE</button></a>");
  client.println("<a href=\"?cmd=TURN_OFF_SERVO\"><button>OFF</button></a>");
  client.println("<a href=\"?cmd=AUTO_ON\"><button>AUTO</button></a>");
  client.println("<p>The photocell's value is:<b> ");
  client.println(analogRead(analogIn)); 
  client.println("</b></p>");
  client.println("<a href=\"?cmd=RELOAD_PHOTOCELL\"><button>CHECK LUMINOSITY</button></a>");
  client.println("</html>");
  
  delay(1);
  Serial.println("CLIENT DISCONNECTED");
  Serial.println("");
}



//execute the command string
void chkCmd(String request)
{
  if (request.indexOf("cmd=OPEN_BLINDS") != -1)
  {
    openb();//open the blinds
  }
  else if (request.indexOf("cmd=CLOSE_BLINDS") != -1)
  {
    closeb();//close the blinds
  }
  else if (request.indexOf("cmd=TURN_OFF_SERVO") != -1)
  {
    boff();//switch off the motor 
  }
  else if (request.indexOf("cmd=AUTO_ON") != -1)
  {
    automate();//initiate automatic operation
    delay(5);
  }
  
}

void openb()
{
  servoState = 95;
  servo.write(servoState);  // Update the servo based on servoState
  delay(5000);
  servo.write(80); // Stop servo
  Serial.println(servoState);
}

void closeb()
{
  servoState = 65;
  servo.write(servoState);// Update the servo based on servoState
  delay(5000);
  servo.write(80);  // Stop servo
  Serial.println(servoState);
}

void boff()
{
  // Update the servo based on servoState
  servo.write(80);
}

void automate()
{
  int i = 0;
  while (i < 1) 
  {
    Serial.println(analogRead(analogIn));
//    Serial.println(servoState);
    if((analogRead(analogIn))>900)
    {
      if(servoState == 65)
      {
        openb();
        i++;
      }
      else
      {
        boff();
      } 
    }
    else if ((analogRead(analogIn))<600)
    {
      if(servoState == 95)
      {
        closeb();
        i++;
      }
      else
      {
        boff();
      } 
    } 
    delay(10);
  }
  
}
