//  square.c: Uses timer 2 interrupt to generate a square wave in pin
//  P2.0 and a 75% duty cycle wave in pin P2.1
//  Copyright (c) 2010-2018 Jesus Calvino-Fraga
//  ~C51~

#include <stdio.h>
#include <stdlib.h>
#include <EFM8LB1.h>

// ~C51~  

#define SYSCLK 72000000L
#define BAUDRATE 115200L

#define OUT0 P2_1
#define OUT1 P2_2
#define OUT2 P2_3
#define OUT3 P2_4
#define ARM_P  P2_5
#define ARM_S  P2_2

volatile unsigned char pwm_count=0;
volatile unsigned char state=0;
volatile long unsigned int counter=0;

char _c51_external_startup (void)
{
	// Disable Watchdog with key sequence
	SFRPAGE = 0x00;
	WDTCN = 0xDE; //First key
	WDTCN = 0xAD; //Second key
  
	VDM0CN=0x80;       // enable VDD monitor
	RSTSRC=0x02|0x04;  // Enable reset on missing clock detector and VDD

	#if (SYSCLK == 48000000L)	
		SFRPAGE = 0x10;
		PFE0CN  = 0x10; // SYSCLK < 50 MHz.
		SFRPAGE = 0x00;
	#elif (SYSCLK == 72000000L)
		SFRPAGE = 0x10;
		PFE0CN  = 0x20; // SYSCLK < 75 MHz.
		SFRPAGE = 0x00;
	#endif
	
	#if (SYSCLK == 12250000L)
		CLKSEL = 0x10;
		CLKSEL = 0x10;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 24500000L)
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 48000000L)	
		// Before setting clock to 48 MHz, must transition to 24.5 MHz first
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
		CLKSEL = 0x07;
		CLKSEL = 0x07;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 72000000L)
		// Before setting clock to 72 MHz, must transition to 24.5 MHz first
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
		CLKSEL = 0x03;
		CLKSEL = 0x03;
		while ((CLKSEL & 0x80) == 0);
	#else
		#error SYSCLK must be either 12250000L, 24500000L, 48000000L, or 72000000L
	#endif
	
	P0MDOUT |= 0x10; // Enable UART0 TX as push-pull output
	P2MDIN   = 0b0000_0000;
	P2MDOUT  = 0b0000_0000;
	XBR0     = 0x01; // Enable UART0 on P0.4(TX) and P0.5(RX)                     
	XBR1     = 0X00;
	XBR2     = 0x40; // Enable crossbar and weak pull-ups

	// Configure Uart 0
	#if (((SYSCLK/BAUDRATE)/(2L*12L))>0xFFL)
		#error Timer 0 reload value is incorrect because (SYSCLK/BAUDRATE)/(2L*12L) > 0xFF
	#endif
	SCON0 = 0x10;
	TH1 = 0x100-((SYSCLK/BAUDRATE)/(2L*12L));
	TL1 = TH1;      // Init Timer1
	TMOD &= ~0xf0;  // TMOD: timer 1 in 8-bit auto-reload
	TMOD |=  0x20;                       
	TR1 = 1; // START Timer1
	TI = 1;  // Indicate TX0 ready

	// Initialize timer 2 for periodic interrupts
	TMR2CN0=0x00;   // Stop Timer2; Clear TF2;
	CKCON0|=0b_0001_0000; // Timer 2 uses the system clock
	TMR2RL=(65535L-0.0001*SYSCLK); // Initialize reload value
	TMR2=0xffff;   // Set to reload immediately
	ET2=1;         // Enable Timer2 interrupts
	TR2=1;         // Start Timer2 (TMR2CN is bit addressable)

	EA=1; // Enable interrupts

  	
	return 0;
}

void Timer2_ISR (void) interrupt 5
{
	TF2H = 0; // Clear Timer2 interrupt flag
	counter++;
	if(counter>10000){
		counter=0;
		state++;
		if(state==7)state=0;
	}
	pwm_count++;
	/* 
	pwm_count>20 == 2.0ms pulse
	pwm_count>12 == 1.2ms pulse
	pwm_count>5  == 0.5ms pulse*/
	if(pwm_count>200) pwm_count=0;
	switch(state){
		case 0:
			ARM_P = pwm_count>20?1:0; // Pivots about z-axes, 180 deg counterclockwise
			break;
		case 1:
			ARM_S = pwm_count>20?1:0; // Pivots about horizontal-axes, 180 deg downwards
			break;
		case 2:
			ARM_P = pwm_count>9?1:0; // Pivots about z-axes, 90 deg clockwise
			break;
		case 3:
			ARM_S = pwm_count>5?1:0; // Pivots about horizontal-axes, 180 deg upwards
			break;
		case 4:
			ARM_P = pwm_count>6?1:0; // Pivots about z-axes, 180 deg clockwise
			break;
		case 5:
			ARM_S = pwm_count>12?1:0; // Pivots about horizontal-axes, ~45 deg downwards
			break;
		case 6:
			ARM_S = pwm_count>5?1:0; // Pivots about horizontal-axes, 180 deg upwards
			break;
		default : ARM_S = pwm_count>5?1:0;
				  ARM_P = pwm_count>5?1:0; // Pivots about z-axes, 180 deg clockwise
		}
	/*if(state==0)
	ARM_S = pwm_count>20?1:0;//2ms fully extended
						   
	else if(state==1)
	ARM_S = pwm_count>5?1:0;//0.5 ms fully contracted*/
	


}

void main (void)
{
	printf("\x1b[2J"); // Clear screen using ANSI escape sequence.
	printf("Square wave generator for the EFM8LB1.\r\n"
	       "Check pins P2.0 and P2.1 with the oscilloscope.\r\n");
	while(1)
	{
	}
}
