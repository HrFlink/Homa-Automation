# Grovepi + grove RGB LCD module
# http://www.seeedstudio.com/wiki/Grove_-_LCD_RGB_Backlight.
# 
# Henrik Vestergaard 2014
#
# Args: line [01] clear [01] text1 text2

import sys
sys.path.append('/home/pi/ha/Project/Core')

from grove_rgb_lcd import *

def setCursor(col, row):
        col = (col | 0x80) if row == 0 else (col | 0xc0)
        textCommand(col)


if __name__=="__main__":
    setCursor(0, 1) if sys.argv[1]=="1" else setCursor(0, 0)


    textCommand(0x28) # 2 lines
    textCommand(0x08|0x04) # display on, no cursor
    time.sleep(0.05)

    if sys.argv[2]=="1":
        textCommand(0x01) # clear display
        time.sleep(0.05)

    text = sys.argv[3] + sys.argv[4]

    count = 0
    row=sys.argv[1]
    for c in text:
        count+=1
        bus.write_byte_data(DISPLAY_TEXT_ADDR,0x40,ord(c))

    setRGB(0,128,64)
    time.sleep(10)
    setRGB(0,0,0)
