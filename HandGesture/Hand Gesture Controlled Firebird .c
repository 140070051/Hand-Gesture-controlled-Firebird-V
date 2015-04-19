/*
 * x_men5.c
 *
 * Created: 4/11/2015 1:51:16 AM
 *  Author: RaviTeja
 */ 


#define F_CPU 14745600
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>


#include <math.h>
#include "lcd.h"

unsigned char data; //to store received data from UDR1

void buzzer_pin_config (void)
{
	DDRC = DDRC | 0x08;		//Setting PORTC 3 as outpt
	PORTC = PORTC & 0xF7;		//Setting PORTC 3 logic low to turnoff buzzer
}

void motion_pin_config (void)
{
	DDRA = DDRA | 0x0F;
	PORTA = PORTA & 0xF0;
	DDRL = DDRL | 0x18;   //Setting PL3 and PL4 pins as output for PWM generation
	PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM.
}

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

void lcd_port_config (void)
{
	DDRC = DDRC | 0xF7;
	PORTC = PORTC & 0x80;
}

//ADC pin configuration
void adc_pin_config (void)
{
	DDRF = 0x00;
	PORTF = 0x00;
	DDRK = 0x00;
	PORTK = 0x00;
}

//Function to initialize ports
void port_init()
{
	motion_pin_config();
	buzzer_pin_config();
	servo1_pin_config();
	servo2_pin_config();
	lcd_port_config();
	adc_pin_config();
}

void buzzer_on (void)
{
	unsigned char port_restore = 0;
	port_restore = PINC;
	port_restore = port_restore | 0x08;
	PORTC = port_restore;
}

void buzzer_off (void)
{
	unsigned char port_restore = 0;
	port_restore = PINC;
	port_restore = port_restore & 0xF7;
	PORTC = port_restore;
}

//Function to Initialize ADC
void adc_init()
{
	ADCSRA = 0x00;
	ADCSRB = 0x00;
	ADMUX = 0x20;
	ACSR = 0x80;
	ADCSRA = 0x86;
}

//This Function accepts the Channel Number and returns the corresponding Analog Value
unsigned char ADC_Conversion(unsigned char Ch)
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
}




void print_sensor(char row, char coloumn,unsigned char channel)
{
	unsigned char ADC_Value;
	ADC_Value = ADC_Conversion(channel);
	lcd_print(row, coloumn, ADC_Value, 3);
}

 
 unsigned int Sharp_GP2D12_estimation(unsigned char adc_reading)
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
 }



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






void init_devices()
{
	cli(); //Clears the global interrupts
	timer1_init();
	adc_init();
	port_init();  //Initializes all the ports
	uart0_init(); //Initailize UART1 for serial communiaction
	sei();   //Enables the global interrupts
}


void servo_1(unsigned char degrees)
{
	float PositionPanServo = 0;
	PositionPanServo = ((float)degrees / 1.86) + 35.0;
	OCR1AH = 0x00;
	OCR1AL = (unsigned char) PositionPanServo;
}

void servo_1_5(unsigned char degrees)
{
	float PositionPanServo = 0;
	PositionPanServo = ((float)degrees / 1.86) + 35.0;
	OCR1AH = 0x00;
	OCR1AL = (unsigned char) PositionPanServo;
}

void servo_1_free (void) //makes servo 1 free rotating
{
	OCR1AH = 0x03;
	OCR1AL = 0xFF; //Servo 1 off
}



void servo_2(unsigned char degrees)
{
	float PositionTiltServo = 0;
	PositionTiltServo = ((float)degrees / 1.86) + 35.0;
	OCR1BH = 0x00;
	OCR1BL = (unsigned char) PositionTiltServo;
}

void servo_2_5(unsigned char degrees)
{
	float PositionTiltServo = 0;
	PositionTiltServo = ((float)degrees / 1.86) + 35.0;
	OCR1BH = 0x00;
	OCR1BL = (unsigned char) PositionTiltServo;
}

void servo_2_free (void) //makes servo 2 free rotating
{
	OCR1BH = 0x03;
	OCR1BL = 0xFF; //Servo 2 off
}





SIGNAL(SIG_USART0_RECV) 		// ISR for receive complete interrupt
{
	data = UDR0; 				//making copy of data from UDR0 in 'data' variable

	UDR0 = data; 				//echo data back to PC

	if(data == 0x38) //ASCII value of 8
	{
		PORTA=0x06;  //forward
				
	}

	if(data == 0x32) //ASCII value of 2
	{
		PORTA=0x09; //back
	}

	if(data == 0x34) //ASCII value of 4
	{
		PORTA=0x05;  //left
	}

	if(data == 0x36) //ASCII value of 6
	{
		PORTA=0x0A; //right
	}

	if(data==0x30){
		PORTA=0x00;
	}
	
	if(data==0x31){ // ascii value of 1
		unsigned char i=0;
		for (i = 0; i <3; i++)
		{
			servo_1(i);
			_delay_ms(30);
			
		}

		/*_delay_ms(2000);
		servo_1_free();
		while(1);*/
	}
	
	if(data==0x37){ 
		unsigned char i=0;
		for (i = 0; i <50; i++)
		{
			servo_1(i);
			_delay_ms(30);
			
		}

		/*delay_ms(2000);
		servo_1_free();
		while(1);*/
	}
	
	if(data==0x39){
		unsigned char i=0;
		for (i = 0; i <70; i++)
		{
			servo_2(i);
			_delay_ms(30);
			
		}

		/*_delay_ms(2000);
		servo_1_free();
		while(1);*/
	}
	   
	
	if(data==0x33){ // ascii value of 3
	   unsigned char i=0;
	   for (i = 0; i <3; i++)
	   {
		   servo_2(i);
		   _delay_ms(30);
		   
	   }

	  /* _delay_ms(2000);
	   servo_1_free();
	   while(1);*/
	  }
	  
	  
	  if(data==0x71){ // ascii value of q
	   unsigned char i=0;
	   for (i = 0; i <30; i++)
	   {
		   servo_2(i);
		   _delay_ms(30);
		   
	   }

	  /* _delay_ms(2000);
	   servo_1_free();
	   while(1);*/
	  }
	  
	 if(data==0x77){ // ascii value of w
	   unsigned char i=0;
	   for (i = 0; i <40; i++)
	   {
		   servo_2(i);
		   _delay_ms(30);
		   
	   }

	  /* _delay_ms(2000);
	   servo_1_free();
	   while(1);*/
	  }
	  
	 if(data==0x61){ // ascii value of a
	   unsigned char i=0;
	   for (i = 0; i <50; i++)
	   {
		   servo_2(i);
		   _delay_ms(30);
		   
	   }

	  /* _delay_ms(2000);
	   servo_1_free();
	   while(1);*/
	  }
	    
	 if(data==0x73){ // ascii value of s
	   unsigned char i=0;
	   for (i = 0; i <60; i++)
	   {
		   servo_2(i);
		   _delay_ms(30);
		   
	   }

	  /* _delay_ms(2000);
	   servo_1_free();
	   while(1);*/
	  }
	  
	

}





//Main Function
int main(void)
{
	init_devices();
	lcd_set_4bit();
	lcd_init();
	while(1);
}
