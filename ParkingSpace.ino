#include <Ethernet.h>
#include <SPI.h>
#include <Servo.h>

const int redLED = 5;
const int yellowLED = 6;
const int greenLED = 7;
const int PhotocellLDR = 0;
const int FSR = 1;
const int buzzer = 3;
const int distSharp = 4;

int clk = 0;
String message;
String parkClear = "-4";
String alarmMessage = "-2";
String cancelBooking = "-3";
String parkBusy = "-5";

bool reserva = false;
bool carDetected = true;

// MAC-ADDRESS 4d:61:7e:ff:12:28
byte mac[] = {0x4d,0x61,0x7e,0xff,0x12,0x28};
byte ip[] = {192,168,10,103};

// Pi Server
byte serverIP[] = {192,168,10,104};
Servo servo;

EthernetServer server = EthernetServer(5560);
EthernetClient client;

void setup() {
  Serial.begin(9600);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(FSR, OUTPUT);
  pinMode(PhotocellLDR, INPUT);
  pinMode(distSharp, INPUT);
  pinMode(buzzer, OUTPUT);

  servo.attach(2);
  upBar();
  delay(500);
  Ethernet.begin(mac,ip);
  delay(500);
  if(client.connect(serverIP, 5560)) {
    client.println(parkClear);
  }
  delay(500);
}

void loop() {
  if(!reserva) {
    client = server.available();
    message = client.readString();
    clk = message.toInt();
    if(clk > 0) {
      // Notify server for reservation
      reserva = true;
    }
    //FSR && LDR && Dist Sensors
    int FSRval = analogRead(FSR);
    int LDRval = analogRead(PhotocellLDR);
    int Distval = analogRead(distSharp);
    if(FSRval >= 400 && Distval >= 370){
      digitalWrite(greenLED, LOW);
      delay(450);
      digitalWrite(redLED, HIGH);
      if(carDetected) {
        if(client.connect(serverIP, 5560)) {
          Serial.println("BUSY");
          client.println(parkBusy);
          client.flush();
          client.stop();
        }
        carDetected = false;
      }
    }
    else {
      digitalWrite(redLED, LOW);
      delay(450);
      digitalWrite(greenLED, HIGH);
      // Park is clear -> alert server
      if(!carDetected) {
        if(client.connect(serverIP, 5560)) {
          Serial.println("FREE");
          client.println(parkClear);
          client.flush();
          client.stop();
        }
        carDetected = true;
      }
    }
    if(LDRval >= 890) {
      // Annouce alarm to server
      if(client.connect(serverIP, 5560)) {
        client.println(alarmMessage);
      }
      alarm();
    }
  }
  else {
    downBar();
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    client = server.available();
    message = client.readString();
    if(message == "0" || clk == 0) {
      upBar();
      digitalWrite(redLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(greenLED, HIGH);
      if(client.connect(serverIP, 5560)) {
        client.println(cancelBooking);
      }
      reserva = false;
    }
    Serial.println(clk);
    clk--;
  }
}

void alarm() {
  while(true) {
    for(int i=500 ; i<1500 ; i++) {
        tone(buzzer, i, 200);
    }
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, HIGH);
    delay(50);
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(greenLED, LOW);
    delay(50);
    digitalWrite(redLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, LOW);
    client = server.available();
    message = client.readString();
    if(message == "-1") {
      break;
    }
  }
}

void upBar() {
  servo.write(90);
}

void downBar() {
  servo.write(8);
}
