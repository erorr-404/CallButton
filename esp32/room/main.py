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

accept_btn = Button(16)
accept_btn.set_debounce_time(100)  # Set debounce time to 100 milliseconds

wait_btn = Button(19)
wait_btn.set_debounce_time(100)  # Set debounce time to 100 milliseconds

reject_btn = Button(21)
reject_btn.set_debounce_time(100)  # Set debounce time to 100 milliseconds


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

def wait_for_user_answear():
    for i in range(15):
        accept_btn.loop()
        wait_btn.loop()
        reject_btn.loop()
        print(f"Waiting for user response: {i}...")
        
        # todo: make esp return a response
        if accept_btn.is_pressed():
            print("User accepted request") 
        
        if wait_btn.is_pressed():
            print("User delayed request")
        
        if reject_btn.is_pressed():
            print("User rejected request")

pared = False 
peer = None # peer mac

while True:
    host, msg = e.recv()
    
    if msg: # msg == None if timeout in recv()
        
        if msg == b'pare': # command to create peer for 2-way connection
        print("Got pare command")
            if peer is None: # if it is the first message
                peer = host # save mac address
                e.add_peer(peer) # peer with device
                e.send(peer, "accept", True) # return that paring is succesful
                pared = True
                led.on()
                print("Successfully pared")
        
        elif msg == b'dinner':,
            print("Got dinner command")
            play_dinner_ready()
            for i in range(15):
                
                # todo: wait for user to press the button and return response
        
        elif msg == b'come':
            print("Got come command")
            play_come_here()
            # todo: wait for user to press the button and return response
        
        elif msg == b'end':
            break