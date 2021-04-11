#include "I2C.h"
#include <stdint.h>
#include "definitions.h"

void I2C_Init()
{
    SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;   // activate I2C0
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1; // activate port B
    while ((SYSCTL_PRGPIO_R & 0x0002) == 0) {}; // ready?

    GPIO_PORTB_AFSEL_R |= 0x0C; // 3) enable alt funct on PB2,3       0b00001100
    GPIO_PORTB_ODR_R |= 0x08;   // 4) enable open drain on PB3 only

    GPIO_PORTB_DEN_R |= 0x0C; // 5) enable digital I/O on PB2,3
    //    GPIO_PORTB_AMSEL_R &= ~0x0C;          // 7) disable analog functionality on PB2,3

    // 6) configure PB2,3 as I2C

    //  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00003300;
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFF00FF) + 0x00002200; //TED
    I2C0_MCR_R = I2C_MCR_MFE;                                          // 9) master function enable
    I2C0_MTPR_R = 0b0000000000000101000000000111011;                   // 8) configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
    //    I2C0_MTPR_R = 0x3B;                                        // 8) configure for 100 kbps clock

    // 20*(TPR+1)*20ns = 10us, with TPR=24
    // TED 100 KHz
    //     CLK_PRD = 8.3ns
    //    TIMER_PRD = 1
    //    SCL_LP = 6
    //    SCL_HP = 4
    //    10us = 2 * (1 + TIMER_PRD) * (SCL_LP + SCL_HP) * CLK_PRD
    //    10us = 2 * (1+TIMER+PRD) * 10 * 8.3ns
    //  TIMER_PRD = 59 (0x3B)
    //
    // TIMER_PRD is a 6-bit value.  This 0-127
    //    @0: 2 * (1+ 0) * 10 * 8.3ns --> .1667us or 6.0MHz
    //  @127: 2 * (1+ 127) * 10 * 8.3ns --> 47kHz
}
