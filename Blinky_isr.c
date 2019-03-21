//  Nick's group master code for coin picker upper robot

//      P1OUT ^= 0x01; // Toggle P1.0    LEFT WHEEL FORWARD
//      P1OUT ^= 0x08; // Toggle P1.3	 RIGHT WHEEL FORWARD
//      P1OUT ^= 0x10; // Toggle P1.4    LEFT WHEEL BACKWARD
//      P2OUT ^= 0x01; // Toggle P2.0	 RIGHT WHEEL BACKWARD

// PIN 13: SWINGY SWINGY 	P2.5
// PIN 15: CHOPPY CHOPPY	P1.7

#include <msp430.h>			
#include <stdio.h>

#define CCR_halfMS_RELOAD (16000000L/4000L)		// This will interrupt every 0.25ms

volatile unsigned int msec_cnt0 = 0;
volatile unsigned int CoinFound = 1;
volatile unsigned char pwm_count = 0;
volatile long unsigned int counter = 0;
volatile unsigned char state = 0;

void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void) // Timer0 A0 ISR. Only used by CC0
{
   TA0CCR0 += CCR_halfMS_RELOAD; // Add Offset to CCR0. this refills the timer so interrupt triggers   
   
   counter++;
   if(counter>4000){
		counter=0;
		state++;
		if(state==7){
			CoinFound = 0;
			state=0;
		}
	}
	
	pwm_count++;
	if(pwm_count>80)
		 pwm_count=0;
		 
		
	if (CoinFound){
	
		 
	switch(state){
		case 0:
			
			if(pwm_count>7)P2OUT |= 0x20; // make it one
			else P2OUT &= 0b11011111;//make it zeroARM_S = pwm_count>4?1:0; // 13 choppy down Pivots about horizontal-axes, 180 deg downwards
			break;
		case 1:
			if(pwm_count>7)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111;//make it zeroARM_P = pwm_count>2?1:0; // 15 Pivots about z-axes, 90 deg clockwise
			break;
		case 2:
			if(pwm_count>3)P2OUT |= 0x20; // make it one
			else P2OUT &= 0b11011111;//make it zeroARM_S = pwm_count>1?1:0; // 13 Pivots about horizontal-axes, 180 deg upwards
			break;
		case 3:
			if(pwm_count>2)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111;//make it zeroARM_S = pwm_count>1?1:0; // 13 Pivots about horizontal-axes, 180 deg upwards
			break;
		case 4:
			if(pwm_count>2)P2OUT |= 0x20; // make it one
			else P2OUT &= 0b11011111;//make it zeroARM_P = pwm_count>1?1:0; // 15 Pivots about z-axes, 180 deg clockwise
			break;
		case 5:
			if(pwm_count>4)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111;//make it zeroARM_S = pwm_count>3?1:0; // 13 Pivots about horizontal-axes, ~45 deg downwards
			break;
		case 6:
			if(pwm_count>1)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111;//make it zeroARM_S = pwm_count>1?1:0; // 13 Pivots about horizontal-axes, 180 deg upwards
			break;
		default : break;
		}
		
		}
	else {
   	  P2OUT |= 0x20;				
   	  P1OUT |= 0x80;
   	 // P1OUT |= 0x01;				// conditional to set right wheel and left wheel forward
   	  P1OUT |= 0x08;
   }
   
   
   
   
   
   
   
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
	P1DIR |= 0x01; // Set P1.0 		PIN 2
	P1DIR |= 0x08; // Set P1.3 		PIN 5
	P1DIR |= 0x10; // set P1.4 		PIN 6
	P2DIR |= 0x01; // set P2.0 		PIN 8
	
	P2DIR |= 0x20; // set P2.5		PIN 13
	P1DIR |= 0x80; // set P1.7		PIN 15
		
	P1OUT = 0x00;			// initialize all pins to 0 to not fook a mosfets
	P2OUT = 0x00;
	   
    if (CALBC1_16MHZ != 0xFF) // This checks if the system is recalibrated
    {
		BCSCTL1 = CALBC1_16MHZ; // SET THE DCO
	  	DCOCTL  = CALDCO_16MHZ;
	}

    TA0CCTL0 = CCIE; // CCR0 interrupt enabled
    TA0CCR0 = CCR_halfMS_RELOAD;
    TA0CTL = TASSEL_2 + MC_2;  // SMCLK, contmode 
    _BIS_SR(GIE); // Enter LPM0 w/ interrupt  THIS ALLOWS INTERRUPTS
 	while (1){
 	// neutral state: CoinFound = 0 so we just going forward
 	// CheckForCoin Function will check frequency of square wave from inductor to verify if coin
 	// CheckForPerimeter will be looking if perimeter, if there is we adjust pwm to turn the cart
 	;
 	}
}
