## Arduino Due in Atmel Studio with ARM SAM3X8E
- uses UART with PDC, TC0 timer Interrupt, Port PIOB with Interrupt
- for Arduino Due in Atmel Studio 7 - gcc c compiler

## How To
- Create new Atmel Studio 7 SAM3X8E gcc c project
- Check-out/paste main.c
- Build, then use Arduino's bossac.exe to upload binary
- Use serial console (termite.exe?) to Connect to Due's programming serial port (8N1@115200)
   - send 'a' - led should be blinking fast
   - send 'b' - led should be blinking slow
   - short Arduino pin 22 to ground - message 'Interrupt...' should appear on your serial console

## What it does
- uses Atmel's SystemInit() to initialize crystal oscillator, PLL loop @84MHz, switch to APLL as master clock
- initializes PIOB pin PB27 as output (Arduino Yellow Led)
- initializes PIOB pin PB26 as input (with slow clock debouncing and interrupt on falling edge)(Arduino Pin22)
- initializes TC0 with slow clock (32kHz) and interrupt on RC register match (every 2 seconds)
- initializes UART @ 8N1 - 115200baud with interrupt on receive byte
- disable watchdog timer (defaults to 16sec timeout on reset)
- on TC0 IRQ 
   - flip LED
- on UART IRQ receive:  
   - if 'a' received set reload value for TC0 to a 1/3rd of a sec - fast led blipping
   - if 'b' then set reload value to match whole second - slow led blipping
- on PIOB PB26 IRQ (falling edge): 
   - initialize PDC channel for UART to send text message 'Interrupt on PB26'
   - and flip Led

## Credits
- http://codetron.net/arduino-due-in-atmel-studio-using-c-led-blinking/
- http://codetron.net/uart-interface-sam3x8e-arduino-due/
- http://www.jaxcoder.com/Article/SinglePost?postID=834972618
- http://www.cloud-rocket.com/2014/05/programming-arduino-due-atmel-studio-asf/ - bossac.exe howto

## License
Do whatever you want. No warranty though.


