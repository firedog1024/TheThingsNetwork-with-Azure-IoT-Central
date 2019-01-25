from network import LoRa
import socket
import time
import ubinascii
import uos
import struct

# from pysense import Pysense
# from LIS2HH12 import LIS2HH12
# from SI7006A20 import SI7006A20
# from LTR329ALS01 import LTR329ALS01
# from MPL3115A2 import MPL3115A2,ALTITUDE,PRESSURE

# global sensor instances
# py = Pysense()
# mp = MPL3115A2(py,mode=PRESSURE)
# si = SI7006A20(py)
# lt = LTR329ALS01(py)
# li = LIS2HH12(py)

# global variables
pressCounter = 0
lastSend = 0
lastPress = 0

# callback for confirmation and/or error in sending LoRa payloads, also called on a downlink
def lora_cb(lora):
    events = lora.events()
    if events & LoRa.RX_PACKET_EVENT:
        print('Lora packet received')
    if events & LoRa.TX_PACKET_EVENT:
        print('Lora packet sent')
    if events & LoRa.TX_FAILED_EVENT:
        print('Lora packet error')

# Initialise LoRa in LORAWAN mode.
# Please pick the region that matches where you are using the device:
# Asia = LoRa.AS923
# Australia = LoRa.AU915
# Europe = LoRa.EU868
# United States = LoRa.US915
lora = LoRa(mode=LoRa.LORAWAN, region=LoRa.US915)

lora.callback(trigger=(LoRa.RX_PACKET_EVENT | LoRa.TX_PACKET_EVENT | LoRa.TX_FAILED_EVENT), handler=lora_cb)

dev_eui = ubinascii.unhexlify('70B3D549982E2EE1')
# these values come from TTN application https://console.thethingsnetwork.org/applications/<app_name>
app_eui = ubinascii.unhexlify('70B3D57ED00149CA')
# app key is specific to the device https://console.thethingsnetwork.org/applications/<app_name>/devices/<device_name>
app_key = ubinascii.unhexlify('CA82B1FF37562E2D4DBC113417B9910A')

#lora.join(activation=LoRa.OTAA, auth=(app_eui, app_key), timeout=0)
# for channel in range(0, 72):
#     lora.add_channel(channel, frequency=903900000, dr_min=0, dr_max=3)

# join a network using OTAA
lora.join(activation=LoRa.OTAA, auth=(app_eui, app_key), timeout=0, dr=0)

# wait until the module has joined the network
# print('Waiting to join network .', end='')
# while not lora.has_joined():
#     time.sleep(2.5)
#     print('.', end='')
# print('')

join_wait = 0
while not lora.has_joined():
    time.sleep(2.5)
    if not lora.has_joined():
        print('Not joined yet…')
        join_wait += 1
    if join_wait == 5:
        lora.join(activation=LoRa.OTAA, auth=(app_eui, app_key), timeout=0, dr=0)
        join_wait = 0

# create a LoRa socket
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)

# set the LoRaWAN data rate
s.setsockopt(socket.SOL_LORA, socket.SO_DR, 0)
s.setsockopt(socket.SOL_LORA, socket.SO_CONFIRMED, True)

# none blocking so as to not block the main loop
s.setblocking(True)
s.bind(1) 

# once joined we can start sending data
while lora.has_joined():
    if time.ticks_ms() - lastSend > 10000:
        # pull new values from the sensors
        # temperature = si.temperature()
        # humidity = si.humidity()
        # pressure = mp.pressure()
        # battery_voltage = py.read_battery_voltage()
        # acceleration_xyz = li.acceleration()
        # b_r_lux = lt.light()

        temperature = 2350
        battery_voltage = 3456
        acceleration_x = 90
        acceleration_y = 30
        acceleration_z = 20
        lux = 56

        # debug output to console
        print('temperature: {}, battery: {}'.format(temperature, battery_voltage))
        print ('lux: {}'.format(lux))
        print('Acceleration: x: {}, y: {}, z: {}'.format(acceleration_x, acceleration_y, acceleration_z))

        # package the data as a byte array and send
        data = struct.pack('<hhhhhhB', battery_voltage, lux, temperature, acceleration_x, acceleration_y, acceleration_z, 0)
        s.send(data)
        lastSend = time.ticks_ms()

    # polling for button presses, could not get IRQ code to work :-(
    # if py.button_pressed() and time.ticks_ms() - lastPress > 1000:
    #     if pressCounter < 100:
    #         pressCounter = pressCounter + 1
    #     else:
    #         pressCounter = 0
    #     lastPress = time.ticks_ms()
