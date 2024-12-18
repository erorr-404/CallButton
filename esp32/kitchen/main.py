from dfplayermini import Player
from DIYables_MicroPython_Button import Button
from time import sleep
import network
import espnow
from machine import Pin


# Enable the Mini MP3 DFPlayer
player = Player(pin_TX=17, pin_RX=16)
player.volume(30)
player.module_sleep()

led = Pin(2, Pin.OUT)

dinner_btn = Button(21)
dinner_btn.set_debounce_time(100)  # Set debounce time to 100 milliseconds


# Play sound and go sleep
def play_no():
    player.module_wake()
    player.play(1)
    player.module_sleep()


# Play sound and go sleep
def play_5_mins():
    player.module_wake()
    player.play(2)
    player.module_sleep()

# Play sound and go sleep
def play_yes():
    player.module_wake()
    player.play(3)
    player.module_sleep()

# A WLAN interface must be active to send()/recv()
sta = network.WLAN(network.WLAN.IF_STA)
sta.active(True)

# Init ESPNow protocol
e = espnow.ESPNow()
e.active(True)

peer = b'<\x8a\x1f\x9d\x1e\x1c'   # MAC address of peer's wifi interface
e.add_peer(peer)      # Must add_peer() before send()
e.send(peer, "pare", True)

while True:
    dinner_btn.loop()

    if dinner_btn.is_pressed():
        print("The button is pressed")
        led.on()

    if dinner_btn.is_released():
        print("The button is released")
        led.off()

e.send(peer, b'end', True)
