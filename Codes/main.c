#include <avr/interrupt.h>
#include <avr/signal.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <util/delay.h>

//initialize all ports
void init_ports()
{

	DDRA = 0x0F;
	PORTA = 0x00;
	PORTJ = 0x00;
	DDRJ  = 0xFF;
	PORTE = 0x00;
	DDRE  = 0x00;

}


//to control forward,backward, right turn,left turn motion of 
//blocks

void motion_set (unsigned char Direction) 
{

	unsigned char PortARestore = 0;
	Direction &= 0x0F; 
	PortARestore = PORTA; 
	PortARestore &= 0xF0; 
	PortARestore |= Direction; 
	PORTA = PortARestore; 

}

//timer0 init

void timer0_init(void)
{

	//8-bit timer for measuring delay between IR pulses
	TCCR0 = 0x03;//CLK / 64
	TCNT0 = 0; //timer reset

}


//Function to read IR message from the detector
//Return value contains code in upper byte and addr in lower 
//byte
unsigned int IR_read (void)
{

unsigned char Pulse_counter=0, code = 0, addr = 0;
unsigned int input_IR;

	//capturing the 5 bit addr codes
	while(Pulse_counter < 5)
	{

	_delay_ms(1.8); // pulses are at a delay of 1.8ms
	Pulse_counter++;
	//value at PORTE 7th pin identified and appropriately appended
		if((PINE & 0x80) == 0x80)
		{
			addr = addr & ~(1 << (Pulse_counter-1));
		}
		else
		{
			addr = addr | (1 << (Pulse_counter-1));
		}

	}

	Pulse_counter =0;

	//capturing the last 6 bit codes
	while(Pulse_counter < 6)
	{

	_delay_ms(1.8); // pulses are at a delay of 1.8ms
	Pulse_counter++;
	//value at PORTE 7th pin identified and appropriately appended
		if(PINE & 0x80 == 0x80)
		{
			code = code & ~(1 << (Pulse_counter-1));
		}
		else
		{
			code = code| (1 << (Pulse_counter-1));
		}
	
	}

input_IR = (((unsigned int)code) << 8) | addr;
//Return value contains code in upper byte and addr in lower byte
return(input_IR);

}


//to perform necessary actions
int Action(unsigned int x)
{

	if(x == (unsigned int) 54)//values may vary
	{
		motion_set(0x00);//stop
	}

	if(x == (unsigned int) 16)
	{
		motion_set(0x06);//forward movement
	}

	if(x == (unsigned int) 13)
	{
		motion_set(0x09);//backward movement
	}

return 1;

}


//interrupt which is called when IR signal is received by the sensor
ISR(INT7_vect) 
{

unsigned int count, code, addr, input_IRR;
code=(unsigned char)0x00;
PORTJ = 0x04; //on receiving an interrupt PORTJ changes value to show interrupt raised	   

	//to avoid the initial 3 bits
	while((PINE & 0x80) == 0x80);_delay_ms(2.4);
	if((PINE & 0x80) == 0x00){}else{return 0;}

	_delay_ms(1.8);
	input_IRR = IR_read ();//reading input by calling function,obtain address,code

	code =  ((input_IRR & 0xff00) >> 8);//extracting code from the return value of IR_read()-upper bits
	addr =  (input_IRR & 0x00ff);////extracting addr from the return value of IR_read()-lower bits
	PORTJ = code;// to depict the button pressed
	int n = Action(code);	
	
}


//To initialise all the devices
void init_devices()
{

	cli(); //Clears the global interrupt

	init_ports(); //To initialize all the other required ports
	EICRA = 0x00; 
	EICRB = 0xc0; // trigger interrupt for INT7-INT4
	EIMSK = 0x80; // INT7 enabled

	sei();   // Enables the global interrupt

}


//main function
int main(void)
{

init_devices();

	while(1)
	{
		if((PINE & 0x80) == 0x80) PORTJ = 0x01;// PORTJ used as a marker
	}
   
}
