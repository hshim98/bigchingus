//  Nick's group master code for coin picker upper robot

//      P1OUT BIT0; // Toggle P1.0    LEFT WHEEL FORWARD
//      P1OUT BIT3; // Toggle P1.3	 RIGHT WHEEL FORWARD
//      P1OUT BIT4; // Toggle P1.4    LEFT WHEEL BACKWARD
//      P2OUT BIT0; // Toggle P2.0	 RIGHT WHEEL BACKWARD

// PIN 13: SWINGY SWINGY 	P2.5
// PIN 15: CHOPPY CHOPPY	P1.7

#include <msp430.h>			
#include <stdio.h>

#define CCR_halfMS_RELOAD (16000000L/4000L)		// This will interrupt every 0.25ms
#define CCR_1MS_RELOAD (16000000L/1000L)
#define CLK 16000000L

#define PIN_PERIOD ( P2IN & BIT3 )  		// THIS MEANS WE WILL READ FROM PIN 2.3
#define PERIMETER_PIN_ONE ( P2IN & BIT4 )
#define PERIMETER_PIN_TWO ( P1IN & BIT6 )
#define RXD BIT1 // Receive Data (RXD) at P1.1
#define TXD BIT2 // Transmit Data (TXD) at P1.2
#define BAUD 115200

volatile unsigned int msec_cnt0 = 0;
volatile unsigned int CoinFound = 0;
volatile unsigned char pwm_count = 0;
volatile long unsigned int counter = 0;
volatile unsigned char state = 0;
volatile unsigned char Perim_found = 0;
volatile unsigned char coincount =0;
volatile unsigned char done = 0;

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
		if(state==9){
			CoinFound = 0;
			coincount++;
			if (coincount == 20){
				P1OUT &= ~BIT3;
				P1OUT &= ~BIT0;		
				P1OUT &= ~BIT4;				// turn off all of the wheels to be safe
				P2OUT &= ~BIT0;
				done = 1;
			}
			state=0;
		}
	}
	
		 
	switch(state){
		case 0:	
			if (counter < 500){
			P1OUT &= ~BIT3;
			P1OUT &= ~BIT0;		
			P1OUT |= BIT4;				// drive backwards for a certain amount of time
			P2OUT |= BIT0;
			}
			else {
			P1OUT &= ~BIT3;
			P1OUT &= ~BIT0;		
			P1OUT &= ~BIT4;				// drive backwards for a certain amount of time
			P2OUT &= ~BIT0;
			}
			P2OUT |= BIT1;
			break;
		case 1:			
			if(pwm_count>7)P2OUT |= BIT5; // make it one
			else P2OUT &= ~BIT5; // 13 choppy down Pivots about horizontal-axes, 180 deg downwards
			P2OUT |= BIT1;
			break;
		case 2:
			if(pwm_count>7)P2OUT |= BIT4; // make it one
			else P2OUT &= ~BIT4; // 15 Pivots about z-axes, 90 deg clockwise
			P2OUT |= BIT1;
			break;
		case 3:
			if(pwm_count>3)P2OUT |= BIT5; // make it one
			else P2OUT &= ~BIT5; // 13 Pivots about horizontal-axes, 180 deg upwards
			P2OUT &= ~BIT1;
			break;
		case 4:
			if(pwm_count>5)P2OUT |= BIT4; // make it one
			else P2OUT &= ~BIT4; // 13 Pivots about horizontal-axes, 180 deg upwards
			P2OUT &= ~BIT1;
			break;
		case 5:
			if(pwm_count>3)P2OUT |= BIT4; // make it one
			else P2OUT &= ~BIT4; // 13 Pivots about horizontal-axes, 180 deg upwards
			P2OUT &= ~BIT1;
			break;
		case 6:
			if(pwm_count>2)P2OUT |= BIT5; // make it one
			else P2OUT &= ~BIT5; // 15 Pivots about z-axes, 180 deg clockwise
			P2OUT &= ~BIT1;
			break;
		case 7:
			if(pwm_count>4)P2OUT |= BIT4; // make it one
			else P2OUT &= ~BIT4; // 13 Pivots about horizontal-axes, ~45 deg downwards
			P2OUT &= ~BIT1;
			break;
		case 8:
			if(pwm_count>1)P2OUT |= BIT4; // make it one
			else P2OUT &= ~BIT4; // 13 Pivots about horizontal-axes, 180 deg upwards
			P2OUT |= BIT1;
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
				P1OUT &= ~BIT3;
				P1OUT &= ~BIT0;		
				P1OUT &= ~BIT4;				// turn off all of the wheels to be safe
				P2OUT &= ~BIT0;
		}
	}		
		switch(state){
		case 0:	
			P1OUT &= ~BIT3;
			P1OUT &= ~BIT0;		
			P1OUT |= BIT4;				// drive backwards for a certain amount of time
			P2OUT |= BIT0;
			break;
		case 1:
			P1OUT &= ~BIT3;
			P1OUT &= ~BIT4;
			P2OUT |= BIT0;
			P1OUT |= BIT0;
			break;
			default: break;
		}		
	}  
}
void uart_init(void)
{
	P1SEL  |= (RXD | TXD);                       
  	P1SEL2 |= (RXD | TXD);                       
  	UCA0CTL1 |= UCSSEL_2; // SMCLK
  	UCA0BR0 = (CLK/BAUD)%0x100;
  	UCA0BR1 = (CLK/BAUD)/0x100;
  	UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
  	UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
}

unsigned char uart_getc()
{
    while (!(IFG2&UCA0RXIFG)); // USCI_A0 RX buffer ready?
	return UCA0RXBUF;
}

void uart_putc (char c)
{
	if(c=='\n')
	{
		while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
	  	UCA0TXBUF = '\r'; // TX
  	}
	while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
  	UCA0TXBUF = c; // TX
}

void uart_puts(const char *str)
{
     while(*str) uart_putc(*str++);
}

char HexDigit[]="0123456789ABCDEF";

void PrintNumber(long int val, int Base, int digits)
{ 
	int j;
	#define NBITS 32
	char buff[NBITS+1];
	buff[NBITS]=0;

	j=NBITS-1;
	while ( (val>0) | (digits>0) )
	{
		buff[j--]=HexDigit[val%Base];
		val/=Base;
		if(digits!=0) digits--;
	}
	uart_puts(&buff[j+1]);
}

// Use TA0 configured as a free running timer to delay 1 ms
void wait_1ms (void)
{
	unsigned int saved_TA0R;
	
	saved_TA0R=TA0R; // Save timer A0 free running count
	while ((TA0R-saved_TA0R)<(16000000L/1000L));
}

void waitms(int ms)
{
	while(--ms) wait_1ms();
}

long int GetPeriod (int n)
{
	int i, overflow;
	unsigned int saved_TCNT1a, saved_TCNT1b;
	
	TA0CCR0 = CCR_1MS_RELOAD;				// 1 MS RELOAD TIME
	
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
	float period;
	float Period_Threshold;
	int v1;
	int v2;
	
	
	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
	P1DIR |= 0x01; // Set P1.0 		PIN 2			LEFT WHEEL FORWARD
	P1DIR |= 0x08; // Set P1.3 		PIN 5			RIGHT WHEEL FORWARD   // ARM PIN 15 1.7 GOES TO PIN 12 P2.4
	P1DIR |= 0x10; // set P1.4 		PIN 6			LEFT WHEEL BACKWARD
	P2DIR |= 0x01; // set P2.0 		PIN 8			RIGHT WHEEL BACKWARD
	
	P2DIR |= 0x20; // set P2.5		PIN 13			// ARM PIN 1
					P2DIR |= 0x10; // set P2.4		PIN 12			// ARM PIN 2
	
	P2DIR |= 0x02; // set P2.1		PIN 9			// ELECTROMAGNET PIN
	
	
	P2DIR &= ~(BIT3); // set P2.3 AS AN INPUT  PIN 11		// PERIOD MEASUREMENT PIN
	P2OUT |= BIT3;    // Select pull-up
	P2REN |= BIT3;    // Enable pull-up
	
		//			P2DIR &= ~(BIT4); // set P2.4 AS AN INPUT  PIN 12			// PERIMETER DETECTION PIN	 
	
	P1OUT &= ~BIT0;			// initialize all DRIVING pins to 0 to not fook a mosfets
	P1OUT &= ~BIT3;			// set this to 1 so magnet is off
	P1OUT &= ~BIT4;
	P2OUT &= ~BIT0;
	
	P2OUT &= ~BIT5;			// initialize the arm pins to 0
	P2OUT &= ~BIT4;
	
	P2OUT |= BIT1;			// set the magnet pin to one to turn it off
	
	   
    if (CALBC1_16MHZ != 0xFF) // This checks if the system is recalibrated
    {
		BCSCTL1 = CALBC1_16MHZ; // SET THE DCO
	  	DCOCTL  = CALDCO_16MHZ;
	}

    TA0CCTL0 = CCIE; // CCR0 interrupt enabled
    TA0CCR0 = CCR_halfMS_RELOAD;				// CHANGE AS NECESSARY FOR INTERRUPT TIMER
    TA0CTL = TASSEL_2 + MC_2;  // SMCLK, contmode 
    
	//TRYING TO USE ADC LOL
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + REFON + ADC10ON; 
	
//	ADC10
	
	uart_init();
    _DINT();			// REMEMBER TO DISABLE
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
 	
 /*	uart_puts("Period measurement using the free running counter of timer TA0.\n"
	          "Connect signal to P2.3 (pin 11).\n");
	          while(1){
	          period=GetPeriod(100);
	          uart_puts("f=");
			  PrintNumber(PERIMETER_PIN_TWO, 10, 6);
	          }*/
 	
 	if(!Perim_found&&!CoinFound){			// IF PERIM NOT FOUND CHECK
 		_DINT();
 		ADC10CTL1 = INCH_6;				// PIN 1.6 IS PERIMETER DETECTION PIN
		ADC10AE0 |= 0x40;
 		ADC10CTL0 |= ENC + ADC10SC;    // Sampling and conversion start
		while (ADC10CTL1 & ADC10BUSY); // ADC10BUSY?
		v1=(ADC10MEM*3290L)/1023L;
		ADC10CTL1 = INCH_7;				// PIN 1.7 IS PERIMETER DETECTION PIN
		ADC10AE0 |= 0x80;
 		ADC10CTL0 |= ENC + ADC10SC;    // Sampling and conversion start
		while (ADC10CTL1 & ADC10BUSY);
		v2=(ADC10MEM*3290L)/1023L;
		
 		if(v1 > 500 || v2 > 500){			// CHECK PEAK DETECTOR PIN TO SEE IF PERIMETER FOUND
 			P1OUT &= ~BIT0;				
   	  		P1OUT &= ~BIT3;
   	  		P1OUT &= ~BIT4;			// turn off all the wheel just to be safe
	 		P2OUT &= ~BIT0;
 			state=0;
 			counter=0;
 			Perim_found=1;
 			TA0CCR0 = CCR_halfMS_RELOAD;
 			_EINT();
 		}
 	}
 	if(!CoinFound&&!Perim_found){		// IF COIN NOT FOUND CHECK		
 		_DINT();
 		period = (GetPeriod(100) / (CLK * 10000.0));				// threshold frequency 56.05KHz
 		PrintNumber((1.0/period), 10, 6);
 		uart_puts("\r");
 		if((1.0 / period) > 5815000){
 			P1OUT &= ~BIT0;			
   	  		P1OUT &= ~BIT3;
   	  		P1OUT &= ~BIT4;
	 		P2OUT &= ~BIT0;
 			state=0;
 			counter=0;
 			CoinFound=1;
 			TA0CCR0 = CCR_halfMS_RELOAD;
 			_EINT(); 			
 		}
 	}
 	if(!Perim_found&&!CoinFound){
 	  _DINT();
 	  P1OUT |= BIT0;				// conditional to set right wheel and left wheel forward
   	  P1OUT |= BIT3;
   	  P1OUT &= ~BIT4;			// turn off backward wheels to be safe
	  P2OUT &= ~BIT0;
 	}
 	if (done == 1){
 	P1OUT &= ~BIT0;			
   	P1OUT &= ~BIT3;
   	P1OUT &= ~BIT4;
	P2OUT &= ~BIT0;
 	break;
 	}
  }
}
