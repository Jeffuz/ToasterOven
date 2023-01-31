# Toaster Oven
Toaster Oven is a finite state machine (FSM) that implements event trigger state transitions with an idealized instantaneous transition. There are four states in this FSM:  SETUP, SELECTOR_CHANGE_PENDING, COOKING, and RESET_PENDING.  Six events are of concern, although any button event is possible:   TIMER_TICK, ADC_CHANGED, and the button events for buttons 3 and 4.

<img width="733" alt="Screenshot 2023-01-30 at 6 42 41 PM" src="https://user-images.githubusercontent.com/52511888/215649301-62ceb6b9-ede1-42f3-8568-b4cc1625f1cb.png">

## Files 
- BOARD.c/.h – Standard hardware library 
- Oled.h, Ascii.h – These files provides the interface for manipulating the oled Oled.   No .c files are needed here, as Lab7SupportLib contains compiled definitions for the functions in Oled.h.
- Buttons.h – The header file for the button event checker. Contains the prototype for the event checking function, and enums to describe possible button events: BUTTON_EVENT_NONE, BUTTON_EVENT_3UP, BUTTON_EVENT_1DN, etc.
- ADC.h - The header file for the ADC (Analog to Digital Converter): This compares to a digital control, which has only 2 possible values. Hardware called an analog-to-digital converter is used to translate these analog signals into digital values
that a processor can understand in the form of an integer.
- Lab7SupportLib.a – This is a “static library” file.  It’s similar to a *.o file.  It contains the executable code for the functions in Buttons.h and ADC.h. 
- Leds.h – a simple macro file.  LED 1 should toggle on each TimerA event, LED2 should toggle on each TimerB event, and LED3 should toggle on each TimerC event.
- Lab07_main.c – This file will contain all of the executable code

## Concepts
- const variables
- Timer interrupts
- Free Running Counters
- Event-driven programming
- Finite state machines

## Functionality 
- The system displays (on the OLED) the heating elements state in a little graphical toaster oven, the cooking mode, the current time (set time or remaining time when on), and the current temperature (except for when in toast mode). Additionally a greater-than sign (>) is used in Bake mode to   indicate   whether   time  or   temp   is   configurable   through   the potentiometer.
- First,   the  user   will  select   a   mode  and   configure  a   cook   time   and/or temperature:
  - The   toaster   oven   has   3   cooking   modes,   which   can   be   rotated through by pressing BTN3 for < 1s. They are, in order: bake, toast, and broil.
    - Bake mode:  Both temperature and time are configurable, with temperature defaulting to 350 degrees F and time to 0:01. Switching between temp and time can be done by holding   BTN3   for   >   1s   (defined   as   a   LONG_PRESS). Whichever is selected has an indicator beside its label (the selector should always default to time when entering this mode). Both top and bottom heating elements are used when cooking in bake mode.
    - Toast mode: Only the time can be configured in this mode, and the temperature is not displayed. There is no selector indicator on the display. Only the bottom heating elements come on in toast mode.
    - Broil mode: The temperature is fixed at 500 degrees F and only time is configurable in this mode. The temperature is displayed in broil mode. Again, the input selector indicator is not displayed. Only the top heating elements come on in broil mode. 
  - While   in   this   phase   of   operation,   the   user   can   rotate   the potentiometer to adjust the time or temperature.  There is a 2-state variable, called the “settings selector”,  which determines which setting the pot controls.
    - The settings selector is switched by holding BTN3 for >1 second.
    - The cooking time is derived from the ADC value obtained from the Adc library by using only the top 8 bits of the ADC reading and adding 1.  This results in a range from 0:01 to 4:16 minutes.
    - The cooking temperature is obtained from the potentiometer by using only the top 8 bits of the ADC value and adding 300 to it. This allows for temperatures between 300 and 555.
- Once a mode, time, and (if appropriate) temperature are selected, then cooking is started by pressing down on BTN4. This turns on the heating elements on the display (as they're otherwise off) and the LEDs (see below).
  - Cooking can be ended early by holding down BTN4 for >1 second. This should reset the toaster to the same cooking mode that it was in before the button press. 
    - Additionally, if the time/temp selector should return to their settings when baking started.   So, if the user selects 1:00 minute, then cooks for 30 seconds, then cancels cooking, the timer should now say 1:00 minute.
  - When the toaster oven is on, the 8 LEDs indicate the remaining cook time in a horizontal “progress bar” to complement the text on the OLED. At the start of cooking, all LEDs should be on.  After 1/8 of the total time has passed, LD1 will turn off. After another 1/8 of the original cook time, LD2 will turn off, and so on until all LEDs are off at the end.
  - After cooking is complete, the system will return to the current mode with the last used settings; the heating elements should be off, the time and temperature reset to the pot value, and the input selector displayed if in bake mode. 
