       ____  ____   ___    __ __  ____  ____    ___       ___     ___    ___   ____   _       ___     __  __  _ 
      /    ||    \ |   \  |  |  ||    ||    \  /   \     |   \   /   \  /   \ |    \ | |     /   \   /  ]|  |/ ]
     |  o  ||  D  )|    \ |  |  | |  | |  _  ||     |    |    \ |     ||     ||  D  )| |    |     | /  / |  ' / 
     |     ||    / |  D  ||  |  | |  | |  |  ||  O  |    |  D  ||  O  ||  O  ||    / | |___ |  O  |/  /  |    \ 
     |  _  ||    \ |     ||  :  | |  | |  |  ||     |    |     ||     ||     ||    \ |     ||     /   \_ |     \
     |  |  ||  .  \|     ||     | |  | |  |  ||     |    |     ||     ||     ||  .  \|     ||     \     ||  .  |
     |__|__||__|\_||_____| \__,_||____||__|__| \___/     |_____| \___/  \___/ |__|\_||_____| \___/ \____||__|\_|
                                                                                                           
===============
Arduino Door Lock Project
-------------------------

This is an automated door lock to help the Lansing Makers Network.  It features an RFID reader (125 Khz) that
reads RDIF tags and checks a database to see if that card is authorized.  If it is, it drives a car power-window
motor to squeeze the crashbar on the front door. 

Built using the Arduino Uno r2 board.

_Pinout:_


Digital 0 - Unconnected / USB Tx

Digital 1 - Unconnected / USB Rx

Digital 2 - RFID Enable Pin

Digital 3 - Blue LED (+)

Digital 4 - Green LED (+)

Digital 5 - Red LED (+)

Digital 6 - Motor Direction

Digital 7 - Motor Control (Relay)

Digital 8 - Unused

Digital 9 - Unused

Digital 10 - Keep Open Switch

Digital 11 - RFID Serial Out (SOUT) Pin

Digital 12 - Unused

Digital 13 - Unused



Analog 0 - Unused

Analog 1 - Unused

Analog 2 - Unused

Analog 3 - Unused

Analog 4 - Unused

Analog 5 - Unused
