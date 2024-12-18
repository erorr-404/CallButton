from dfplayermini import Player
from time import sleep
import network
import espnow
from machine import Pin


# Enable the Mini MP3 DFPlayer
player = Player(pin_TX=17, pin_RX=16)
player.volume(30)
player.module_sleep()


led = Pin(2, Pin.OUT)


# Play sound and go sleep
def play_come_here():
    player.module_wake()
    player.play(1)
    player.module_sleep()


# Play sound and go sleep
def play_dinner_ready():
    player.module_wake()
    player.play(2)
    player.module_sleep()


# A WLAN interface must be active to send()/recv()
sta = network.WLAN(network.WLAN.IF_STA)
sta.active(True)

# Init ESPNow protocol
e = espnow.ESPNow()
e.active(True)

# Print device mac-address
print(sta.config('mac').hex())
print(sta.config('mac'))

peer = None # peer mac

while True:
    host, msg = e.recv()
    
    if msg: # msg == None if timeout in recv()
        
        if msg == b'pare' # command to create peer for 2-way connection
        print("Got pare command")
            if peer is None: # if it is the first message
                peer = host # save mac address
                e.add_peer(peer) # peer with device
        
        elif msg == b'dinner':,
            print("Got dinner command")
            play_dinner_ready()
            # todo: wait for user to press the button and return response
        
        elif msg == b'come':
            print("Got come command")
            play_come_here()
            # todo: wait for user to press the button and return response
        
        elif msg == b'end':
            break