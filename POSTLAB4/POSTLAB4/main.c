/* Universidad del Valle de Guatemala
IE2023:: Programación de Microcontroladores
PRELAB4.c
Autor: Alejandra Cardona
Hardware: ATMEGA328P
Creado: 08/04/2024
Última modificación: 09/04/2024

****************************************************************** */

// LIBRERÍAS

#define F_CPU 16000000 //Frecuencia en la que opera el sistema - 16 MHz

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Variable Formato: Tipo nombre = Valor

//Variables: Contador 8 bits
uint8_t contador8bits = 0;  //Contador de 8 bits en 0

//Variables: Display Hexadecimal
int valueADC = 0; 
int have4A = 0; //High
int have4B = 0; //Low
unsigned char t7s[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71}; // Lista Hexadecimal

//******************************************************************

void setup(void);
void initADC(void);

int main(void){ //Función main
	
	setup();
	initADC();
	
	//LOOP 
	while (1){
		
		ADCSRA |= (1<<ADSC); //Inicia ADC

		have4A = valueADC/16;
		have4B = valueADC%16; //Residuo
		
		PORTD = t7s[have4A]; // Saca el valor de la lista correspondiente al puerto 
		
		PORTB &= ~(1<<PORTB0); //Apaga las LEDs
		PORTB &= ~(1<<PORTB2); //Apaga el segundo display
		PORTB |= (1<<PORTB1); //Enciende el primer display
		_delay_ms(5);
		
		PORTD = t7s[have4B]; // Saca el valor de la lista correspondiente al puerto
		
		PORTB &= ~(1<<PORTB0); //Apaga las LEDs
		PORTB &= ~(1<<PORTB1); //Apaga el primer display
		PORTB |= (1<<PORTB2); //Enciende el segundo display
		_delay_ms(5);
		
		PORTD = contador8bits; // Muestra en el puerto D lo que hay en el contador
		
		PORTB |= (1<<PORTB0); //Enciende las LEDs
		PORTB &= ~(1<<PORTB1); //Apaga el primer display
		PORTB &= ~(1<<PORTB2); //Apaga el segundo display
		_delay_ms(5);
		
		if ((valueADC) > (contador8bits)) {
			// Cuando los valores son iguales se enciende la LED alarma
			PORTB |= (1<<PORTB3); 
		}
		else {
			// De lo contrario los valores se apaga la LED alarma
			PORTB &= ~(1<<PORTB3);
		}
		
	}
	
}

void setup(void){ //Se utiliza void cuando no se emplean parámetros
	
	cli(); //Deshabilita interrupciones
	
	UCSR0B = 0; //Deshabilita RX y TX
	
	// Puerto D como salidas
	DDRD = 0xFF;
	PORTD = 0;
	
	// Puerto B como salidas
	DDRB = 0xFF;
	PORTB = 0;
	
	//Pines PC4 y PC5 como entradas
	DDRC &= ~(1 << PINC4) | ~(1 << PINC5);
	PORTC |= (1 << PINC4) | (1 << PINC5);
	
	
	// Interrupción del contador 8 bits
	PCICR |= (1<<PCIE1); // PCIE1 para puerto C
	PCMSK1 |= ((1<<PCINT12)|(1<<PCINT13)); // Habilita la interrupción en el puerto C -- PCMSK1 corresponde al puerto C -- PCINT12-PC4 y PCINT13-PC5
	
	sei(); // Habilita las interrupciones 
	
}

//Interrupciones
ISR(ADC_vect){
	
	valueADC = ADCH;
	ADCSRA |= (1<<ADIF);
}


ISR(PCINT1_vect){
	
	if (!(PINC & (1 << PINC4))) { 
		// Cuando botón 0 presionado:
		contador8bits++;
		}
	else if (!(PINC & (1 << PINC5))) {
		// Cuando botón 1 presionado:
		contador8bits--;
	}
	else if((!(PINC & (1 << PINC4))) && (contador8bits==255))  {
		//Cuando el contador llegué a su máximo (255) y botón 0 presionado, entonces se suma  y regresa a 0
		contador8bits=0;
	}
	else if((!(PINC & (1 << PINC5))) && (contador8bits==255))  {
		//Cuando el contador llegué a su máximo (255) y botón 1 presionado, entonces se resta y regresa a 254
		contador8bits=254;
	}
	else {
		// De lo contrario
		contador8bits=contador8bits;
	}
}

void initADC(void){ 
	// Configurando bits de ADC

	// Seleccionando una referencia (Vref)=AVCC=5V
	ADMUX = 0; // Configurando el canal 0
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);

	// ADLAR - Justificación hacia la izquierda
	ADMUX |= (1 << ADLAR);

	ADCSRA = 0; 
	// ADEN - Habilita o enciende ADC
	ADCSRA |= (1 << ADEN);

	// ADIE - Habilita ISR ADC
	ADCSRA |= (1 << ADIE);

	// Ya que nos encontramos en el rango permitido, es posible usar alta resolución 
	// Prescaler 128 > 16MHz / 128 = 125KHz
	ADCSRA |= (1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0);

}