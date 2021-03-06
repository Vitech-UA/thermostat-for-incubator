#include <mega8.h>
#include <delay.h>
#include <1wire.h>
#include <ds18b20.h>
#include <alcd.h>
#include <stdlib.h>


#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (1<<ADLAR))
#define RELE PORTD.1
#define GISTERESIS 0.1


unsigned char read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCH;
}


float temper = 0.0;
float Dtemper = 0.0;

char lcd_buffer[20];
char lcd_buffer2[20];

interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{        
           lcd_clear();           
           lcd_gotoxy(0,0);
           lcd_puts("Incubator= ");
           
           lcd_gotoxy(11,0);
           ftoa(temper,1,lcd_buffer);
           lcd_puts(lcd_buffer); 
           
           lcd_gotoxy(0,1);
           lcd_puts("Termostat= ");
           
           lcd_gotoxy(11,1);
           ftoa(Dtemper,1,lcd_buffer2);
           lcd_puts(lcd_buffer2);        
          
          
          TCNT1H=0;
          TCNT1L=0;                  
}


void Init_Port(void)
{
 DDRB=0xFF;
 PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);
 DDRC=(0<<DDC6) | (0<<DDC5) | (0<<DDC4) | (0<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
 PORTC=(0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0);
 DDRD=(0<<DDD7) | (0<<DDD6) | (0<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (1<<DDD1) | (0<<DDD0);
 PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);
 
}

void InitTimer(void)
{
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (1<<CS12) | (0<<CS11) | (1<<CS10);
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x0B;
OCR1AL=0xB8;
OCR1BH=0x00;
OCR1BL=0x00;
// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (1<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<TOIE0);
}


void main(void)
    {
        TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<TOIE0);
        MCUCR=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
        UCSRB=(0<<RXCIE) | (0<<TXCIE) | (0<<UDRIE) | (0<<RXEN) | (0<<TXEN) | (0<<UCSZ2) | (0<<RXB8) | (0<<TXB8);
        
    //////////////////////////���///////////////////////////   
        // ADC initialization
        // ADC Clock frequency: 1000,000 kHz
        // ADC Voltage Reference: AREF pin
        // Only the 8 most significant bits of
        // the AD conversion result are used
        ADMUX=ADC_VREF_TYPE;
        ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADFR) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
        SFIOR=(0<<ACME);       
        
    //////////////////////////������������_�������///////////////////////////    
        InitTimer();
        Init_Port();
        w1_init();
        lcd_init(16);
        ds18b20_init(0,0,40,1); 
        
        #asm("sei");                       


    while (1)
          {

           temper = (ds18b20_temperature(0));
           
           Dtemper = read_adc(0) / 6.375;  
           
           
           if(temper > Dtemper)
           {
            RELE = 1;
           }
           
           if(temper < Dtemper - GISTERESIS)
           {
            RELE = 0;
           }
        
                    
          }
}
