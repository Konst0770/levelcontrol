#include <SoftwareSerial.h>
#include <SPI.h>
//oled
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <AsyncElegantOTA.h>
//dallas
#include <OneWire.h>
#include <DallasTemperature.h>
//web
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <SPIFFS.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <Hash.h>
  #include <FS.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h> 
//#include <ESP8266mDNS.h>
// screen
#define WIRE Wire
#define i2c_Address 0x3c
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
// pump
#define PUMP 13
#define HEAT 1
#define SOUND_VELOCITY 0.0343
// temp
#define ONE_WIRE_BUS 0 // вывод, к которому подключён DS18B20
#define TEMPERATURE_PRECISION 9 // точность измерений (9 ... 12)

const long utcOffsetInSeconds = 36000;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
DeviceAddress Thermometer1;
DeviceAddress Thermometer2;
float temp1 = 0.0; // текущее значение температуры ds
float min_temp1 = 50.0; // минимальное значение температуры ds
float max_temp1 = -50.0; // максимальное значение температуры ds
float temp2 = 0.0; // текущее значение температуры ds
float min_temp2 = 50.0; // минимальное значение температуры ds 
float max_temp2 = -50.0; // максимальное значение температуры ds
float temp3 = 0.0; // текущее значение температуры 401
float min_temp3 = 50.0; // минимальное значение температуры 401
float max_temp3 = -50.0; // максимальное значение температуры 401
// for echo mode
const int echoPin = 12; // D6 pin
const int trigPin = 14; // D5 pin
// for rs232 mode
const int US100_RX = 12; // D6 pin
const int US100_TX = 14; // D5 pin
SoftwareSerial US100Serial(US100_RX, US100_TX);
// D7 pump control
// D8 heat control

char lcd_buffer[16];   

int hours = 0;
int minute = 0;
int cmDist = 0;
int TcmDist = 0;
float Ttemp1 = 0;
float Ttemp2 = 0;
float Ttemp3 = 0;
int flag = 1;
int btnpump = 0;
int btnheat = 0;
unsigned int MSByteDist = 0;
unsigned int LSByteDist = 0;
unsigned int mmDist = 0;
unsigned int low = 50;
unsigned int hi = 30;
unsigned int trig = 0;
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
long duration;
//web
String ptr = "";
AsyncWebServer server(80);
AsyncEventSource events("/events");
DNSServer dns;


const char* PARAM_UP = "inputUp";
const char* PARAM_DW = "inputDw";

// HTML web page to handle 2 input fields (inputString, inputInt, inputFloat)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html lang="en"><head>
  <link rel='icon' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAS0lEQVR42s2SMQ4AIAjE+P+ncSYdasgNXMJgcyIIlVKPIKdvioAXyWBeJmVpqRZKWtj9QWAKZyWll50b8IcL9JUeQF50n28ckyb0ADG8RLwp05YBAAAAAElFTkSuQmCC' type='image/x-png' />
  <title>ESP32 Tank unload</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body 		{
  		font-family: arial,sans-serif;
      		text-align: center;
  		padding: 0px;
     		}
h2 	 	{
    		display: block;
    		font-size: 1.3em;
    		margin-block-start: -0.5em;
    		margin-block-end: -0.2em;
        	color: blue;
     		}
h3 	     	{
    		display: block;
    		font-size: 1.1em;
    		margin-block-start: 0em;
    		margin-block-end: 0.2em;
	     	}
[type="number"] {
  		font-size: 18px;
  		transition: border-color 1000ms;
 	 	width: 85px;
		float: left;
		}
[type="submit"] {
  		font-size: 16px;
  		transition: border-color 1000ms;
		border: 1px solid #ccc !important;
		width: 180px;
		float: right; 	     	
		}
form:invalid [type="submit"] {
  		background-color: var(--grey);
  		cursor: cursor: not-allowed;
	     	pointer-events: none;
		}
form:invalid [type="subm1it"]:active {
  		pointer-events: none;
	     	}
form 		{
    		display: flow-root;
        	}	 
.butt {display: inline-block;}
.main		{
		margin: auto;
		width: 300px;
    		padding: 5px;
		}
.form         	{
  	margin-bottom: 5px;	
    display: inline-block;
    padding: 2px 5px 5px 5px;
    border: 1px solid blue;
    border-radius: 4px;
    width: 270px;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
	     	}

.form__field 	{
		display: inline-block;
  		width: 85px;
		float: left;
		}

.form__error 	{
  		line-height: 15px;
		color: red;
  		text-align: left;
  		font-size: 13px;
  		display: block;
  		width: 300px;
		display: none;
	     	padding-top: 34px;
		}

#DistW {
    margin-top: 6px;
    width: 87px;
    text-align: center;
    float: left;
    font-size: xxx-large;
}

#up, #dw     	{
		font-weight: bold;
	     	}
.onb {color: red;}
.offb {color: green;}
.form input  	{
  		    outline: none;
    border-radius: 3px;
    border: 1px solid #ccc;
    box-sizing: border-box;
    height: 32px;
		}
input::placeholder {
  		color: black;
  		font-size: 12px;
		}
input:valid:not(:placeholder-shown) {
  		border-color: green;
	     	}

input:invalid:not(:placeholder-shown) {
  		border-color: red;
             	}
input:invalid:not(:placeholder-shown) + .form__error {
  		display: block;
		}

    </style>
  <script>
    function submitMessage(el) {
   if (el.id == "vup") {     
        document.getElementById('up').innerHTML = document.getElementById("inputUp").value;
	setTimeout(function(){document.getElementById("inputUp").value = null;}, 500);
	         	}
   if (el.id == "vdw") {     
     	document.getElementById('dw').innerHTML = document.getElementById("inputDw").value;
	setTimeout(function(){document.getElementById("inputDw").value = null;}, 500);
			}
    }
  </script>
  <script>
if (!!window.EventSource) {
            var source = new EventSource('/events');
            source.addEventListener('open', function(e) {console.log("Events Connected");}, false);
            source.addEventListener('error', function(e) {if (e.target.readyState != EventSource.OPEN) {console.log("Events Disconnected");}}, false);
  source.addEventListener('message', function(e) {console.log("message", e.data);}, false);
  
  source.addEventListener('Dist', function(e) {console.log("Distance", e.data);
  document.getElementById("DistW").innerHTML = e.data;}, false);
  
  source.addEventListener('Temp', function(e) {console.log("Temperature", e.data);
  document.getElementById("TempW").innerHTML = e.data;}, false);
  
  source.addEventListener('Pump', function(e) {console.log("Pump state", e.data);
  document.getElementById("PumpW").innerHTML = (e.data == '1') ? "<b class=onb>ON</b>" : "<b class=offb>OFF</b>";}, false);
  
  source.addEventListener('Heat', function(e) {console.log("Heat state", e.data);
  document.getElementById("HeatW").innerHTML = (e.data == '1') ? "<b class=onb>ON</b>" : "<b class=offb>OFF</b>";}, false);}
</script>
</head><body>
<div class = "main">
<H2>Tank unload</H2>
<H3>Form for tuning on/off pump</H3>
<div class="form">
<form action="/get" target="hidden-form">
Current On pump value Set <span id="up">%inputUp%</span> cm.</br>
<div class="form__field">
<input type="number" name="inputUp" id="inputUp" placeholder="Up level" min="5" max="150" onkeyup="value=value.replace(/^0+/,'')" required />
<span class="form__error">Up distance from sensor to water 5-150 cm</span></div>
<input type="submit" id="vup" value="Save value to SPIFSS" onclick="submitMessage(this)">
</form>
<form action="/get" target="hidden-form">
Current Off pump value Set <span id="dw">%inputDw%</span> cm.</br>
<div class="form__field"> 
<input type="number" name="inputDw" id="inputDw" placeholder="Down level" min="10" max="160" onkeyup="value=value.replace(/^0+/,'')" required>
<span class="form__error">Down distance from sensor to water 10-160 cm</span></div>
<input type="submit" id="vdw" value="Save value to SPIFSS" onclick="submitMessage(this)">
</form>
<span id="DistW" >%DistW%</span>
<form id="btpump" method="get" action="/pump" target="hidden-form" >
  <input type="submit" id="heat" value="Pump toggle">
</form>
<form id="btheat" method="get" action="/heat" target="hidden-form">
  <input type="submit" id="pump" value="Heat toggle" />
</form>
<iframe style="display:none" name="hidden-form"></iframe>
</div></br>
Pump <span id="PumpW" >%PumpW%</span>  Heat <span id="HeatW" >%HeatW%</span></br>
Temperature</br> <span id="TempW" >%TempW%</span></br>
<a href="/update">Updater</a>
</div>
</body></html>)rawliteral";

void pumpoff() {
Serial.println("pump off function ");
Serial.println(trig);
digitalWrite(LED_BUILTIN, HIGH); // off pump
pinMode(PUMP, INPUT);
trig = 0;
events.send(String(trig).c_str(),"Pump",millis());
}
void pumpon() {
  Serial.println("pump on function");
  Serial.println(trig);
  digitalWrite(LED_BUILTIN, LOW); // on pump
  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP, LOW);
  trig = 1;
  events.send(String(trig).c_str(),"Pump",millis());
}

void heatoff() {
Serial.println("heat off function");
Serial.println(btnheat);
//digitalWrite(LED_BUILTIN, HIGH); // off pump
pinMode(HEAT, INPUT);
events.send(String(btnheat).c_str(),"Heat",millis());
btnheat=0;
}
void heaton() {
  Serial.println("heat on function");
  Serial.println(btnheat);
//  digitalWrite(LED_BUILTIN, LOW); // on pump
  pinMode(HEAT, OUTPUT);
  digitalWrite(HEAT, LOW);
  events.send(String(btnheat).c_str(),"Heat",millis());
btnheat=1;
}

void disttemp() {
                   
                 /* Serial.println("Reading a measurement... echo ");
                  // Clears the trigPin
                  digitalWrite(trigPin, LOW);
                  delayMicroseconds(2);
                  // Sets the trigPin on HIGH state for 10 micro seconds
                  digitalWrite(trigPin, HIGH);
                  delayMicroseconds(10);
                  digitalWrite(trigPin, LOW);
                  duration = pulseIn(echoPin, HIGH);
                  cmDist = duration * SOUND_VELOCITY/2;*/
                                    
                  US100Serial.flush();
                  US100Serial.write(0x55); 
                  delay(500);
                  if(US100Serial.available() >= 2) 
                  {
                      MSByteDist = US100Serial.read(); 
                      LSByteDist = US100Serial.read();
                      mmDist  = MSByteDist * 256 + LSByteDist; 
                      cmDist = int(mmDist/10);


                      if (cmDist > 450) {cmDist = 444;}
                      if(cmDist > 1) 
                      {   Serial.println(" ");
                          Serial.print("Distance: ");
                          Serial.print(cmDist, DEC);
                          Serial.println(" cm   ");
                                   
                          if (trig == 0 && cmDist <= hi && low >= hi) 
                                {pumpon();}
                          if (trig == 1 && cmDist >= low)
                                {pumpoff();}
                          if (cmDist != TcmDist) { flag = 1; TcmDist=cmDist;}
                          
                      }
                  }
                  US100Serial.flush(); 
                    US100Serial.write(0x50); 
                    delay(500);
                    if(US100Serial.available() >= 1) 
                    {
                        temp3 = US100Serial.read();
                        if((temp3 > 1) && (temp3 < 130)) // temprature is in range
                          {  temp3 -= 45;} //
                    }



                     sensor.requestTemperatures(); // считывание значение температуры
                     temp1 = sensor.getTempC(Thermometer1); // температура в градусах Цельсия
                     temp2 = sensor.getTempC(Thermometer2);
                     Serial.println(temp1, DEC);
                     Serial.println(temp2, DEC);
                     Serial.println(temp3, DEC);
                     if (temp1 > max_temp1) max_temp1 = temp1; // обновление минимального значения температуры
                     if (temp1 < min_temp1) min_temp1 = temp1; // обновление максимального значения температуры
                     if (temp2 > max_temp2) max_temp2 = temp2; // обновление минимального значения температуры
                     if (temp2 < min_temp2) min_temp2 = temp2; // обновление максимального значения температуры
                     if (temp3 > max_temp3) max_temp3 = temp3; // обновление минимального значения температуры 401
                     if (temp3 < min_temp3) min_temp3 = temp3;
                     if (temp1 != Ttemp1) {flag=1; Ttemp1=temp1;}
                     if (temp2 != Ttemp2) {flag=1; Ttemp2=temp2;}
                     if (temp3 != Ttemp3) {flag=1; Ttemp3=temp3;}        
                     if (flag)
                          {
                          ptr = "";
                          display.clearDisplay();
                          display.display();
                          display.setCursor(62,0);
                          display.setTextSize(3);
                          if (cmDist<100) display.print(" ");
                          if (cmDist<10) display.print(" ");
                          display.print(cmDist);
                          display.setTextSize(2);
                          display.setCursor(0,0);
                          display.print(dtostrf(temp1, 4, 1, lcd_buffer));
                          display.setCursor(0,16);
                          display.print(dtostrf(temp2, 4, 1, lcd_buffer));
                          display.setCursor(0,33);
                          display.print(dtostrf(min_temp1, 4, 1, lcd_buffer));
                          display.print("::");
                          display.print(dtostrf(max_temp1, 4, 1, lcd_buffer));
                          display.setCursor(0,48);
                          display.print(dtostrf(min_temp2, 4, 1, lcd_buffer));
                          display.print("::");
                          display.print(dtostrf(max_temp2, 4, 1, lcd_buffer));
                          ptr+="\n";
                          ptr+=dtostrf(temp1, 4, 1, lcd_buffer);
                          ptr+="  Min : ";
                          ptr+=dtostrf(min_temp1, 4, 1, lcd_buffer);
                          ptr+="  Max : ";
                          ptr+=dtostrf(max_temp1, 4, 1, lcd_buffer);
                          ptr+="</br>";
                          ptr+=dtostrf(temp2, 4, 1, lcd_buffer);
                          ptr+="  Min : ";
                          ptr+=dtostrf(min_temp2, 4, 1, lcd_buffer);
                          ptr+="  Max : ";
                          ptr+=dtostrf(max_temp2, 4, 1, lcd_buffer);
                          ptr+="</br>";
                          ptr+=dtostrf(temp3, 4, 1, lcd_buffer);
                          ptr+="  Min : ";
                          ptr+=dtostrf(min_temp3, 4, 1, lcd_buffer);
                          ptr+="  Max : ";
                          ptr+=dtostrf(max_temp3, 4, 1, lcd_buffer);
                          ptr+="\n";
                          
                          display.display();
                          //flag=0;
                          }
}

void stvol() {
  String tmps;
  tmps=readFile(SPIFFS, "/inputUp.txt");
  hi=tmps.toInt();
  tmps=readFile(SPIFFS, "/inputDw.txt");
  low=tmps.toInt();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found, you have problem");
}

String readFile(fs::FS &fs, const char * path){
  Serial.printf("     Reading file: %s  ", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.print("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s    ", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.print("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.print("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// Replaces placeholder with stored values
String processor(const String& var){
  Serial.print("Processor start   ");
  if(var == "inputUp"){
    return readFile(SPIFFS, "/inputUp.txt");
  }
  if(var == "inputDw"){
    return readFile(SPIFFS, "/inputDw.txt");
  }
 /* if(var == "TempW"){
    return String(temp1);
  }
  if(var == "PumpW"){
    return String(trig);
  }
  else if(var == "DistW"){
    return String(cmDist);
  }*/

  return String();
}
//web


void setup() {
    Serial.begin(115200);
    US100Serial.begin(9600);
    pinMode(PUMP, INPUT); // pump off
    pinMode(HEAT, INPUT); // heat off
    
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input
    digitalWrite(LED_BUILTIN, HIGH);
    display.begin(i2c_Address, true);
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();
    display.setTextSize(3);
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Init display.");
    display.display();
    sensor.begin(); // инициализация DS18B20
    sensor.getAddress(Thermometer1, 0); // адрес DS18B20 (поиск по индексу)
    sensor.getAddress(Thermometer2, 1);
    sensor.setResolution(Thermometer1, TEMPERATURE_PRECISION);// установка точности измерения 9...12 разрядов
    sensor.setResolution(Thermometer2, TEMPERATURE_PRECISION);
//web
#ifdef ESP32
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif
  stvol();
 
  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  //wifiManager.setSTAStaticIPConfig(IPAddress(192,168,1,35), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
  
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    display.setCursor(0,10);
    display.print("Err connect.");
    display.display();
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  Serial.println("connected...yeey :)");
  Serial.println("local ip");
  display.setCursor(0,10);
  display.print("connected.");
  display.display();
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  display.setCursor(0,20);
  display.print(WiFi.localIP());
  display.display();
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.png", "image/png");
  });
  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  
  server.on("/pump", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (trig == 0) {
      pumpon();
      trig=1;
      Serial.println("PUMP ON");
    }
    else if (trig == 1) {
      pumpoff();
      trig=0;
      Serial.println("PUMP OFF");
    }
    Serial.println("InputMessage debugging pump");
    request->send_P(200, "text/plain", "test work get pump");
    });
  
  server.on("/heat", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (btnheat == 0) {
      heaton();
      btnheat=1;
      Serial.println("HEAT ON");
    }
    else if (btnheat == 1) {
      heatoff();
      btnheat=0;
      Serial.println("HEAT OFF");
    }
    Serial.println("InputMessage debugging heat");
    request->send_P(200, "text/plain", "test work get heat");
    });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputUp=<inputMessage>
    if (request->hasParam(PARAM_UP)) {
      inputMessage = request->getParam(PARAM_UP)->value();
      hi=inputMessage.toInt();
      Serial.println(inputMessage);
      writeFile(SPIFFS, "/inputUp.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputDw=<inputMessage>
    else if (request->hasParam(PARAM_DW)) {
      inputMessage = request->getParam(PARAM_DW)->value();
      low=inputMessage.toInt();
      Serial.println(inputMessage);
      writeFile(SPIFFS, "/inputDw.txt", inputMessage.c_str());
    }
      else {
      inputMessage = "No message sent";
    }
    Serial.print(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  AsyncElegantOTA.begin(&server);
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events);
  server.begin();
  timeClient.begin();
  pumpoff();
  heatoff();
//web
}
 
void loop() {
 if ((millis() - lastTime) > timerDelay) {
    disttemp();
    Serial.println(hi, DEC);
    Serial.println(low, DEC);
    Serial.println(temp1, DEC);
    Serial.println(temp2, DEC);
    Serial.println(temp3, DEC);
    Serial.println(cmDist, DEC);
    Serial.println(trig, DEC);
    Serial.println(btnheat, DEC);
    events.send("ping",NULL,millis());
    events.send(String(cmDist).c_str(),"Dist",millis());
    events.send(String(trig).c_str(),"Pump",millis());
    events.send(String(btnheat).c_str(),"Heat",millis());
    events.send(ptr.c_str(),"Temp",millis());
    //events.send(String(temp1).c_str(),"Temp",millis());
    ptr ="";
    lastTime = millis();
    
    timeClient.update();
  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  if (timeClient.getHours() == 23 && timeClient.getMinutes() == 59)
   {min_temp1 = 50;
    min_temp2 = 50;
    min_temp3 = 50;
    max_temp1 = -50;
    max_temp2 = -50;
    max_temp3 = -50;
    }
  }
   
}