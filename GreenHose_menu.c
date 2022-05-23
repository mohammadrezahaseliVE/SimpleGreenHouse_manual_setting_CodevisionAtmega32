/*******************************************************
Project : Simple GreenHouse With Manual Setting
Version : 3.0.0
Date    : 05/19/2022
Author  : Mohammad Reza Haseli 
Comments: int this project will show an avrage value of two temperature
and value of light. 
it is do some actitys by changing temp or torch and also can change refrence value
with help of buttons and change condition by user.


Chip type               : ATmega32
Program type            : Application
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 512
*******************************************************/

// Declare Function
#include <mega32.h>
#include <stdio.h>
#include <delay.h>
#include <alcd.h>

// Declare Output PortB
#define greenLED  PORTB.0
#define blueLED  PORTB.1
#define redLED  PORTB.2
#define yellowLED  PORTB.3

// Declare Iutput PIND
#define inc_btn  PIND.0
#define dec_btn  PIND.1
#define switch_btn  PIND.2
#define set_btn  PIND.3

// Declare your global variables here
unsigned int adc_temp1, adc_temp2, adc_torch;                                                                                     
int count = 0;
//char data_temp[4], data_torch[4];
char data[16];

//refrence value for temp and torch
int Htemp_ref = 22, Ltemp_ref = 18, torch_ref = 100;

//change use for define witch parameter has to change
int change = 5;

//Function displaying parameters on lcd
void Displaye();

//Function that compare adc value with refrence and put condition
void AdcControl(int temp1, int temp2, int torch);

// External Interrupt 0 service routine
interrupt [EXT_INT0] void interrupt_0(void)
{
    change = 0;
    while(1)
    {     
        // 0 = High temp;   1 = Low temp;    2 = torch           
        if (switch_btn == 0)
        {   
            while(switch_btn == 0);
            change++;
        }
        else if(inc_btn == 0)
        {   
            while(inc_btn == 0);
            if (change == 0) 
                {if (Ltemp_ref < Htemp_ref-1) Ltemp_ref++;}
            else if (change == 1) Htemp_ref++;
            else if (change == 2) 
                {if (torch_ref < 1000)torch_ref += 50;}
        }
        else if(dec_btn == 0)
        {   
            while(dec_btn == 0);
            if (change == 0) Ltemp_ref--;
            else if (change == 1) 
                {if (Ltemp_ref+1 < Htemp_ref)Htemp_ref--;}
            else if (change == 2)
                {if (torch_ref > 0)torch_ref -= 50;}
        }                      
        
        if (change == 0)
        {            
            lcd_gotoxy(12, 0);
            lcd_puts("  ");
            delay_ms(200);
            lcd_gotoxy(12, 0);
            sprintf(data, "%02d", Ltemp_ref);
            lcd_puts(data);
            delay_ms(200);
        }
        else if (change == 1)
        {              
            lcd_gotoxy(18, 0);
            lcd_puts("  ");
            delay_ms(200);
            lcd_gotoxy(18, 0);
            sprintf(data, "%02d", Htemp_ref);
            lcd_puts(data);
            delay_ms(200);
        }
        else if (change == 2)
        {              
            lcd_gotoxy(13, 1);
            lcd_puts("    ");
            delay_ms(200);
            lcd_gotoxy(13, 1);
            sprintf(data, "%04d", torch_ref);
            lcd_puts(data);
            delay_ms(200);
        } 
        else {lcd_clear(); break;}
    }
}

// External Interrupt 0 service routine
interrupt [EXT_INT1] void interrupt_1(void)
{
    Htemp_ref = 22, Ltemp_ref = 18, torch_ref = 100;
    delay_ms(1000);
    lcd_clear();        

}


// ADC interrupt service routine
interrupt [ADC_INT] void adc_isr(void)
{
    
    //count will increse each time intrupt called and help to swith between adc pins (0-4)
    count++;
    if (count == 3) count = 0;

    if (count == 0)
    {
        ADMUX=0x01;
        adc_temp1 = ADCW;
        adc_temp1 = (( adc_temp1 * 4.8) / 10)+1;    
    }
    
    else if (count == 1)
    {
        ADMUX=0x02;
        adc_temp2 = ADCW;                        
        adc_temp2 = (( adc_temp2 * 4.8) / 10)+1;
    }
    
    else if (count == 2)
    {
        ADMUX=0x00;
        adc_torch = ADCW;      
    }
         
    //start conversion
    ADCSRA |= (1<<ADSC); 
    
    AdcControl(adc_temp1, adc_temp2, adc_torch);                        
}

void main(void)
{
    // Declare PORTA as input
    DDRA = 0x00;
    
    // Declare PORTB as input
    DDRB = 0xFF; 
    
    // Declare PORTD pin 0, 1, 2 as input 
    DDRD = 0x00;
    PIND = 0xFF; 
    
    // External Interrupt(s) initialization
    // INT0: On
    // INT0 Mode: Rising Edge
    // INT1: Off
    // INT2: Off
    GICR|=(1<<INT1) | (1<<INT0) | (0<<INT2);
    MCUCR=(1<<ISC11) | (1<<ISC10) | (1<<ISC01) | (1<<ISC00);
    MCUCSR=(0<<ISC2);
    GIFR=(1<<INTF1) | (1<<INTF0) | (0<<INTF2);
    
    // ADC initialization
    // ADC Clock frequency: 1000.000 kHz
    // ADC Voltage Reference: AREF pin
    // ADC Auto Trigger Source: ADC Stopped
    ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    SFIOR=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
    ADCSRA |= (1<<ADSC);
    
    // Global enable interrupts
    #asm("sei") 
    
    lcd_init(20);
    lcd_clear();

    while (1)
          {
            Displaye();                                             
          }
}


void AdcControl(int temp1, int temp2, int torch)
{   

    //condition for temp 
    if (((temp1 + temp2)/2) >  Htemp_ref)
    {
        greenLED= 1;
        blueLED = 0;
        redLED = 0;
        
    }
    else if (((temp1 + temp2)/2)  <  Ltemp_ref)
    {
        greenLED = 0;
        blueLED = 0;
        redLED = 1;
    }
    
    else if ((((temp1 + temp2)/2)  >=  Ltemp_ref) && (((temp1 + temp2)/2)  <=  Htemp_ref))
    {
        greenLED = 0;
        blueLED = 1;
        redLED = 0;
    }

    //condition for torch
    if (torch >= torch_ref)
    {
        yellowLED= 1;
    }
    else if (torch < torch_ref)
    {
        yellowLED= 0;
    } 

}


void Displaye()
{
    lcd_gotoxy(0, 0);
    lcd_puts("ref Temp: ");
    lcd_gotoxy(10, 0);
    sprintf(data, "L %02d  H %02d", Ltemp_ref, Htemp_ref);
    lcd_puts(data);
      
    lcd_gotoxy(0 ,1);
    lcd_puts("ref Torch: "); 
    lcd_gotoxy(13, 1);
    sprintf(data, "%04d", torch_ref);
    lcd_puts(data);
    
    lcd_gotoxy(0, 3);
    lcd_puts("Temp: ");
    sprintf(data, "%02d", ((adc_temp1+adc_temp2)/2));
    lcd_puts(data);    
    lcd_gotoxy(9, 3);
    lcd_puts("Torch: ");
    sprintf(data, "%04d", adc_torch);
    lcd_puts(data);    
}
