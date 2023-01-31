// Standard libraries
#include <stdio.h>

#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>

// libraries 
#include "Leds.h"
#include "Ascii.h"
#include "Oled.h"
#include "Adc.h"
#include "Buttons.h"
#include <string.h>

// Macro for resetting the timer
#define TIMER_2HZ_RESET() (TMR1 = 0)

// typedefs
typedef enum {
    SETUP, SELECTOR_CHANGE_PENDING, COOKING, RESET_PENDING
} OvenState;

typedef enum {
    BAKE, TOAST, BROIL
} OvenModes;

typedef enum {
    TIME, TEMP
} OvenSelector;

typedef struct {    
    //add more members to this struct
    OvenState state;
    OvenModes modes;
    OvenSelector selector;
    uint8_t event;
    int time; 
    int starttime;
    int currenttime;
    int temperature;
    int global_time;
    
} OvenData;

// Global Variables
#define WINDOW 5
#define BAKE_TEMP_MIN 300
#define BAKE_TEMP_MAX 555
#define BROIL_TEMP_MAX 500
#define VOLTAGE_MAX 1023

int count = 5;

OvenData dataOven; 

// Helper Function

void updateOvenOLED(OvenData ovenData){ // Update LED to reflect state of oven   
    // assign
    char line[100];        
    char oven_top_off[100];
    char oven_bottom_off[100];
    char oven_top_on[100];
    char oven_bottom_on[100];
    char oven_mode[50];
    int min = dataOven.time/60;
    int sec = dataOven.time%60;
    int temp = dataOven.temperature;
    
    OledClear(OLED_COLOR_BLACK);
     
    // COOKING STATE
    sprintf(oven_top_off, "|" OVEN_TOP_OFF OVEN_TOP_OFF OVEN_TOP_OFF OVEN_TOP_OFF OVEN_TOP_OFF "|");
    sprintf(oven_bottom_off, "|" OVEN_BOTTOM_OFF OVEN_BOTTOM_OFF OVEN_BOTTOM_OFF OVEN_BOTTOM_OFF OVEN_BOTTOM_OFF "|");
    sprintf(oven_top_on, "|" OVEN_TOP_ON OVEN_TOP_ON OVEN_TOP_ON OVEN_TOP_ON OVEN_TOP_ON "|");
    sprintf(oven_bottom_on, "|" OVEN_BOTTOM_ON OVEN_BOTTOM_ON OVEN_BOTTOM_ON OVEN_BOTTOM_ON OVEN_BOTTOM_ON "|");
    
    // COOKING MODES
    if (dataOven.modes == BAKE){

        if (dataOven.selector == TIME){
            if (dataOven.state == COOKING){
                sprintf(line,"%s  Mode: BAKE\n", oven_top_on);
            } else {
                sprintf(line,"%s  Mode: BAKE\n", oven_top_off);
            }
            sprintf(line + strlen(line),"|     | >Time: %d:%02d\n",min, sec);
            sprintf(line + strlen(line),"|-----|  Temp: %3i%sF\n", temp, DEGREE_SYMBOL);
            if (dataOven.modes == COOKING){
                sprintf(line + strlen(line),"%s", oven_bottom_on);
            } else {
                sprintf(line + strlen(line),"%s", oven_bottom_off);
            }                                 
        }      
        if (dataOven.selector == TEMP){
            if (dataOven.state == COOKING){
                sprintf(line,"%s  Mode: BAKE\n", oven_top_on);
            } else {
                sprintf(line,"%s  Mode: BAKE\n", oven_top_off);
            }
            sprintf(line + strlen(line),"|     |  Time: %d:%02d\n",min, sec);
            sprintf(line + strlen(line),"|-----| >Temp: %3i%sF\n", temp, DEGREE_SYMBOL);
            if (dataOven.modes == COOKING){
                sprintf(line + strlen(line),"%s", oven_bottom_on);
            }
                sprintf(line + strlen(line),"%s", oven_bottom_off);                                 
        }
        
    } else if (dataOven.modes == TOAST){
        sprintf(line,"%s  Mode: TOAST\n", oven_top_off);
        sprintf(line + strlen(line),"|     |  Time: %d:%02d\n",min, sec);      
        sprintf(line + strlen(line),"|-----|\n");
        sprintf(line + strlen(line),"%s", oven_bottom_on);      
    } else if (dataOven.modes == BROIL){
        sprintf(line,"%s  Mode: BROIL\n", oven_top_on);
        sprintf(line + strlen(line),"|     |  Time: %d:%02d\n",min, sec);
        sprintf(line + strlen(line),"|-----|  Temp: 500\n", temp, DEGREE_SYMBOL);
        sprintf(line + strlen(line),"%s", oven_bottom_off);
    }       
    OledDrawString(line);
    OledUpdate();    
}

// Execute state machine
void runOvenSM(void)
{   
    //write your SM logic here.
    int elapsed_time;
    uint8_t AdcValue;
    uint8_t AdcTemp;
    switch(dataOven.state){
        case SETUP:
        default:
            if (dataOven.modes == BAKE){
                if (dataOven.selector == TIME){
                    AdcValue = AdcRead() >> 2; 
                    dataOven.time = AdcValue + 1;                     
                } 
                if (dataOven.selector == TEMP){
                    AdcTemp = AdcRead() >> 2;
                    dataOven.temperature = AdcTemp + 300;                     
                }                 
            } else {
               if (dataOven.selector == TIME){
                    AdcValue = AdcRead() >> 2; 
                    dataOven.time = AdcValue + 1;          
                }  
            }
            updateOvenOLED(dataOven);
            break;
        case SELECTOR_CHANGE_PENDING:
            elapsed_time = dataOven.currenttime - dataOven.starttime;         
            if (elapsed_time < 5){
                if (dataOven.modes == BAKE){
                    dataOven.modes = TOAST;
                } else if (dataOven.modes == TOAST){
                    dataOven.modes = BROIL;               
                } else {
                    dataOven.modes = BAKE;                   
                }  

            } else {
                if (dataOven.selector == TIME){
                    dataOven.selector = TEMP;             
                } else { 
                    dataOven.selector = TIME;
                }                                                      
            } 
            updateOvenOLED(dataOven);
            dataOven.state = SETUP;
            break;  
        case COOKING:
            updateOvenOLED(dataOven);
            break;
        case RESET_PENDING:
            elapsed_time = dataOven.currenttime - dataOven.starttime;   
            if (elapsed_time >= 5){
                dataOven.modes = BAKE;
                dataOven.selector = TIME;
                dataOven.state = SETUP;
                dataOven.temperature = 300;
                dataOven.time = 0;  
            }
            updateOvenOLED(dataOven);
            break;            
    }      
}    


int main()
{
    BOARD_Init();

    T2CON = 0; // everything should be off
    T2CONbits.TCKPS = 0b100; // 1:16 prescaler
    PR2 = BOARD_GetPBClock() / 16 / 100; // interrupt at .5s intervals
    T2CONbits.ON = 1; // turn the timer on

    // Set up the timer interrupt with a priority of 4.
    IFS0bits.T2IF = 0; //clear the interrupt flag before configuring
    IPC2bits.T2IP = 4; // priority of  4
    IPC2bits.T2IS = 0; // subpriority of 0 arbitrarily 
    IEC0bits.T2IE = 1; // turn the interrupt on

    T3CON = 0; // everything should be off
    T3CONbits.TCKPS = 0b111; // 1:256 prescaler
    PR3 = BOARD_GetPBClock() / 256 / 5; // interrupt at .5s intervals
    T3CONbits.ON = 1; // turn the timer on

    // Set up the timer interrupt with a priority of 4.
    IFS0bits.T3IF = 0; //clear the interrupt flag before configuring
    IPC3bits.T3IP = 4; // priority of  4
    IPC3bits.T3IS = 0; // subpriority of 0 arbitrarily 
    IEC0bits.T3IE = 1; // turn the interrupt on;
   
    printf("Toaster Oven.  Compiled on %s %s.", __TIME__, __DATE__);

    //initialize state machine 
    AdcInit();
    OledInit();
    ButtonsInit();
    dataOven.modes = BAKE;
    dataOven.selector = TIME;
    dataOven.state = SETUP;
    dataOven.temperature = 300;
    dataOven.time = 0;
    updateOvenOLED(dataOven);
    while (1){  
        if (dataOven.event == TRUE){
            runOvenSM(); 
            dataOven.event = FALSE;
        }
    };
}

/*The 5hz timer is used to update the free-running timer and to generate TIMER_TICK events*/
void __ISR(_TIMER_3_VECTOR, ipl4auto) TimerInterrupt5Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 12;

    // Event-checking code 
    dataOven.global_time ++;
    
    // timer
    if (dataOven.state == COOKING || dataOven.state == RESET_PENDING){
        count --;
        if (count == 0){
            dataOven.event = TRUE;
            if (dataOven.time > 0){
                dataOven.time--;                      
            } else {
                dataOven.state = SETUP;
            }
            count = 5;
        }        
    }
}

/*The 100hz timer is used to check for button and ADC events*/
void __ISR(_TIMER_2_VECTOR, ipl4auto) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8; 
    
    // Event-checking code

    uint8_t ButtonCheck = ButtonsCheckEvents();
    if (AdcChanged() == TRUE){
        dataOven.event = TRUE;
    } 
    if (ButtonCheck == BUTTON_EVENT_3DOWN && dataOven.state == SETUP){
        dataOven.starttime = dataOven.global_time;
        dataOven.state = SELECTOR_CHANGE_PENDING;
    }
    if (ButtonCheck == BUTTON_EVENT_3UP){
        dataOven.currenttime = dataOven.global_time;
        dataOven.event = TRUE;
    }
    if (ButtonCheck == BUTTON_EVENT_4DOWN && dataOven.state == SETUP){
        dataOven.starttime = dataOven.global_time;
        dataOven.state = COOKING;
    } 
    if (ButtonCheck == BUTTON_EVENT_4UP){
        dataOven.currenttime = dataOven.global_time;
        dataOven.event = TRUE;
    }
    

}