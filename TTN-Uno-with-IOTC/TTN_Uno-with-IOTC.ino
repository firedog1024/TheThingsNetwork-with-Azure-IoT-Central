#include <TheThingsNetwork.h>

// First install "DHT sensor library" via the Library Manager
#include <DHT.h>

// Set your AppEUI and AppKey from the application afer registering the device
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";

#define loraSerial Serial1
#define debugSerial Serial

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan REPLACE_ME

// these are the events that cause the device to send data
#define PORT_SETUP 1    // initial startup of the device
#define PORT_INTERVAL 2 // device wakes from a sleep interval
#define PORT_MOTION 3   // device wakes from motion change (stop, start, shake)
#define PORT_BUTTON 4   // device wakes from button press

#define ALERT_MIN_TEMP      20   // min temperature in degC for alert
#define ALERT_MAX_TEMP      24   // max temperature in degC for alert
#define ALERT_CRITICAL_TEMP 30   // temperature in degC when critical

// interval timer values in milliseconds
#define SEND_INTERVAL_MS 60000
#define MIN_SHAKE_INTERVAL_MS 20000

#define DHTPIN 2
#define BUTTON_PIN 3

//Choose your DHT sensor moddel
//#define DHTTYPE DHT11
//#define DHTTYPE DHT21
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);

// values for random generated sensor values
long lastSendInterval = millis();
long lastShakeInterval = millis();
uint16_t battery = 3000;
uint16_t light = 20;
uint16_t accelX = 200;
uint16_t accelY = 100;
uint16_t accelZ = 90;

// called at device startup
void setup()
{
    // kick off the serial ports
    loraSerial.begin(57600);
    debugSerial.begin(9600);

    // Wait a maximum of 10s for Serial Monitor
    while (!debugSerial && millis() < 10000)
    ;

    debugSerial.println("-- STATUS");
    ttn.showStatus();

    debugSerial.println("-- JOIN");
    ttn.join(appEui, appKey);

    // init the DHT sensor
    dht.begin();

    // setup button input
    pinMode(BUTTON_PIN, INPUT);

    // init random seed
    randomSeed(analogRead(0));

    // send initial data payload after connecting
    sendData(PORT_SETUP);
}

void loop()
{
    // interval send
    if (millis() - lastSendInterval > SEND_INTERVAL_MS) {
        sendData(PORT_INTERVAL);
        lastSendInterval = millis();
    }

    // button press send
    if (digitalRead(BUTTON_PIN) == HIGH) {
        sendData(PORT_BUTTON);
        lastSendInterval = millis(); // reset interval send
        lastShakeInterval = millis(); // reset shake send interval
    }

    // motion sense
    if (random(0, 1000) > 750 && millis() - lastShakeInterval > MIN_SHAKE_INTERVAL_MS) {
        sendData(PORT_MOTION);
        lastShakeInterval = millis();
        lastSendInterval = millis(); // reset interval send
    }
}

// generate the next random walk value
uint16_t randomWalkValue(uint16_t lastValue, uint8_t deltaMax, uint8_t minValue, uint8_t maxValue) {
    bool up = random(-1, 1);
    uint8_t delta = random(0, deltaMax + 1);

    if (up && (lastValue + delta < maxValue))
        return lastValue + delta;
    else if (lastValue - delta > maxValue)
        return lastValue - delta; 
    else
        return lastValue;
}

void sendData(uint8_t port) {
    // Read sensor values and multiply by 100 to effictively have 2 decimals
    uint16_t humidity = dht.readHumidity(false) * 100;

    // false: Celsius (default)
    // true: Farenheit
    uint16_t temperature = dht.readTemperature(false) * 100;

    // Split both words (16 bits) into 2 bytes of 8
    byte *bytes;
    byte payload[13];

    //battery - random walk
    battery = randomWalkValue(battery, 10, 0, 4790);
    bytes = (byte *)&battery;
    payload[0] = bytes[1];
    payload[1] = bytes[0];

    // light - random walk
    light = randomWalkValue(light, 20, 0, 128);
    bytes = (byte *)&light;
    payload[2] = bytes[1];
    payload[3] = bytes[0];

    // temperature - actual sensor value
    payload[4] = highByte(temperature);
    payload[5] = lowByte(temperature);

    // accelerometer -random walk
    // X
    accelX = randomWalkValue(accelX, 50, -500, 500);
    bytes = (byte *)&accelX;
    payload[6] = bytes[1];
    payload[7] = bytes[0];
    // Y
    accelY = randomWalkValue(accelY, 50, -500, 500);
    bytes = (byte *)&accelY;
    payload[8] = bytes[1];
    payload[9] = bytes[0];
    // Z
    accelZ = randomWalkValue(accelZ, 50, -500, 500);
    bytes = (byte *)&accelZ;
    payload[10] = bytes[1];
    payload[11] = bytes[0];

    // set bit 1 if moving
    bool isMoving = random(-1,1);
    if (isMoving) {
        payload[12] = 0x01;
    } else {
        payload[12] = 0x00;
    }

    // set bit 2 if temperature alert and bit 3 if critical
    if (temperature < ALERT_MIN_TEMP * 100 || temperature > ALERT_MAX_TEMP * 100) {
        if (temperature < ALERT_CRITICAL_TEMP * 100) {
            payload[12] = payload[12] + 0x02; // set bit 2
        } else {
            payload[12] = payload[12] + 0x04; // set bit 3
        }
    } else {
        payload[12] = payload[12] & 0xF9;
    }

    debugSerial.print("Temperature: ");
    debugSerial.println(temperature);
    debugSerial.print("Humidity: ");
    debugSerial.println(humidity);

    // send the data to The Things Network
    ttn.sendBytes(payload, sizeof(payload), port);
}