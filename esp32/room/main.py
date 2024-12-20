from dfplayermini import Player
from DIYables_MicroPython_Button import Button
from time import sleep, time
import network
import espnow
from machine import Pin


# Enable the Mini MP3 DFPlayer
player = Player(pin_TX=17, pin_RX=16)
player.volume(30)

led = Pin(2, Pin.OUT)

accept_btn = Button(22)
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

peer = b'<\x8a\x1f\x9b\xcd\x00'   # MAC address of peer's wifi interface
e.add_peer(peer)      # Must add_peer() before send()

def wait_for_button():
    print("Чекаю на натискання кнопки...")
    start_time = time()
    while time() - start_time < 15:  # Чекаємо максимум 15 секунд
        accept_btn.loop()
        wait_btn.loop()
        reject_btn.loop()
        
        if accept_btn.is_pressed():
            print("User accepted request")
            return b'yes'
        
        if wait_btn.is_pressed():
            print("User delayed request")
            return b'wait'
        
        if reject_btn.is_pressed():
            print("User rejected request")
            return b'no'

    print("Час очікування вийшов!")
    return None

while True:
    accept_btn.loop()
    wait_btn.loop()
    reject_btn.loop()
    
    host, msg = e.recv()
    
    if msg: # msg == None if timeout in recv()
        print(f"HOST: {host}; MSG: {msg}")
        
        if msg == b'dinner':
            print("Got dinner command")
            led.on()
            player.play(2)
            
            resp = wait_for_button()
            if resp:
                e.send(peer, resp)
                print("Відповідь надіслано!")
            else:
                print("Кнопка не була натиснута. Відповідь не надіслана.")

        
        elif msg == b'come':
            print("Got come command")
            led.on()
            player.play(1)
            
            resp = wait_for_button()
            if resp:
                e.send(peer, resp)
                print("Відповідь надіслано!")
            else:
                print("Кнопка не була натиснута. Відповідь не надіслана.")
        
        elif msg == b'end':
            break
        
        led.off()