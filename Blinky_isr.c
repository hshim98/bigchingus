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
#define CCR_1MS_RELOAD (16000000L/1000L)

volatile unsigned int msec_cnt0 = 0;
volatile unsigned int CoinFound = 0;
volatile unsigned char pwm_count = 0;
volatile long unsigned int counter = 0;
volatile unsigned char state = 0;
volatile unsigned char Perim_found = 0;

void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void) // Timer0 A0 ISR. Only used by CC0
{
   TA0CCR0 += CCR_halfMS_RELOAD; // Add Offset to CCR0. this refills the timer so interrupt triggers   
	
	pwm_count++;
	if(pwm_count>80)
		 pwm_count=0;
		 
		
	if (CoinFound){
	
	counter++;
    if(counter>4000){
		counter=0;
		state++;
		if(state==8){
			CoinFound = 0;
			state=0;
		}
	}
	
		 
	switch(state){
		case 0:
			
			if(pwm_count>7)P2OUT |= 0x20; // make it one
			else P2OUT &= 0b11011111; // 13 choppy down Pivots about horizontal-axes, 180 deg downwards
			P2OUT |= 0x02;
			break;
		case 1:
			if(pwm_count>7)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111; // 15 Pivots about z-axes, 90 deg clockwise
			P2OUT |= 0x02;
			break;
		case 2:
			if(pwm_count>3)P2OUT |= 0x20; // make it one
			else P2OUT &= 0b11011111; // 13 Pivots about horizontal-axes, 180 deg upwards
			P2OUT &= 0b11111101;
			break;
		case 3:
			if(pwm_count>5)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111; // 13 Pivots about horizontal-axes, 180 deg upwards
			P2OUT &= 0b11111101;
			break;
		case 4:
			if(pwm_count>3)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111; // 13 Pivots about horizontal-axes, 180 deg upwards
			P2OUT &= 0b11111101;
			break;
		case 5:
			if(pwm_count>2)P2OUT |= 0x20; // make it one
			else P2OUT &= 0b11011111; // 15 Pivots about z-axes, 180 deg clockwise
			P2OUT &= 0b11111101;
			break;
		case 6:
			if(pwm_count>4)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111; // 13 Pivots about horizontal-axes, ~45 deg downwards
			P2OUT &= 0b11111101;
			break;
		case 7:
			if(pwm_count>1)P1OUT |= 0x80; // make it one
			else P1OUT &= 0b01111111; // 13 Pivots about horizontal-axes, 180 deg upwards
			P2OUT |= 0x02;
			break;
		default : break;
		}
		
		}
	else if(Perim_found){
		counter++;
    	if(counter>3100){
			counter=0;
			state++;
			if(state==2){
				Perim_found = 0;
				//state=2;
				P2OUT = 0x02;
				P1OUT = 0x00;
		}
	}
		
		switch(state){
		case 0:	
			P2OUT = 0x02;
			P1OUT = 0x00;		
			P1OUT |= 0x10;
			P2OUT |= 0x01;
			break;
		case 1:
			P2OUT = 0x02;
			P1OUT = 0x00;
			P2OUT |= 0x01;
			P1OUT |= 0x01;
			break;
			default: break;
		}		
	}  
}

#define PIN_PERIOD ( P2IN & BIT3 )  		// THIS MEANS WE WILL READ FROM PIN 2.3
long int GetPeriod (int n)
{
	int i, overflow;
	unsigned int saved_TCNT1a, saved_TCNT1b;
	
	overflow=0;
	TA0CTL&=0xfffe; // Clear the overflow flag
	while (PIN_PERIOD!=0) // Wait for square wave to be 0
	{
		if(TA0CTL&1) { TA0CTL&=0xfffe; overflow++; if(overflow>5) return 0;}
	}
	overflow=0;
	TA0CTL&=0xfffe; // Clear the overflow flag
	while (PIN_PERIOD==0) // Wait for square wave to be 1
	{
		if(TA0CTL&1) { TA0CTL&=0xfffe; overflow++; if(overflow>5) return 0;}
	}
	
	overflow=0;
	TA0CTL&=0xfffe; // Clear the overflow flag
	saved_TCNT1a=TA0R;
	for(i=0; i<n; i++) // Measure the time of 'n' periods
	{
		while (PIN_PERIOD!=0) // Wait for square wave to be 0
		{
			if(TA0CTL&1) { TA0CTL&=0xfffe; overflow++; if(overflow>1024) return 0;}
		}
		while (PIN_PERIOD==0) // Wait for square wave to be 1
		{
			if(TA0CTL&1) { TA0CTL&=0xfffe; overflow++; if(overflow>1024) return 0;}
		}
	}
	saved_TCNT1b=TA0R;
	if(saved_TCNT1b<saved_TCNT1a) overflow--; // Added an extra overflow.  Get rid of it.

	return overflow*0x10000L+(saved_TCNT1b-saved_TCNT1a);
}

int main(void)
{
	int i = 0;
	
	
	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
	P1DIR |= 0x01; // Set P1.0 		PIN 2			LEFT WHEEL FORWARD
	P1DIR |= 0x08; // Set P1.3 		PIN 5			RIGHT WHEEL FORWARD
	P1DIR |= 0x10; // set P1.4 		PIN 6			LEFT WHEEL BACKWARD
	P2DIR |= 0x01; // set P2.0 		PIN 8			RIGHT WHEEL BACKWARD
	
	P2DIR |= 0x20; // set P2.5		PIN 13			// ARM PIN 1
	P1DIR |= 0x80; // set P1.7		PIN 15			// ARM PIN 2
	
	P2DIR |= 0x02; // set P2.1		PIN 9			// ELECTROMAGNET PIN
	
	
	P2DIR &= ~(BIT3); // set P2.3 AS AN INPUT  PIN 11		// PERIOD MEASUREMENT PIN
	P2OUT |= BIT3;    // Select pull-up
	P2REN |= BIT3;    // Enable pull-up
	
	P2DIR &= ~(BIT4); // set P2.4 AS AN INPUT PIN12			// PERIMETER DETECTION PIN
	P2OUT |= BIT4;    // Select pull-up
	P2REN |= BIT4;    // Enable pull-up
	
	
	
		
	P1OUT = 0x00;			// initialize all pins to 0 to not fook a mosfets
	P2OUT = 0x02;			// set this to 1 so magnet is off
	   
    if (CALBC1_16MHZ != 0xFF) // This checks if the system is recalibrated
    {
		BCSCTL1 = CALBC1_16MHZ; // SET THE DCO
	  	DCOCTL  = CALDCO_16MHZ;
	}

    TA0CCTL0 = CCIE; // CCR0 interrupt enabled
    TA0CCR0 = CCR_halfMS_RELOAD;				// CHANGE AS NECESSARY FOR INTERRUPT TIMER
    TA0CTL = TASSEL_2 + MC_2;  // SMCLK, contmode 
   // _BIS_SR(GIE); // Enter LPM0 w/ interrupt  THIS ALLOWS INTERRUPTS
    _DINT();
 	while (1){
 	// neutral state: CoinFound = 0 so we just going forward
 	// CheckForCoin Function will check frequency of square wave from inductor to verify if coin
 	// CheckForPerimeter will be looking if perimeter, if there is we adjust pwm to turn the cart
 	
 	//if(pinforcoinfound==1)state =0, counter=0,coinfound=1
 	//for frequency, disable timer 1 at beginning
 	//find the frequency
 	//reload timer
 	//turn it on
 	//wait for it to overflow
 	//
 	
 	if(Perim_found){			// CHANGE THIS TO A NOT
 	//	if((pinregister&0b(correctpin))==0b(correctpin)){
 		
 		if (i==0){
 			i=1;
 			P1OUT = 0x00;				
   	  		P2OUT = 0x02;
 			state=0;
 			counter=0;
 			Perim_found=1;
 			TA0CCR0 = CCR_halfMS_RELOAD;
 			_EINT();
 			//reset timer to quarter ms
 		}
 	}
 	else if(!CoinFound){
 		_DINT();
 		//checkperiod
 		//if(period>somenum){
 		  if(i == 0){
 		  	i = 1;
 			P1OUT = 0x00;			
   	  		P2OUT = 0x02;
 			state=0;
 			counter=0;
 			CoinFound=1;
 			TA0CCR0 = CCR_halfMS_RELOAD;
 			_EINT(); 			
 		}
 	}
 	else if(!Perim_found&&!CoinFound){
 	  _DINT();
 	  P1OUT |= 0x01;				// conditional to set right wheel and left wheel forward
   	  P1OUT |= 0x08;
 	}
 	
 	
  };
}
