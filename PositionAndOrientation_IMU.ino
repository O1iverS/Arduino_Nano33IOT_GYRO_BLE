#include <Arduino_LSM6DS3.h>
#include <MadgwickAHRS.h>
#include <ArduinoBLE.h>
#include <math.h>

// create service and create characteristics and allow remote device to read and get notifications:
BLEService accelService("00002a37-e83c-11e8-9f32-f2801f1b9fd1");
BLECharacteristic accelX("0000180d-e83c-11e8-9f32-f2801f1b9fd1", BLERead | BLENotify, 8);
BLECharacteristic accelY("0000180a-e83c-11e8-9f32-f2801f1b9fd1", BLERead | BLENotify, 8);
BLECharacteristic accelZ("00002a29-e83c-11e8-9f32-f2801f1b9fd1", BLERead | BLENotify, 8);

// initialize a Madgwick filter:
Madgwick filter;
// sensor's sample rate is fixed at 104 Hz:
const float sensorRate = 104.00;
int i = 0;
char rollchar [8];
char pitchchar [8];
char headingchar [8];

String rollstring = "0.0";
String pitchstring = "0.0";
String headingstring = "0.0";

void setup() {
  Serial.begin(9600);

  while (!BLE.begin()) {
    Serial.println("Waiting for BLE to start");
    delay(1);
  }
  // set the local name that the peripheral advertises:
  BLE.setLocalName("NanoIOT-Gyroscop");
  // set the UUID for the service:
  BLE.setAdvertisedService(accelService);
  // add the characteristics to the service
  accelService.addCharacteristic(accelX);
  accelService.addCharacteristic(accelY);
  accelService.addCharacteristic(accelZ);
  // add the service
  BLE.addService(accelService);
  // start advertising the service:
  BLE.advertise();

  pinMode(LED_BUILTIN, OUTPUT); // For level (Wasserwaage)
  // attempt to start the IMU:
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU");
    // stop here if you can't access the IMU:
    while (true);
  }
  // start the filter to run at the sample rate:
  filter.begin(sensorRate);
}

void loop() {
  // values for acceleration & rotation:
  float xAcc, yAcc, zAcc;
  float xGyro, yGyro, zGyro;

  // values for orientation:
  float roll, pitch, heading;




  // check if the IMU is ready to read:

  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    // read accelerometer & gyrometer:
    IMU.readAcceleration(xAcc, yAcc, zAcc);
    IMU.readGyroscope(xGyro, yGyro, zGyro);

    // update the filter, which computes orientation:
    filter.updateIMU(xGyro, yGyro, zGyro, xAcc, yAcc, zAcc); 

  }

  //Turn inbuild LED on when Roll and Pitch are nearly 0
  roll = -(round (filter.getRoll() * 10.0) / 10.0); //-: to correct movement: in clock direction 
  pitch = (round (filter.getPitch() * 10.0)) / 10.0;
  heading = 360 -(round (filter.getYaw() * 10.0)) / 10.0; //360 -:to correct movement: in clock direction 



  // i++; //Counter for BLE update to client e.g. every 11th time i>10
  //if (central.connected() && i > 8) {

  rollstring = String(roll);
  rollstring.toCharArray (rollchar, 8) ;

  pitchstring = String(pitch);
  pitchstring.toCharArray (pitchchar, 8) ;

  headingstring = String(heading);
  headingstring.toCharArray (headingchar, 8) ;
  
  BLEDevice central = BLE.central();

  i++;
  if (central.connected() && i > 15) {
    accelX.writeValue((unsigned char *)rollchar, 8);
    accelY.writeValue((unsigned char *)pitchchar, 8);
    accelZ.writeValue((unsigned char *)headingchar, 8);
    i = 0;
  }


  if (abs(roll) < 1 and abs(pitch) < 1) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  // print the heading, pitch and roll

  Serial.print("Orientation: ");
  Serial.print(rollchar);
  Serial.print(" ");
  Serial.print(pitchchar);
  Serial.print(" ");
  Serial.println(headingchar);




}
