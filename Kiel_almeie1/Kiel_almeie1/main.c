/*
Elston Almeida
400114780
almeie1
Bus Speed: 96 MHz 
LED: PN0 and PL4
*/

#include <stdint.h>

#include "PLL.h"
#include "SysTick.h"
#include "definitions.h"
#include "uart.h"
#include "I2C.h"
#include "VL53L1X_api.h"


// Notes: Default address for the VL53LX is 0x29
// I2C requires the 0 bit to indicate if a read or write is needed.
// So we offset the address to [7:1] giving us DEV = 0x52
#define DEV 0x52

void toggleLED_OnBoard(void);
void toggleLED_External(void);
void setupGPIO(void);
void getData(void); 

void StepperMotor_StepForward(void);
void StepperMotor_StepBackward(void);
void StepperMotor_Sleep(void);

void VL53L1X_XSHUT(void);
void VL53L1X_Setup(void);
void VL53L1X_RangeInit(void);
uint16_t VL53L1X_Distance(void);


// INPUT LED SEQUENCE

int main(void)
{
    setupGPIO();
    PLL_Init();
    SysTick_Init();
    I2C_Init();
    UART_Init();
    toggleLED_External();
	VL53L1X_Setup();
	VL53L1X_RangeInit();

	while(1)
    {
        //negative logic
        if((GPIO_PORTJ_DATA_R & 0x1) == 0)
            getData();

        delayMilliseconds(50);
    }   
    //VL53L1X_StopRanging(DEV);
}



void getData(void)
{
	// Ensure PL4 LED is off
	// toggleLED_External();

	// 4096 steps; smStep has 4 core steps (8 total)
	// So we need to call 4096/8 = 512
	// Stop at each 45 deg => 360/8 => 512/8 = 64 steps.
    uint16_t DIVISIONS = 512;
	for (uint16_t i = 0; i < DIVISIONS; i++)
	{
		for (uint16_t j = 0; j < (uint16_t)(512/DIVISIONS); j++)
		{
			StepperMotor_StepForward();
		}
        // Turn on Distance LED
        GPIO_PORTN_DATA_R = 0x1;
        // Get distance
		uint16_t dist = VL53L1X_Distance();
        // Send Word
		UART_OutWord(dist);
        // Wait 100ms
		SysTick_Wait10ms(10);
        // Turn off Distance LED
        GPIO_PORTN_DATA_R = 0x0;
	}
    for(uint16_t i = 0; i < DIVISIONS*(uint16_t)(512/DIVISIONS); i++)
        StepperMotor_StepBackward();
	// Ensure we do not cook the motor
	StepperMotor_Sleep();
	toggleLED_External();
}

/*
	VL53L1X_Setup

	Description: 
	Sensor setup for the VL53L1X (Time of Flight Sensor). Ensures that interfacing with the
	sensor works properly and setting the sensor to use a default configuration.

	You must call the function VL53L1X_StartRanging after this initialization
*/
void VL53L1X_Setup(void)
{
	uint32_t status = 0;
	uint8_t  sensorState = 0;

	uint8_t modelID;
	uint8_t moduleType;

	UART_printf("[VL53L1X_Setup]: VL53L1X Booting\n");

	status = VL53L1_RdByte(DEV, 0x010F, &modelID);
	status = VL53L1_RdByte(DEV, 0x0110, &moduleType);

	sprintf(printf_buffer, "[VL53L1X_Setup]: Module ID: 0x%x , Module Type: 0x%x\r\n", modelID, moduleType);
	UART_printf(printf_buffer);

	while(sensorState == 0)
	{
		status = VL53L1X_BootState(DEV, &sensorState);
		SysTick_Wait10ms(10);
	}

	UART_printf("[VL53L1X_Setup]: VL53L1X Booted\n");

	status = VL53L1X_ClearInterrupt(DEV);

	status = VL53L1X_SensorInit(DEV);
	if (status != 0)
	{
		sprintf(printf_buffer, "[VL53L1X_Setup]: SensorInit failed with status: %d\n", status);
		UART_printf(printf_buffer);
	}

	return;
}


/*
	VL53L1X_StartRanging

	Description: 
	Configures the ranging options and enables the VL53L1X to start reading distances.
	Ranging must not be enabled when changing the timing budget or the inter-measurement timing.
	
	VL53L1X_SetDistanceMode(DEV, x);
	The VL53L1X supports 2 distance modes: short ( 1.3m ) x = 1 , long ( 4.0m ) x = 2
	
	VL53L1X_SetTimingBudgetInMs(DEV, x);
	The VL53L1X supports limiting the time require by the sensor to perform one range measurement
	in milliseconds. x = [20ms, 1000ms].

	VL53L1X_SetInterMeasurementInMs(DEV, x);
	The time the VL53L1X would be in a low-power state between measurements in milliseconds
*/
void VL53L1X_RangeInit(void)
{
	uint32_t status;
    uint16_t temp;
    //status = VL53L1X_SetTimingBudgetInMs(DEV, 500);
	status = VL53L1X_StartRanging(DEV);
	if (status != 0)
	{
		sprintf(printf_buffer, "[VL53L1X_Setup]: StartRanging failed with status: %d\n", status);
		UART_printf(printf_buffer);
	}
   VL53L1X_GetTimingBudgetInMs(DEV, &temp);
   sprintf(printf_buffer, "[VL53L1X_Setup]: Timing Budget: %d\n", temp);
   UART_printf(printf_buffer);
   
   VL53L1X_GetInterMeasurementInMs(DEV, &temp);
   sprintf(printf_buffer, "[VL53L1X_Setup]: Timing InterMeasurement: %d\n", temp);
   UART_printf(printf_buffer);
   
   
}


uint16_t VL53L1X_Distance(void)
{
	uint8_t dataReady = 0;
	uint16_t distance;
	uint16_t status;
	while (dataReady == 0)
	{
		status = VL53L1X_CheckForDataReady(DEV, &dataReady);
		VL53L1_WaitMs(DEV, 5);
	}
	
	status = VL53L1X_GetDistance(DEV, &distance);
    if (status == 0){};
	return distance;
}


void setupGPIO(void)
{
    // External LEDs: PN0 and PL4
    //PORT L [LED_EXTERNAL]
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R10;
    while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R10) == 0){}
    GPIO_PORTL_DEN_R  |= 0b10000;
    GPIO_PORTL_DIR_R  |= 0b10000;
    GPIO_PORTL_DATA_R |= 0b00000;
    
    //PORT N [LED_INTERNAL]
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;
    while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R12) == 0){}
    GPIO_PORTN_DEN_R  |= 0b0001;
    GPIO_PORTN_DIR_R  |= 0b0001;
    GPIO_PORTN_DATA_R |= 0b0000;
    
    // PORT M[0:3] [MOTOR]
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;
    while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R11) == 0){}
    GPIO_PORTM_DEN_R  |= 0b1111;
    GPIO_PORTM_DIR_R  |= 0b1111;
    
    //PORT J [INPUTS]
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R8;
    while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R8) == 0){}
    GPIO_PORTJ_DEN_R = 0b0001;
    GPIO_PORTJ_PUR_R = 0b0001;
    GPIO_PORTJ_DIR_R  = 0b0000;

    // PORTG [VL53L1X XSHUT]
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;
    while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R6) == 0) {};
    GPIO_PORTG_DIR_R   &=  0x00;
    GPIO_PORTG_AFSEL_R &= ~0x01;
    GPIO_PORTG_DEN_R   |=  0x01; 
}

void toggleLED_External(void)
{
    GPIO_PORTL_DATA_R ^= 0b10000;
}

void toggleLED_OnBoard(void)
{
    GPIO_PORTN_DATA_R ^= 0b0001;
}


// Step counter clockwise
void StepperMotor_StepForward(void)
{
    GPIO_PORTM_DATA_R = 0b0011;
    delayMilliseconds(5);
    GPIO_PORTM_DATA_R = 0b1001;
    delayMilliseconds(5);
    GPIO_PORTM_DATA_R = 0b1100;
    delayMilliseconds(5);
    GPIO_PORTM_DATA_R = 0b0110;
    delayMilliseconds(5);
}

// Step counter clockwise
void StepperMotor_StepBackward(void)
{
    GPIO_PORTM_DATA_R = 0b0110;
    delayMilliseconds(5);
    GPIO_PORTM_DATA_R = 0b1100;
    delayMilliseconds(5);
    GPIO_PORTM_DATA_R = 0b1001;
    delayMilliseconds(5);
    GPIO_PORTM_DATA_R = 0b0011;
    delayMilliseconds(5);
}


void StepperMotor_Sleep(void)
{
    GPIO_PORTM_DATA_R = 0b0000;
    delayMilliseconds(5);
}


// Use to put VL53L1X into hardware standby
// Used when using multiple I2C sensors
void VL53L1X_XSHUT(void){
    GPIO_PORTG_DIR_R |= 0x01;
    GPIO_PORTG_DATA_R  &= 0b11111110;
    SysTick_Wait10ms(10);
    GPIO_PORTG_DIR_R &= ~0x01;
}
