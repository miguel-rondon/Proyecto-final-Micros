/*
 * 2servosjoystick.c
 *
 * Created: 3/11/2021 3:23:35 p. m.
 * Author : Miguel Palomino
 */ 

#define F_CPU 16000000L
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int cuenta=0;
void ADC_init()
	
{
	ADCSRA=0x87; //ADC enable, prescaler 128
	ADMUX=0x40; // Voltaje de referncia VCC, 1 <<REFS0
		
}

int	ADC_read(char channel)

{
	if (channel==1)
	{
		ADMUX=0x43; //activado A3
	}else if (channel==0)
	{
		ADMUX=0x40 ; //activado A0
     }else if (channel==2)
     {
	    ADMUX=0x42 ; //activado A0
     }
	
	ADCSRA|=(1<<ADSC); //inicia conversión
	
	while (!(ADCSRA & (1<<ADIF))); //Esperar hasta que termine la conversión
	ADCSRA|=(1<<ADIF);  //limpiar bandera interrupción
	_delay_ms(1);
	return ADCW; //return ADC word
}





int main(void)
{
	DDRB |= ( 1<< PORTB3 );
	//1 Configuramos el PB3 como salida.
	// initialize TCCR2 as: " ast pum mode, non inverting
	TCCR2A |= (1<<COM2A1 ) | (1<<COM2A0) | (1<<WGM20) | (1<<WGM21);
	TCCR2B |= (1<<CS22) | (0<<CS21) | (0<<CS20); // cIkT2S/256 prescaler
	
	//Interrupciones INT1 e INT0
	EIMSK = 0b00000011; //HABILITAR INT1 E INT0
	EICRA = 0b00001111; // ACTIVACION POR FLANCO SUBIDA
	// Banderas de INT0 limpias
	sei();
	
	DDRD=0xFF;
    DDRB=0xFF;
	ADC_init(); //llama método
	TCNT1=0;
	ICR1=4999; //Top timer 1 para periodo de 20 mS
	
	//Configurar modo fast PWM, TOP en ICR1, limpiar OC1A, OC1B en compare match, clk/64
	
	TCCR1A=(1<<WGM11)|(1<<COM1A1)|(1<<COM1B1);
	TCCR1B=(1<<WGM12)|(1<<WGM13)|(1<<CS10)|(1<<CS11);

	
    while (1) 
    {
		//OCR1A	= (ADC_read(0)*0.199);
		//OCR1B	= (ADC_read(1)*0.199);
		
		OCR1A	= 250 + (ADC_read(0)/4.1); //Asignación oc1ra y ocr1b teniendo en cuenta el rango de funcionamiento del servo
        OCR1B	= 250 + (ADC_read(1)/4.1);		
		
		//lo siguiente es para el motor paso a paso
		
		if(ADC_read(2)>600)
		{
			
			
			
			for (int i = 0; i < 64; i++)   //giro a la derecha, full step, velocidad intermedia
			{
				
				PORTD=0b00000001;
				_delay_ms(15);
				PORTD=0b00000010;
				_delay_ms(15);
				PORTD=0b00001000;
				_delay_ms(15);
				PORTD=0b00010000;
				_delay_ms(15);
				
			}
			
		}else
		{
			if(ADC_read(2)<400)
			{
				
				
				for (int i=0; i < 64; i++) //giro a la izquierda, full step, velocidad intermedia
				{
					
					PORTD=0b00010000;
					_delay_ms(15);
					PORTD=0b00001000;
					_delay_ms(15);
					PORTD=0b00000010;
					_delay_ms(15);
					PORTD=0b00000001;
					_delay_ms(15);
					
				}
				
				
				
			}else
			{
				PORTD=0b00000000;  // si no se cumplen las condiciones anteriores se detiene
				
			}
		}
		
    }
}
ISR(INT0_vect)  // PRESIONAR BOTON POWER (INICIO)
{
	cuenta++;
	if (cuenta==2)
	{
		PORTB|=(1<<4);
		OCR2A = 120;
	} else if (cuenta==4)
	{
		PORTB&= ~(1<<4);
		OCR2A = 0;
		cuenta=0;
	}
	
	// OCR2A = 0; //tiempo en ms para 90° del servo
}

