/*
 * CS101_Eyanthra.c
 *
 * Created: 4/15/2015 8:04:27 AM
 *  Author: RaviTeja Gundeti 
 */ 

#define F_CPU 14745600

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>



#include <math.h>
#include "lcd.h"


unsigned char data;     //to store received data from UDR1


// initializing ports for motion 
void motion_pin_config (void)
	{
	DDRA = DDRA | 0x0F;       //set direction of the PORTA 3 to PORTA 0 pins as output
	PORTA = PORTA & 0xF0;     // set initial value of the PORTA 3 to PORTA 0 pins to logic 0
	DDRL = DDRL | 0x18;       //Setting PL3 and PL4 pins as output for PWM generation
	PORTL = PORTL | 0x18;     //PL3 and PL4 pins are for velocity control using PWM
	}
//ended initialization of ports for motion


//initializing ports for servo motor motion for servo motors 1 and 2 
void servo1_pin_config (void)
	{
	DDRB  = DDRB | 0x20;  //making PORTB 5 pin output
	PORTB = PORTB | 0x20; //setting PORTB 5 pin to logic 1
	}

void servo2_pin_config (void)
	{
	DDRB  = DDRB | 0x40;  //making PORTB 6 pin output
	PORTB = PORTB | 0x40; //setting PORTB 6 pin to logic 1
	}
//ended initialization ports for motion of servo  motors 1 and 2



//initializing ports for LCD
void lcd_port_config (void)
	{
	DDRC = DDRC | 0xF7;
	PORTC = PORTC & 0x80;
	}
//ended initialization of ports for LCD

//initializing ports for ADC(ADC pin configuration)
 /*void adc_pin_config (void)
	{
	 DDRF = 0x00;
	 PORTF = 0x00;
	 DDRK = 0x00;
	 PORTK = 0x00;
	}*/
 //ending it
 
 



//function to initialize all ports 
void port_init()
	{
		motion_pin_config();
		servo1_pin_config();
		servo2_pin_config();
		lcd_port_config();
		//adc_pin_config();
	}
//ending the function



//Function To Initialize UART0
// desired baud rate:9600
// actual baud rate:9600 (error 0.0%)
// char size: 8 bit
// parity: Disabled
void uart0_init(void)
	{
	UCSR0B = 0x00; //disable while setting baud rate
	UCSR0A = 0x00;
	UCSR0C = 0x06;
	// UBRR0L = 0x47; //11059200 Hz
	UBRR0L = 0x5F; // 14745600 Hzset baud rate lo
	UBRR0H = 0x00; //set baud rate hi
	UCSR0B = 0x98;
	}
//ending the function


//function for timer to control motion of servo motor
void timer1_init(void)
	{
	TCCR1B = 0x00; //stop
	TCNT1H = 0xFC; //Counter high value to which OCR1xH value is to be compared with
	TCNT1L = 0x01;	//Counter low value to which OCR1xH value is to be compared with
	OCR1AH = 0x03;	//Output compare Register high value for servo 1
	OCR1AL = 0xFF;	//Output Compare Register low Value For servo 1
	OCR1BH = 0x03;	//Output compare Register high value for servo 2
	OCR1BL = 0xFF;	//Output Compare Register low Value For servo 2
	OCR1CH = 0x03;	//Output compare Register high value for servo 3
	OCR1CL = 0xFF;	//Output Compare Register low Value For servo 3
	ICR1H  = 0x03;	
	ICR1L  = 0xFF;
	TCCR1A = 0xAB; /*{COM1A1=1, COM1A0=0; COM1B1=1, COM1B0=0; COM1C1=1 COM1C0=0}
 					For Overriding normal port functionality to OCRnA outputs.
				  {WGM11=1, WGM10=1} Along With WGM12 in TCCR1B for Selecting FAST PWM Mode*/
	TCCR1C = 0x00;
	TCCR1B = 0x0C; //WGM12=1; CS12=1, CS11=0, CS10=0 (Prescaler=256)
	}
//ending it


//Function to Initialize ADC
/*void adc_init()
	{
	ADCSRA = 0x00;
	ADCSRB = 0x00;
	ADMUX = 0x20;
	ACSR = 0x80;
	ADCSRA = 0x86;
	}*/
//ending it


//This Function accepts the Channel Number and returns the corresponding Analog Value
/*unsigned char ADC_Conversion(unsigned char Ch)
	{
	unsigned char a;
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}
	Ch = Ch & 0x07;
	ADMUX= 0x20| Ch;
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for ADC conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	ADCSRB = 0x00;
	return a;
	}*/
//ending it

//function to print the sensor value
/*void print_sensor(char row, char coloumn,unsigned char channel)
	{
	unsigned char ADC_Value;
	ADC_Value = ADC_Conversion(channel);
	lcd_print(row, coloumn, ADC_Value, 3);
	}*/
//ending it

//function to convert the sharp sensor reading on a scale to 800
/*unsigned int Sharp_GP2D12_estimation(unsigned char adc_reading)
	{
	float distance;
	unsigned int distanceInt;
	distance = (int)(10.00*(2799.6*(1.00/(pow(adc_reading,1.1546)))));
	distanceInt = (int)distance;
	if(distanceInt>800)
	{
		distanceInt=800;
	}
	return distanceInt;
	}*/
//ending it




//function to initialize all devices
void init_devices()
	{
	cli(); //Clears the global interrupts
	
	timer1_init();
	port_init();  //Initializes all the ports
	uart0_init(); //Initailize UART1 for serial communication
	//adc_init();
	sei();   //Enables the global interrupts
	}
//ending it



//function for motion of servo motor for 1 and 2
void servo_1(unsigned char degrees)
	{
	float PositionPanServo = 0;
	PositionPanServo = ((float)degrees / 1.86) + 35.0;
	OCR1AH = 0x00;
	OCR1AL = (unsigned char) PositionPanServo;
	}

void servo_2(unsigned char degrees)
	{
	float PositionTiltServo = 0;
	PositionTiltServo = ((float)degrees / 1.86) + 35.0;
	OCR1BH = 0x00;
	OCR1BL = (unsigned char) PositionTiltServo;
	}
//ending it





//////////////////////////////////////////////////////////////////////////
///function to act according to input received through X_Bee.

SIGNAL(SIG_USART0_RECV) 		// ISR for receive complete interrupt
	{
		data = UDR0; 				//making copy of data from UDR0 in 'data' variable

		UDR0 = data; 				//echo data back to PC
		
		
		//for forward motion
		if(data == 0x38) //ASCII value of 8
		{
			PORTA=0x06;  //forward
			lcd_cursor(1,1);
			lcd_string("moving__forward");
		}
		//ended
		
		
		//for backward motion
		if(data == 0x32) //ASCII value of 2
		{
			PORTA=0x09; //back
			lcd_cursor(1,1);
			lcd_string("moving_backward");
		}
		//ended


		//for left motion	
		if(data == 0x34) //ASCII value of 4
		{
			PORTA=0x05;  //left
			lcd_cursor(1,1);
			lcd_string("_turning___left");
		}
		//ended
		
		
		//for right motion
		if(data == 0x36) //ASCII value of 6
		{
			PORTA=0x0A; //right
			lcd_cursor(1,1);
			lcd_string("_turning__right");
		}
		//ended
		
		
		//for stop motion
		if(data==0x30) // ASCII value of 0
		{
			PORTA=0x00;
			lcd_cursor(1,1);
			lcd_string("__stopping__bot");
		}
		//ended
		
		
		//for motion of servo motor 1(wedge like structure) for moving down
		if(data==0x31) // ASCII value of 1
		{	
			unsigned char i=0;
			for (i = 0; i <3; i++)
			{
				servo_1(i);
				_delay_ms(30);
			
			}
			lcd_cursor(1,1);
			lcd_string("moving__gripper");
		}
		//ended
		
		
		//for motion of servo motor1(wedge like structure) for moving up
		if(data==0x37) //ASCII value of 7
		{ 
			unsigned char i=0;
			for (i = 0; i <50; i++)
			{
				servo_1(i);
				_delay_ms(30);
			
			}
			lcd_cursor(1,1);
			lcd_string("moving__gripper");
		}
		//ended
		
		
	//for motion of servo motor 2(gripper arm) at various angles
		//for catching small objects 
		if(data==0x39)
		{
			unsigned char i=0;
			for (i = 0; i <70; i++)
			{
				servo_2(i);
				_delay_ms(30);
				
			}
			lcd_cursor(1,1);
			lcd_string("moving__gripper");
		}
	   //ended
	   
		//for stretching the arms apart
		if(data==0x33) // ASCII value of 3
		{
			unsigned char i=0;
			for (i = 0; i <3; i++)
			{
				servo_2(i);
				 _delay_ms(30);
		   
			}
			lcd_cursor(1,1);
			lcd_string("moving__gripper");
		}
		//ended
	  
		//additional feature to hold objects of medium size
		if(data==0x71) // ASCII value of q
		{	
			unsigned char i=0;
			for (i = 0; i <30; i++)
			{
				servo_2(i);
				_delay_ms(30);
		   
			}
			lcd_cursor(1,1);
			lcd_string("moving__gripper");
		}
	  
		if(data==0x77) // ASCII value of w
		{	
			unsigned char i=0;
			for (i = 0; i <40; i++)
			{
				 servo_2(i);
				_delay_ms(30);
		   
			}
			lcd_cursor(1,1);
			lcd_string("moving__gripper");			
		}
	  
		if(data==0x61) // ASCII value of a
		{	
			unsigned char i=0;
			for (i = 0; i <50; i++)
			{
				servo_2(i);
				_delay_ms(30);
		   
			}
			lcd_cursor(1,1);
			lcd_string("moving__gripper");
		}
	    
		if(data==0x73) //ASCII value of s
		{
			unsigned char i=0;
			for (i = 0; i <60; i++)
			{
				 servo_2(i);
				_delay_ms(30);
		   
			 }
			 lcd_cursor(1,1);
			 lcd_string("moving__gripper");
		}
		//ended
	
	
	}
////////////////////////////////////////////////////////////////////
//ended the function to act accordingly to the input.

	
//Main Function
int main(void)
{
	init_devices();
	lcd_set_4bit();
	lcd_init();
	while(1);
}
















