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

def send_request(data):
    print("Надсилаю запит...")
    e.send(peer, data)
    print("Чекаю на відповідь...")
    led.on()
    
    start_time = time()
    while time() - start_time < 20:  # Чекаємо максимум 20 секунд
        dinner_btn.loop() # inform about loop
        come_btn.loop() # inform about loop
        
        if e.any():
            host, msg = e.recv()
            if msg:
                print(f"Отримано відповідь від сервера!: {msg}")
                led.off()
                return msg
    print("Час очікування вийшов!")
    led.off()
    return None


# A WLAN interface must be active to send()/recv()
sta = network.WLAN(network.WLAN.IF_STA)
sta.active(True)

# Init ESPNow protocol
e = espnow.ESPNow()
e.active(True)

peer = b'<\x8a\x1f\x9d\x1e\x1c'   # MAC address of peer's wifi interface
e.add_peer(peer)      # Must add_peer() before send()

while True:
    dinner_btn.loop() # inform about loop
    come_btn.loop() # inform about loop
    
    if dinner_btn.is_pressed():
        print("The come button is pressed")
        success = send_request(b'dinner')
        if success:
            print(f"Операція успішна!: {success}")
            if success == b'yes':
                player.play(3)
            elif success == b'wait':
                player.play(2)
            elif success == b'no':
                player.play(1)
            else:
                pared_led.on()
        else:
            print("Не вдалося отримати відповідь.")
    
    if come_btn.is_pressed():
        print("The come button is pressed")
        success = send_request(b'come')
        if success:
            print(f"Операція успішна!: {success}")
            if success == b'yes':
                player.play(3)
            elif success == b'wait':
                player.play(2)
            elif success == b'no':
                player.play(1)
            else:
                pared_led.on()
        else:
            print("Не вдалося отримати відповідь.")
        
e.send(peer, b'end', True)
