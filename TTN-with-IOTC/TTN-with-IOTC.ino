#include <TheThingsNode.h>

// Set your AppEUI and AppKey from the application afer registering the device
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";

#define loraSerial Serial1
#define debugSerial Serial

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_US915

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);
TheThingsNode *node;

// these are the events that cause the device to send data
#define PORT_SETUP 1    // initial startup of the device
#define PORT_INTERVAL 2 // device wakes from a sleep interval
#define PORT_MOTION 3   // device wakes from motion change (stop, start, shake)
#define PORT_BUTTON 4   // device wakes from button press

#define ALERT_MIN_TEMP      20   // min temperature in degC for alert
#define ALERT_MAX_TEMP      24   // max temperature in degC for alert
#define ALERT_CRITICAL_TEMP 30   // temperature in degC when critical

/*
Decoder payload function
------------------------

function isBitSet(byte, bitOffset) {
  if (byte & (0x01 << (bitOffset - 1)))
    return true;
  else
    return false;
}

function processFloat(bytes, offset) {
  if (bytes[offset] & 0x80)
    retVal = ((0xffff << 16) + (bytes[offset] << 8) + bytes[offset+1]) / 100;
  else
    retVal = ((bytes[offset] << 8) + bytes[offset+1]) / 100;
  return retVal;
}

function Decoder(bytes, port) {
  var decoded = {};
  var events = {
    1: 'setup',
    2: 'interval',
    3: 'motion',
    4: 'button'
  };
  decoded.event = events[port];
  decoded.battery = (bytes[0] << 8) + bytes[1];
  decoded.light = (bytes[2] << 8) + bytes[3];
  decoded.temperature = processFloat(bytes, 4);
  decoded.accelerationX = processFloat(bytes, 6);
  decoded.accelerationY = processFloat(bytes, 8);
  decoded.accelerationZ = processFloat(bytes, 10);
  
  //if (bytes[12] & 0x01)
  //  decoded.isMoving = true;
  //else
  //  decoded.isMoving = false;
  
  decoded.isMoving = isBitSet(bytes[12], 1)?'moving':'stopped';
  
  //  if (bytes[12] & 0x02)
  //  decoded.tempAlert = true;
  //else
  //  decoded.tempAlert = false;
  
  if (isBitSet(bytes[12], 3)) {
    decoded.tempAlert = 'critical';
  } else {
    decoded.tempAlert = isBitSet(bytes[12], 2)?'alert':'normal';
  }
  
  return decoded;
}
*/

// called at device startup
void setup()
{
  // kick off the serial ports
  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

  // Initialize and configure the Things Node
  // the code uses all the sensors on the device:
  //   + Temperature
  //   + Accelerometer
  //   + Light
  //   + plus sleep interval and button press
  //
  node = TheThingsNode::setup();
  node->configLight(true);
  node->configInterval(true, 60000);
  // sets alert at < 20c > 24c, critical at 30c
  node->configTemperature(true, R_DEGREES_0_0625, ALERT_MIN_TEMP, ALERT_MAX_TEMP, ALERT_CRITICAL_TEMP, H_DEGREES_0_0);

  // setup callbacks to handle events
  node->onWake(wake);
  node->onInterval(interval);
  node->onSleep(sleep);
  node->onMotionStart(onMotionStart);
  node->onMotionStop(onMotionStop);
  node->onButtonRelease(onButtonRelease);
  node->onTemperature(onTemperatureAlert);
  
  // Test sensors and set LED to GREEN if it works
  node->showStatus();
  node->setColor(TTN_GREEN);

  // display the status of The Things Network connection
  debugSerial.println("-- TTN: STATUS");
  ttn.showStatus();

  // connect to The Things Network
  debugSerial.println("-- TTN: JOIN");
  ttn.join(appEui, appKey);

  // send initial data payload after connecting
  debugSerial.println("-- SEND: SETUP");
  sendData(PORT_SETUP);
}

// main loop - do not do anything in here that block for any significant amount of time
void loop()
{
  node->loop();
}

// called when the device interval fires
void interval()
{
  node->setColor(TTN_BLUE);

  debugSerial.println("-- SEND: INTERVAL");
  sendData(PORT_INTERVAL);
}

// called when the device wkaes up from sleep
void wake()
{
  node->setColor(TTN_GREEN);
}

// called when a device goes into sleep mode
void sleep()
{
  node->setColor(TTN_BLACK);
}

// called when the device detects motion
void onMotionStart()
{
  node->setColor(TTN_BLUE);

  debugSerial.println("-- SEND: MOTION");
  sendData(PORT_MOTION);
}

// called when device detects it has come to rest
void onMotionStop(unsigned long duration)
{
  node->setColor(TTN_BLACK);
  debugSerial.print("-- INFO: MOTION STOPPED (duration: ");
  debugSerial.print(duration);
  debugSerial.println("ms)");
}

// handles button press
void onButtonRelease(unsigned long duration)
{
  node->setColor(TTN_BLUE);

  debugSerial.print("-- SEND: BUTTON (duration:  ");
  debugSerial.print(duration);
  debugSerial.println("ms)");

  sendData(PORT_BUTTON);
}

// handles temperature alerts
void onTemperatureAlert() {
  node->setColor(TTN_YELLOW);
  debugSerial.println("-- ALERT: TEMPERATURE ALERT");
}

// Pulls data from the onboard sensors and sends to The Things Network
void sendData(uint8_t port)
{
  // Wake RN2483
  ttn.wake();

  ttn.showStatus();
  node->showStatus();
  
  // create the payload to send
  byte *bytes;
  byte payload[13];

  // get battery value in mV
  uint16_t battery = node->getBattery();
  bytes = (byte *)&battery;
  payload[0] = bytes[1];
  payload[1] = bytes[0];

  // get light value
  uint16_t light = node->getLight();
  bytes = (byte *)&light;
  payload[2] = bytes[1];
  payload[3] = bytes[0];

  // get temperature value in degC and multiply by 100 to get an integer
  int16_t temperature = round(node->getTemperatureAsFloat() * 100);
  bytes = (byte *)&temperature;
  payload[4] = bytes[1];
  payload[5] = bytes[0];

  // get acceleration values and multiply by 100 to get an integer
  float x;
  float y;
  float z;
  node->getAcceleration(&x, &y, &z);
  Serial.print("Acceleration (x,y,z): (");
  Serial.print(x); Serial.print(",");
  Serial.print(y); Serial.print(",");
  Serial.print(z); Serial.println(")");

  int16_t myInt = x * 100;
  bytes = (byte *)&myInt;
  payload[6] = bytes[1];
  payload[7] = bytes[0];
  myInt = y * 100;
  bytes = (byte *)&myInt;
  payload[8] = bytes[1];
  payload[9] = bytes[0];
  myInt = z * 100;
  bytes = (byte *)&myInt;
  payload[10] = bytes[1];
  payload[11] = bytes[0];
      
  // set bit 1 if moving
  if (node->isMoving()) {
    payload[12] = 0x01;
  } else {
    payload[12] = 0x00;
  }

  // set bit 2 if temperature alert and bit 3 if critical
  if (node->hasTemperatureAlert()) {
    if (temperature < ALERT_CRITICAL_TEMP * 100) {
      payload[12] = payload[12] + 0x02; // set bit 2
    } else {
      payload[12] = payload[12] + 0x04; // set bit 3
    }
  } else {
    payload[12] = payload[12] & 0xF9;
  }

  
  // send the data to The Things Network
  ttn.sendBytes(payload, sizeof(payload), port);

  // Set RN2483 to sleep mode
  ttn.sleep(60000);

  // This one is not optional, remove it
  // and say bye bye to RN2983 sleep mode
  delay(50);
}
