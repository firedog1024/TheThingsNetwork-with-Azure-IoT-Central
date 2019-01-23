# The Things Network with Azure IoT Central

## Whats this all about

Learn how to get a The Things Network device connected to Azure IoT Central via the Azure IoT Central Device Bridge.  Get the best of both world by connecting LoRaWAN devices on The Things Network to the cloud based IoT solution provided by Azure IoT Central.  Bridging the gap between LoRaWan and the cloud with an Open Source solution called Azure IoT Central Device Bridge.

### Helpful introduction links:

* The Things Network - https://www.thethingsnetwork.org/
* Azure IoT Central - https://azure.microsoft.com/en-us/services/iot-central/
* Azure IoT Central Device Bridge - https://docs.microsoft.com/en-us/azure/iot-central/howto-build-iotc-device-bridge

**Note:** TTN == The Things Network.

## Hardware

We will be utilizing The Things Network Node device for this tutorial.  Details on the device and getting started can be found here: https://www.thethingsnetwork.org/docs/devices/node/.  You can use other LoRaWAN devices as well including The Things Network Uno and Pycom LoPy.  Over time we will try and add sample code for those devices as well.  Remember when purchasing devices for use to take note of the radio frequency of the device and ensure it will work in your region of the world.

If you are not in range of a public The Things Netork gateway you will also need to purchase or build a Things Network compatible gateway.  Details on how to get started with a compatible gateway can be found here https://www.thethingsnetwork.org/docs/gateways/.  We've setup both a RAK831 on a Raspberry Pi 3 and used The Things Newtwork Gateway.  Both worked great but the TTN device is much more plug and play and hence my prefered goto gateway.


## Setting up your Arduino development environment

### For the The Things Node device

![The Things Node](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/theThingsNode.png)

This is the cliff notes version of getting up and running, if you desire more information please see https://www.thethingsnetwork.org/docs/devices/node/quick-start.html and follow the instructions.

Add the following URL to the Arduino preferences - Additional Boards Manager URL's.

```
https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
```

In Tools -> Boards -> Boards Manager filter by "sparkfun avr boards" and install the latest version. Now in Tools -> Boards select the SparkFun Pro Micro.  **Important.** select Tools -> Processor -> ATmega32U4 (3.3V, 8Mhz) before moving forward.  Make sure the device is connected to your computer via a micro-usb cable (note the micro-usb connector is inside the case so undo the two screws and remove the bottom of the device to gain access, also note that the usb cable needs to be not too chunky to fit).  Once connected set the Tools -> Port to the right COM port on Windows or /dev/*usb* on Mac/Linux.

Now we need to grab the library for the Things Node device.  Sketch -> Include Library -> Manage Libraries and filter with "thethingsnetwork" and install the latest version.  After that, Sketch -> Include Library -> Manage Libraries and filter with "thethingsnode" and install the latest version.  Ok we have all the stuff we need to start writing device code.

## Getting setup on The Things Network

### For the The Things Node device

Now that we have our development environment setup we need to setup an account on The Things Network (if you don't have one already) and then create an application and register our device.  Follow the instructions here https://www.thethingsnetwork.org/docs/devices/node/quick-start.html#get-your-device-eui thru **Register your Device**.  After doing this your device will be registered on The Things Network and you should have both an App EUI and a App Key.  If you look at the bootom of the device information page you should see EXAMPLE CODE you can copy the code in the text box for use in the sample code.

![Example code](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/examplecode.png)

**Note:** When choosing a name for your device please only use lowercase letters, numbers, and hyphens.  This is a current limitation of the Device Provisioning Service device naming schema.

## Sending data from the device

### For the The Things Node device

Load the TTN-with-IOTC.ino file into the Arduino IDE and change the values of the appEui and appKey to those copied from the EXAMPLE CODE in the code:

```
// Set your AppEUI and AppKey from the application afer registering the device
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";
```

Connect the device to the computer with the USB cable and click the upload button on the Arduino IDE the code should compile and be uploaded to the device.  The IDE should show "Done uploading." and display output similar to this:

```
Sketch uses 25198 bytes (87%) of program storage space. Maximum is 28672 bytes.
Global variables use 1612 bytes (62%) of dynamic memory, leaving 948 bytes for local variables. Maximum is 2560 bytes.
```

The code is now executing on the device and should be sending data to The Things Network.  We can see the data being sent by going over to our TTN application that we created earlier and clicking on the "Data" tab.  You should see data coming into the application looking something like this:

![Raw data flow](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/data-raw.png)

note that the payload is not being decoded into human readable values and is just shown as it's byte array representation.  If we click on one of the lines we can see that there is nothing in the Fields section as the again the data is not being decoded.

![Raw data detail](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/data-raw-detail.png)

We need to add a payload decoder to our application.  In our application console click the "Payload Formats" tab.  Then cut and paste in the following code into the custom decoder and click the "save payload function" button.  

```
// check if a bit flag in a byte is set
function isBitSet(byte, bitOffset) {
  if (byte & (0x01 << (bitOffset - 1)))
    return true;
  else
    return false;
}

// convert a byte array to a float value
function processFloat(bytes, offset) {
  if (bytes[offset] & 0x80)
    retVal = ((0xffff << 16) + (bytes[offset] << 8) + bytes[offset+1]) / 100;
  else
    retVal = ((bytes[offset] << 8) + bytes[offset+1]) / 100;
  return retVal;
}

// main decode function
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
  
  decoded.isMoving = isBitSet(bytes[12], 1)?'moving':'stopped';
  
  decoded.tempAlert = isBitSet(bytes[12], 2)?'alert':'normal';
  if (isBitSet(bytes[12], 3)) {
    decoded.tempAlert = 'critical';
  }
  
  return decoded;
}
```

We can test the decoder by pasting in one of the raw payloads from the "Data" tab and clicking test.  You should see something like this:

![Payload formating](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/payload-format.png)

Returning back to the "Data" tab you should also that the data payloads are now being correctly decoded using the decoder function, and expanding one of them will show the "Fields" being correctly displayed.

![Payload detail fields formatted](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/payload-detail-fields.png)

The code on the device will send a payload based on one of four events:

* When first powered on and connecting to The Things Network
* Every 60 seconds of inactivity
* When the button is pressed
* When the device is shaken and the accelerometer is triggered

Try each of these and look at the payloads sent to make sure that the event is correctly represented in the "event" field.

## Getting data into Azure IoT Central

Now that we have data flowing through The Things Network we need to get it over to an Azure IoT Central application.  First lets setup an Azure IoT central application.

### Creating an Azure IoT Central application

To connect the device to Azure IoT Central you will need to provision an IoT Central application. This is free for seven days but if you already have signed up for an Azure subscription and want to use pay as you go IoT Central is free as long as you have no more than five devices and do not exceed 1MB per month of data.

Go to https://apps.azureiotcentral.com/ to create an application (you will need to sign in with a Microsoft account identity you may already have one if you use Xbox, office365, Windows 10, or other Microsoft services).

* Choose Trial or Pay-As-You-Go.
* Select the Custom Application (right box)
* Provide an application name and URL domain name
* If you select Pay-As-You-Go you will need to select your Azure subscription and select a region to install the application into. This information is not needed for Trial.
* Click "Create"

You should now have an IoT Central application provisioned but it's completely empty.  We need to add a device template for our Things Node device.  On the Homepage click the "Create Device Template", provide a suitable name and click "Create".

This will create a blank device template and a simulated device that will send simulated data that you add to the device template.  We need to add telemetry fields, State fields, and Event fields.  To add each of these click the "Edit Template" button in the top right, then click the "+ New Measurement" button.  You can now add a new telemetry, State, or Event measure.  Add the following to the template.

#### Telemetry:

| Display name | Field name | Units | Min value | Max value | Decimal places |
|---|---|---|---|---|---|
| Acceleration-X | accelerationX | g | -5 | 5 | 2 |
| Acceleration-Y | accelerationY | g | -5 | 5 | 2 |
| Acceleration-Z | accelerationZ | g | -5 | 5 | 2 |
| Battery | battery | mV | 0 | 5000 | 0 |
| Light | light | lux | 0 | 128 | 0 |
| Temperature | temperature | degC | -10 | 75 | 2 |

#### Event:


| Display name | Field name | Default Severity |
|---|---|---|
| Trigger Action | event | Information |
| Gateway Name | gateway_id | Information |

#### State:

First State:
| Display name | Display Name |
|---|---|
| Moving/Stopped | isMoving |

Values:

| Value | Field name |
|---|---|
| moving | Moving |
| stopped | Stopped |

Second State:
| Display name | Display Name |
|---|---|
| Temperature | tempAlert |

Values:

| Value | Field name |
|---|---|
| alert | Alert |
| critical | Critical |
| normal | Normal |

For more information on setting up a device template you can read the full documentation here https://docs.microsoft.com/en-us/azure/iot-central/howto-set-up-template. We should now have a device template and a simulated device that is sending conforming data to the template and we can see that data plotted on the screen.  Finalize the template by clicking on the "Done" button at the top of the graph.

![IoT Central template editing](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/template.png)

### Connecting The Things Network and Azure IoT Central applications

We now have a Things Network application and a IoT central application up and running but we need to connect the two together.  This is where the Azure IoT Central Device Bridge comes in.  The bridge connects another system to IoT Central via a simple webhook model (https://en.wikipedia.org/wiki/Webhook).  The Azure IoT Central Device Bridge can be found on github here https://github.com/Azure/iotc-device-bridge.  Follow the instructions to deploy the bridge, return back here after you have deployed the bridge and we will configure the Azure function.

### Configure the IoT Central Device Bridge function

We need to change the Azure function for the IoT Central Device Bridge, open up the Azure function from the Azure portal and add the following code snippet before the await handleMessage(... line:

```
        let gateway_id = req.body.metadata.gateways[0].gtw_id;
        req.body = {
            device: {
                deviceId: req.body.dev_id
            },
            measurements: req.body.payload_fields
        };
        req.body.measurements.gateway_id = gateway_id;
```

The resulting function should look like this in the Azure portal function editor:

![Azure function editing](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/azure-function.png)

### Hook the bridge to The Things Network application

This is the penultimate step to make data flow.  Return to your TTN application and go to the "Integrations" tab.  Click the "+ add integration" button and select the "HTTP Integration".  In the URL paste in the URL of your Azure function we just edited, and change the Method to "POST".  Click "Save" and you have connected your Things Network application to Azure IoT central.  Each time a payload comes into The Things Network it will be forwarded to Azure IoT Central.  The first time a device is new device comes online and sends data IoT Central will recognize that the device is new and create a new device for your in IoT Central application.  Return to your IoT Central application and click on the "Device Explorer" icon on the left hand side of the window.  Now click "Unassociated devices" and you should see your device listed, if you do not see your device click the devices button or give it a shake so it sends some data and then refresh browser.  Once the device shows up click the checkbox next to it and then the "Associate" icon on the toolbar.

![Azure IoT Central device association](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/associate.png)

We need to tell IoT Central what template to associate with this device so in the popup dialog select the device template you created earlier from the dropdown box and click "Associate".  Azure IoT Central will then create the device and associate it with the device template and it will disappear from the unassociated devices list.

Click the template name you created earlier in the left hand rail and you should see the device listed below the simulated device.  If you click on the device you should start to see data on the graph after a minute or so.

![Azure IoT Central data flowing](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/data-flowing.png)

## Troubleshooting

There could be an error in several places as we are making three pieces of software interact.  Here is my debugging work flow from device to IoT Central.

1. Is the data being sent from the device.  Open up the Serial Monitor in the Arduino IDE (baud rate:57600) and make sure that when you click the button data gets sent, the device may have gone inactive after a period of time to save battery. You should see something similar to below.  If you do not see data flowing then recompile the code and upload to the device and check that the right device type and port are selected.

    ```
    -- SEND: BUTTON (duration:  299ms)
    EUI: 0004A30B001B4D4C
    Battery: 3304
    AppEUI: 70B3D57ED00149CA
    DevEUI: 0004A30B001B4D4C
    Data Rate: 3
    RX Delay 1: 1000
    RX Delay 2: 2000
    Light: 24
    Temperature: 24.81 C
    Temperature alert: Yes
    Moving: No
    Button pressed: Yes
    Color: Blue
    USB connected: Yes
    Battery voltage: 4762 MV
    Acceleration (x,y,z): (-0.95,0.05,0.24)
    Sending: mac tx uncnf 4 12A0001909B1FFA10005001702
    Successful transmission
    Sending: sys sleep 60000
    -- SEND: INTERVAL
    EUI: 0004A30B001B4D4C
    Battery: 3304
    AppEUI: 70B3D57ED00149CA
    DevEUI: 0004A30B001B4D4C
    Data Rate: 3
    RX Delay 1: 1000
    RX Delay 2: 2000
    Light: 39
    Temperature: 24.81 C
    Temperature alert: Yes
    Moving: No
    Button pressed: No
    Color: Blue
    USB connected: Yes
    Battery voltage: 4768 MV
    Acceleration (x,y,z): (-0.96,0.06,0.25)
    Sending: mac tx uncnf 2 12A0002709B1FFA10005001902
    Successful transmission
    Sending: sys sleep 60000
    ```  

2. Make sure that the data is getting to The Things Network application.  Check the "Data" tab to make sure data shows up and is being decoded correctly.  If no data is flowing make sure you have registered the device with your application and changed the appEUI and appKey in the Arduino code to the right values for your application.  

    If the fields are corrupted or empty then there could be an issue twith the decoder function.  Go to the "Payload Formats" tab and paste a raw payload into the payload text box and click the "Test" button.  Did it decode correctly?  If not then debug the decoder function until you get the desired output.

3. Make sure you have an HTTP integration setup and configured correctly in the "Integrations" tab. This is needed to send the data to the Azure function for the Azure IoT Central Device Bridge, check the URL is correct and that the Method is set to "POST".

4. Go to the Azure function in the Azure portal and click the "Logs" button at the bottom of the window.  Every time the device sends data you should see messages output to the log.  
    ```
    2019-01-23T19:05:03  Welcome, you are now connected to log-streaming service.
    2019-01-23T19:05:10.353 [Information] Executing 'Functions.IoTCIntegration' (Reason='This function was programmatically called via the host APIs.', Id=389283b6-7371-447d-b456-dd9c9809fc7d)
    2019-01-23T19:05:10.365 [Information] [HTTP] Sending telemetry for device frogman-node
    2019-01-23T19:05:10.444 [Information] Executed 'Functions.IoTCIntegration' (Succeeded, Id=389283b6-7371-447d-b456-dd9c9809fc7d)
    2019-01-23T19:05:28.478 [Information] Executing 'Functions.IoTCIntegration' (Reason='This function was programmatically called via the host APIs.', Id=bbd069bc-a5cd-4edf-bec1-0364a1657720)
    2019-01-23T19:05:28.489 [Information] [HTTP] Sending telemetry for device frogman-node
    2019-01-23T19:05:28.532 [Information] Executed 'Functions.IoTCIntegration' (Succeeded, Id=bbd069bc-a5cd-4edf-bec1-0364a1657720)
    2019-01-23T19:06:31.066 [Information] Executing 'Functions.IoTCIntegration' (Reason='This function was programmatically called via the host APIs.', Id=f35ff93f-e59c-4eee-aae1-b2a32cd77d57)
    2019-01-23T19:06:31.077 [Information] [HTTP] Sending telemetry for device frogman-node
    2019-01-23T19:06:31.140 [Information] Executed 'Functions.IoTCIntegration' (Succeeded, Id=f35ff93f-e59c-4eee-aae1-b2a32cd77d57)
    2019-01-23T19:07:33.605 [Information] Executing 'Functions.IoTCIntegration' (Reason='This function was programmatically called via the host APIs.', Id=e613039e-b768-45e8-839c-9a0f694584d2)
    ```

    You may need to click the "Reconnect" button or refresh the page to get the logs to start flowing.  If you see errors in the log then you have a bug in the code you have added.  If you want to see what you are receiving from The Things Network add the following line just after the 'try {' line (currently line:20)

    ```
    context.log(req.body);
    ```
5. In your Azure IoT Central application make sure you have associated the device with a device template and the device is not stuck in the unassociated state awiting your approval.  Assuming you have associated but are not seeing all your data make sure you have clicked the eyeballs next to the measures so they are displayed on the graph.  Currently the selection of eyeballs is not preserved as state so you have to keep clicking them, bug fix coming shortly I hear :-)

Thats my list of troubleshooting steps, I'll update this further if I see people file Github issues having specific areas of difficulty.  The Github Issues area is also a great place to see if anyone else has had a similar issue to yours and either pile on the issue or see if has a resolution.  I'll try to be proactive on keeping up on issues.
    

## What now?

Thats it you have connected a Things Node to The Things Network and connected it to an Azure IoT Central application with just a few lines of code.  You now have all the power of Azure IoT Central at your disposal.  so you can:
* Set rules and actions using Microsoft Flow (https://docs.microsoft.com/en-us/azure/iot-central/howto-add-microsoft-flow) or Azure Logic Apps (https://docs.microsoft.com/en-us/azure/iot-central/howto-build-azure-logic-apps)
* Visualize your data in Power BI (https://docs.microsoft.com/en-us/azure/iot-central/howto-connect-powerbi)
* Pipe data to other Azure Services via IoT Central data export (https://docs.microsoft.com/en-us/azure/iot-central/howto-export-data)
* Plus many other things (https://docs.microsoft.com/en-us/azure/iot-central/)

### Things to try:

* Hack up the code and customize it to your needs, add more sensors (using The Things Network Uno) or change what the button press represents (door bell, garage opener, assistance needed, etc).
* Modify the IoT Central Bridge's Azure function to send more telemetry items to IoT central
* Modify the decoder function on The Things Network application to send a different payload.
* Be creative and if you create something cool share the code on Github.





































































