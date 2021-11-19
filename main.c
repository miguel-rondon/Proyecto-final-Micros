/*
 * PruebaProyectoMicros.c
 *
 * Created: 7/11/2021 1:20:56 a. m.
 * Author : Ivan Holguin - Steven Rondon - Miguel Campo
 */

#define F_CPU 16000000L

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Mis variable
int mi_valor = 0;
int cuenta = 0;

// Mis funciones
void motorPP();
void servos();

void ADC_init()
{
	ADCSRA = 0x87; //ADC enable, prescaler 128
	ADMUX = 0x40;  // Voltaje de referncia VCC, 1 <<REFS0
}

int ADC_read(char channel)
{
	if (channel == 1)
	{
		ADMUX = 0x43; //activado A3
	}
	else if (channel == 0)
	{
		ADMUX = 0x40; //activado A0
	}
	else if (channel == 2)
	{
		ADMUX = 0x41; //activado A1
	}

	ADCSRA |= (1 << ADSC); //inicia conversión

	while (!(ADCSRA & (1 << ADIF)))
		;				   //Esperar hasta que termine la conversión
	ADCSRA |= (1 << ADIF); //limpiar bandera interrupción
	_delay_ms(1);
	return ADCW; //return ADC word
}

/*
 * --------------------------------------------------------------------
 * Principal
 * --------------------------------------------------------------------
 */

int main(void)
{
	DDRB = 0xFF;

	// Salida TIMER 2
	DDRB |= (1 << PORTB3);
	TCCR2A |= (1 << COM2A1) | (1 << COM2A0) | (1 << WGM20) | (1 << WGM21);
	TCCR2B |= (1 << CS22) | (0 << CS21) | (0 << CS20); // PRESCALAR 64

	//Interrupciones INT1 e INT0
	EIMSK = 0b00000011; //HABILITAR INT1 E INT0
	EICRA = 0b00001111; // ACTIVACION POR FLANCO SUBIDA

	// Banderas de INT0 limpias
	ADC_init(); //llama método
	sei();
	
	TCNT1 = 0;
	ICR1 = 4999; //Top timer 1 para periodo de 20 mS
	//Configurar modo fast PWM, TOP en ICR1, limpiar OC1A, OC1B en compare match, clk/64

	TCCR1A = (1 << WGM11) | (1 << COM1A1) | (1 << COM1B1); // Habilitar A & B
	TCCR1B = (1 << WGM12) | (1 << WGM13) | (1 << CS10) | (1 << CS11); // PRESCALAR 64

	while (1)
	{
		servos();
		motorPP();
	}
}

void servos()
{
	// Asignación OCR1A & OCR1B
	OCR1A = 100 + (ADC_read(0) / 1.4);	// Con referencia
	OCR1B = (ADC_read(1) / 1.4);		// escalizacion
}

void motorPP()
{
	mi_valor = ADC_read(2);

	if (mi_valor > 800)
	{
		for (int i = 0; i < 64; i++) //giro a la derecha, full step, velocidad intermedia
		{
			PORTD = 0b00010000;
			_delay_ms(15);
			PORTD = 0b001000000;
			_delay_ms(15);
			PORTD = 0b010000000;
			_delay_ms(15);
			PORTD = 0b10000000;
			_delay_ms(15);
		}
	}
	else
	{
		if (mi_valor < 200)
		{
			for (int i = 0; i < 64; i++) //giro a la izquierda, full step, velocidad intermedia
			{

				PORTD = 0b10000000;
				_delay_ms(15);
				PORTD = 0b01000000;
				_delay_ms(15);
				PORTD = 0b00100000;
				_delay_ms(15);
				PORTD = 0b00010000;
				_delay_ms(15);
			}
		}
		else
		{
			PORTD = 0b00000000; // Detener motor
		}
	}
}

// Accion Garra
ISR(INT0_vect)
{
	cuenta++;
	if (cuenta == 2)
	{
		PORTB |= (1 << 4);
		OCR2A = 50;
	}
	else if (cuenta == 4)
	{
		PORTB &= ~(1 << 4);
		OCR2A = 0;
		cuenta = 0;
	}
}
