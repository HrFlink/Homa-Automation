# GrovePi + Grove Temperature & Humidity Sensor
# http://www.seeedstudio.com/wiki/Grove_-_Temperature_and_Humidity_Sensor_Pro

import sys
sys.path.append('/home/pi/ha/Project/Core')
import grovepi

# Connect the Grove Temperature & Humidity Sensor Pro to digital port D4
# SIG,NC,VCC,GND
sensor = 4

try:
    [temp,humidity] = grovepi.dht(sensor,0)
    sys.stdout.write('%.2f' % humidity)

except IOError:
    print "Error"
