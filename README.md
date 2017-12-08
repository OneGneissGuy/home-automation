
# My Home Assistant Setup #
## 0. Setup
- Setup your Raspberry Pi like [this](https://hackernoon.com/raspberry-pi-headless-install-462ccabd75d0)
## 1. Installed devices
- [Raspberry Pi 3 model B](https://www.raspberrypi.org/products/raspberry-pi-3-model-b/)
- [Z-Stick Gen5](https://aeotec.com/z-wave-usb-stick)
- [GE 14291 Z-Wave Plus In-Wall Paddle Smart ON/OFF Switch](https://www.zwaveproducts.com/shop/brands/ge/14291-ge-14291-z-wave-plus-in-wall-paddle-smart-on-off-switch) 
- [Wemo® Switch Smart Plug](http://www.belkin.com/us/p/P-F7C027/)
- [Adafruit Feather HUZZAH ESP8266](https://www.adafruit.com/product/2821)
- [Adafruit BME280 I2C or SPI Temperature Humidity Pressure Sensor](https://www.adafruit.com/product/2652)
- [Amcrest IP2M-842 HD WiFi Camera](https://amcrest.com/amcrest-1080p-bullt-wifi-video-security-ip-camera-pt-ip2m-842-white.html)
- [GoControl Z-Wave Dimmable LED Light Bulb, LB60Z-1](https://www.amazon.com/gp/product/B00PJH16UC/ref=oh_aui_detailpage_o00_s01?ie=UTF8&psc=1)
## 2. Installed software
- [Hassbian](https://home-assistant.io/docs/installation/hassbian/)
- [DuckDNS](https://www.duckdns.org/install.jsp) for pi
- [Let's Encrypt](https://github.com/letsencrypt)

## 3. Useful commands
* Config the raspberry pi (things like keyboard layout, timezone, etc.)

```sh
sudo raspi-config
```

* Change the default password (for user username)

```sh
sudo passwd username
```

* Prevent the wifi device from going to sleep

```sh
sudo iw dev wlan0 set power_save off
```

* Turn off the pi's wifi device

```sh
sudo ifwconfig wlan0 txpower off
```

* Give homeassistant access to the Pi's GPIO pins

```sh
sudo usermod -G gpio -a homeassistant
```

* Update hassbian

```sh
sudo hassbian-config upgrade hassbian
```

* Update homeassistant

```sh
sudo hassbian-config upgrade home-assistant
```

* Restart homeassistant

```sh
sudo systemctl restart home-assistant@homeassistant
```

*  List the available install scripts for some useful services

```sh
sudo hassbian-config show
```
* Install some useful services

```sh
# Install a mosquitto MQTT server
sudo hassbian-config install mosquitto
# Install samba to view and edit config files as a network location on a PC
sudo hassbian-config install samba
```

* Edit the wifi configuration

```sh
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
```

* List connected USB devices

```sh
lsusb
```

* Install [hassctl](https://github.com/dale3h/hassctl) to more easily control home-assistant

```sh
sudo curl -o /usr/local/bin/hassctl https://raw.githubusercontent.com/dale3h/hassctl/master/hassctl && sudo chmod +x /usr/local/bin/hassctl
```

* hassctl method to update home-assistant

```sh
hassctl update-hass && hassctl config && hassctl restart
```

* Reboot the pi

```sh
sudo reboot now
```
