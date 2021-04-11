#include <stdint.h>
#include "definitions.h"
#include "SysTick.h"
#include "onboardLEDs.h"

#define DELAY 1


// Initialize onboard LEDs
void onboardLEDs_Init(void){
	
    //Use PortN onboard LEDs (PN0, PN1)
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;				        // activate clock 
    while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R12) == 0){};	            // allow time for clock to stabilize
    GPIO_PORTN_DIR_R |= 0x03;        								// make PN0,PN1 out 
    GPIO_PORTN_AFSEL_R &= ~0x03;     								// disable alt funct on PN0,PN1																	
    GPIO_PORTN_AMSEL_R &= ~0x03;     								// disable analog functionality on PN0,PN1
    GPIO_PORTN_DEN_R |= 0x03;        								// enable digital I/O on PN0,PN1

	//Use PortF onboard LEDs (PF0, PF4)
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;				        // activate clock
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};             	// allow time for clock to stabilize
	GPIO_PORTF_DIR_R |= 0x11;        								// make PF0,PF4 out 
    GPIO_PORTF_AFSEL_R &= ~0x11;     								// disable alt funct on PF0,PF4
    GPIO_PORTF_DEN_R |= 0x11;        								// enable digital I/O on PF0,PF4
    GPIO_PORTF_AMSEL_R &= ~0x011;     								// disable analog functionality on PF0,PF4
		
	FlashAllLEDs();
	return;
}


void FlashAllLEDs(){
		GPIO_PORTN_DATA_R ^= 0x3; 								
		GPIO_PORTF_DATA_R ^= 0x11; 									
		delayMilliseconds(250);														
		GPIO_PORTN_DATA_R ^= 0x3;			
		GPIO_PORTF_DATA_R ^= 0x11; 								
		delayMilliseconds(250);																	
}

//Flash D1
void FlashLED1(int count) {
    while(count--) {
        GPIO_PORTN_DATA_R ^= 0b00000010; 								
        SysTick_Wait10ms(DELAY);														
        GPIO_PORTN_DATA_R ^= 0b00000010;			
        SysTick_Wait10ms(DELAY);														
    }
}

//Flash D2
void FlashLED2(int count) {
    while(count--) {
        GPIO_PORTN_DATA_R ^= 0b00000001; 								
        SysTick_Wait10ms(DELAY);														
        GPIO_PORTN_DATA_R ^= 0b00000001;			
        SysTick_Wait10ms(DELAY);																	
    }
}

//Flash D3
void FlashLED3(int count) {
    while(count--) {
        GPIO_PORTF_DATA_R ^= 0b00010000; 								
        SysTick_Wait10ms(DELAY);														
        GPIO_PORTF_DATA_R ^= 0b00010000;			
        SysTick_Wait10ms(DELAY);																	
    }
}

//Flash D4
void FlashLED4(int count) {
    while(count--) {
        GPIO_PORTF_DATA_R ^= 0b00000001; 								
        SysTick_Wait10ms(DELAY);														
        GPIO_PORTF_DATA_R ^= 0b00000001;			
        SysTick_Wait10ms(DELAY);																	
    }
}

void FlashI2CTx(){
	FlashLED1(1);
}

void FlashI2CRx(){
	FlashLED2(1);
}

//Flash Error
void FlashI2CError(int count) {
    while(count--) {
        FlashAllLEDs();
    }
}
