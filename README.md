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

Now that we have our development environment setup we need to setup an account on The Things Network (if you don't have one already) and then create an application and register our device.  Follow the instructions here https://www.thethingsnetwork.org/docs/devices/node/quick-start.html#get-your-device-eui thru **Register your Device**.  After doing this your device will be registered on The Things Network and you should have both an App EUI and a App Key.  If you look at the bootom of the device information page you should see EXAMPLE CODE you can copy the code in the text box for use in the sample code.

![Example code](https://github.com/firedog1024/TheThingsNetwork-with-Azure-IoT-Central/raw/master/assets/examplecode.png)

## Sending data from the device















