
BOARD = esp32
CHIP = esp32
ARDUINO_DIR   = /Applications/Arduino.app/Contents/Java
#ARDUINO_DIR   = $(HOME)/src/christo/Arduino
UPLOAD_PORT = /dev/cu.SLAB_USBtoUART
UPLOAD_SPEED = 921600
SKETCH = twister-strip.ino


include $(HOME)/src/other/makeEspArduino/makeEspArduino.mk
