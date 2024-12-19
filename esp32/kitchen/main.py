from dfplayermini import Player
from DIYables_MicroPython_Button import Button
from time import sleep
import network
import espnow
from machine import Pin


# Enable the Mini MP3 DFPlayer
player = Player(pin_TX=17, pin_RX=16)
player.volume(10)
player.module_sleep()

led = Pin(2, Pin.OUT)
pared_led = Pin(18, Pin.OUT)

dinner_btn = Button(21)
dinner_btn.set_debounce_time(100)  # Set debounce time to 100 milliseconds

come_btn = Button(19)
come_btn.set_debounce_time(100)  # Set debounce time to 100 milliseconds


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

pared = False 
peer = b'<\x8a\x1f\x9d\x1e\x1c'   # MAC address of peer's wifi interface
e.add_peer(peer)      # Must add_peer() before send()

for i in range(30):
    e.send(peer, "pare", True) # Ping the second ESP
    print(f"Trying to pare: {i}...")
    
    host, msg = e.recv()
    if msg: # msg == None if timeout in recv()
        if msg == "accept": # if paring is successful
            pared = True
            pared_led.on()
            print("Pare created!")
            break
    
    sleep(1) # wait a second to save battery

while True:
    dinner_btn.loop() # inform about loop
    come_btn.loop() # inform about loop
    
    if dinner_btn.is_pressed():
        print("The dinner button is pressed")
        led.on()

    if dinner_btn.is_released():
        print("The dinner button is released")
    
    if come_btn.is_pressed():
        print("The come button is pressed")
        led.off()

    if come_btn.is_released():
        print("The come button is released")
    
    if pared:
        pass
    else:
        break
        
e.send(peer, b'end', True)
