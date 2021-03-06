#define MY_BAUD_RATE 9600
#define MY_TRANSPORT_WAIT_READY_MS 3000
//#define MY_DEBUG
#define MY_RADIO_RF24
#define MY_SIGNING_SOFT
#define PARENT_NODE_ID 0
#define MY_SIGNING_NODE_WHITELISTING {{.nodeId = PARENT_NODE_ID,.serial = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}}}
#define MY_SIGNING_REQUEST_SIGNATURES
#define MY_SIGNING_SOFT_RANDOMSEED_PIN 4
#include <MySensors.h>

//DHT Sensors setup
#include <DHT.h>
#define TortSensPin A1
#define RoomSensPin A2
bool HeatEnabled = false;
#define SensType DHT22
#define RoomTempId 0
#define TortTempId 1
#define TortHumdId 2
DHT TortSens(TortSensPin, SensType);
DHT RoomSens(RoomSensPin, SensType);

//IR Sensor setup
#include <IRremote.h>
#define IR_SEND_PIN A0

//Misc defines
#define TortRelayPin 8
#define TestHeatPin A3

// Initialize messages
MyMessage TortTempMsg(TortTempId, V_TEMP);
MyMessage TortHumdMsg(TortHumdId, V_HUM);
MyMessage RoomTempMsg(RoomTempId, V_TEMP);

//Temp defaults
int TortTargetTemp = 85;
int TortTargetHumd = 46;
int RoomTargetTemp = 74;
int RiseAboveTarget = 2;
int FallBelowTarget = 2;

void setup()
{
  Serial.begin(9600);
  TortSens.begin();
  RoomSens.begin();
  pinMode(TortRelayPin, OUTPUT);
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);
  wait(500);
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Logan's Room", "1.0");

	// Register all sensors to gw (they will be created as child devices)
	present(TortTempId, S_TEMP);
  present(TortHumdId, S_HUM);
  present(RoomTempId, S_TEMP);
}

void loop()
{
  wait(5000);
	// Read values
  int16_t TortTemp = TortSens.readTemperature(true);
  wait(500);
	int16_t TortHumd = TortSens.readHumidity();
  wait(500);
  int16_t RoomTemp = RoomSens.readTemperature(true);
  wait(500);

  send(TortTempMsg.set(TortTemp));
  wait(500);
  send(TortHumdMsg.set(TortHumd));
  wait(500);
  send(RoomTempMsg.set(RoomTemp));
  wait(500);
  Serial.print("Room temp: ");
  Serial.print(RoomTemp);
  Serial.print(" Heat ");
  if(HeatEnabled){
    Serial.println("On.");
  }else{
    Serial.println("Off.");
  }
  Serial.print("Oreo's Humidity: ");
  Serial.println(TortHumd);
  Serial.print("Oreo's Temp: ");
  Serial.println(TortTemp);

  if(RoomTemp <= (RoomTargetTemp - FallBelowTarget) && !HeatEnabled){
    PowerUpHeat();
  }
  if(RoomTemp >= (RoomTargetTemp + RiseAboveTarget) && HeatEnabled){
    ShutDownHeat();
  }
  if(TortHumd <= (TortTargetHumd - FallBelowTarget))
  {
    Serial.println("Relay HIGH!");
    digitalWrite(TortRelayPin, HIGH);
  }
  if(TortHumd >= (TortTargetHumd + RiseAboveTarget))
  {
    Serial.println("Relay LOW!");
    digitalWrite(TortRelayPin, LOW);
  }
  if(analogRead(TestHeatPin) > 1000){
    Serial.println("Testing Heat IR Signals...");
    //TestHeat();
  }
}

void PowerUpHeat(){
  HeatEnabled = true;
  Serial.println("Turning on heat.");
  IrSender.sendNEC(0xFF00, 0xC4, 0); // Sony TV power code
  wait(2000);
  IrSender.sendNEC(0xFF00, 0xC8, 0); // Sony TV power code
  wait(2000);
}

void ShutDownHeat() {
  HeatEnabled = false;
  Serial.println("Turning off heat.");
  wait(2000);
  IrSender.sendNEC(0xFF00, 0xC4, 0); // Sony TV power code
  wait(2000);
}

void TestHeat(){
  PowerUpHeat();
  wait(20000);
  ShutDownHeat();
  wait(2000);
}

void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from node");
  }

  if (message.type == V_LOCK_STATUS) {
    if(!HeatEnabled){
      PowerUpHeat();
    }else{
      ShutDownHeat();
    }
    wait(1000);
  }
}
