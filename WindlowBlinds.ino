#include <ESP8266WiFi.h>
#include <Servo.h>
/*
    The server sets a GPIO pin depending on the request
    server_ip is the IP address of the ESP8266 module
*/

Servo servo; //initialise servo object

int analogIn = A0; // set photoresistor read pin
int ledPin1 = D1; // set led1 pin
int ledPin2 = D2; // set led2 pin
int ledPin3 = D3; // set led3 pin
int ledPin4 = D4; // set led4 pin
int servoPin = D7; // set servo pin
int servoState = LOW; //flag for knowing the status of the servo
int automatic = 0;//flag for setting the automatic operation
int bstatus = 0;//flag for knowing the status of the blind
int light = 0;//flag for knowing the status of the lights
int ivalue = 600;


const char* ssid = "X";// Wi-Fi ssid to connect to
const char* password = "motosodope";//Wi-Fi password

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  delay(10);
  servoState = LOW; // Variable inits
  pinMode(servoPin, OUTPUT); //Init Servo output
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  servo.attach(servoPin); //attach servo to output
  servo.writeMicroseconds(1500);//initiate servo -- 1500 puts it in the stop position

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
  while (!client.available())
  {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Find which commmand was sent(in case there was a command sent)
  bstatus = chkCmd(request);


  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Refresh: 5"); //auto-refresh page every 5 seconds
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  //Update the page about the current status of the blinds
  client.print("<center><p>SMART BLINDS MODE:<b> ");
  if (bstatus == 3)
  {
    client.print("<font color=&quot;#66ff33;&quot;>AUTO</font>");
  }
  else
  {
    client.print("<font color=&quot;#66ff33;&quot;>MANUAL</font>");
  }

  //HTML code served by the server running on the ESP8266
  client.println("</b></p>");
  client.println("<a href=\"?cmd=OPEN_BLINDS\"><button>OPEN</button></a>");
  client.println("<a href=\"?cmd=CLOSE_BLINDS\"><button>CLOSE</button></a><br><br>");
  client.println("<a href=\"?cmd=LIGHT_ON\"><button>LIGHT ON</button></a>");
  client.println("<a href=\"?cmd=LIGHT_OFF\"><button>LIGHT OFF</button></a><br><br>");
  client.println("<a href=\"?cmd=AUTO_ON\"><button>AUTO ON</button></a>");
  client.println("<a href=\"?cmd=AUTO_OFF\"><button>AUTO OFF</button></a>");
  client.println("</b></p>");
  client.println("<p>BLINDS STATUS:<b> ");
  if ((servoState != 65) and (servoState != 0))//servo state 65 = close and 0 = initiate
  {
    client.println("<font color=&quot;#66ff33;&quot;>OPEN</font>");
  }
  else if ((servoState != 95) and (servoState != 0))//servo state 95 = open
  {
    client.println("CLOSE");
  }
  else
  {
    client.println(servoState);
  }
  client.println("</b></p>");

  client.println("<p>LIGHTS:<b> ");
  if (light == 1)
  {
    client.println("<font color=&quot;#66ff33;&quot;>ON</font>");
  }
  else
  {
    client.println("OFF");
  }
  client.println("</b></p>");

  client.println("<p>LUMINOSITY:<b> ");
  client.println(analogRead(analogIn));
  client.println("</b></p>");

  client.println("<a href=\"?cmd=RELOAD_PHOTOCELL\"><button>CHECK LUMINOSITY</button></a></center>");
  client.println("</html>");
  delay(1);
  Serial.println("CLIENT DISCONNECTED");
  Serial.println("");
}



//execute the command string
int chkCmd(String request)
{
  if ((request.indexOf("cmd=OPEN_BLINDS") != -1) and (servoState != 95))
  {
    automatic = 0;//disable auto mode
    openb();//open the blinds
    bstatus = 1;//update the blinds status
  }
  else if ((request.indexOf("cmd=CLOSE_BLINDS") != -1)  and (servoState != 65))
  {
    automatic = 0;//disable auto mode
    closeb();//close the blinds
    bstatus = 0;//update the blinds status
  }
  else if (request.indexOf("cmd=AUTO_OFF") != -1)
  {
    automatic = 0;//disable auto mode
    boff();
    bstatus = 2;//update the blinds status
  }
  else if (request.indexOf("AUTO_ON_VALUE") != -1)
  {
    automatic = 1;
    bstatus = 3;
    int len = request.length();
    String svalue = (request.substring(len-12,len-9));
    int ivalue = svalue.toInt();
    automate(ivalue);
    delay(5);
  }
  else if (request.indexOf("cmd=AUTO_ON") != -1)
  {
    automatic = 1;//enable auto mode
    bstatus = 3;//update the blinds status
    automate(ivalue);//initiate automatic operation
    delay(5);
  }
  else if (request.indexOf("cmd=LIGHT_ON") != -1)
  {
    lighton();
  }
  else if (request.indexOf("cmd=LIGHT_OFF") != -1)
  {
    lightoff();
  }
  return bstatus;
}

void lighton()
{
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, HIGH);
  digitalWrite(ledPin3, HIGH);
  digitalWrite(ledPin4, HIGH);
  light = 1;
}

void lightoff()
{
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
  digitalWrite(ledPin4, LOW);
  light = 0;
}

void openb()
{
  servoState = 95;
  servo.writeMicroseconds(1300);  // Update the servo based on servoState (1300 = right)
  delay(5000);
  servo.writeMicroseconds(1500); // Stop servo
  Serial.println(servoState);
}

void closeb()
{
  servoState = 65;
  servo.writeMicroseconds(1700);  // Update the servo based on servoState (1700 = left)
  delay(5000);
  servo.writeMicroseconds(1500);  // Stop servo
  Serial.println(servoState);
}

void boff()
{
  // Update the servo based on servoState
  servo.writeMicroseconds(1500); // Stop servo
}

void automate(int ivalue)
{
  while (automatic)
  {
    Serial.println(analogRead(analogIn));
    if ((analogRead(analogIn)) > ivalue)
    {
      if (servoState == 65 or servoState == 0)
      {
        openb();
        lightoff();
      }
      else
      {
        boff();
        loop();
      }
    }
    else if ((analogRead(analogIn)) < ivalue)
    {
      if (servoState == 95 or servoState == 0)
      {
        closeb();
        lighton();
      }
      else
      {
        boff();
        loop();
      }
    }
    delay(1);
  }

}
