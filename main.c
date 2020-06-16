/* Dutch version
werkende ultrasoon code door alex jongejans 500820457
deze code kan succesvol de de ultrasoon laten trigeren en het singaal terug ontvagen 
deze zal worden verekent naar lengte in cm uitgaand van geluid dat rijst op 340M/S
dit word dan door een printf naar buiten gegooit uiteindelijk zal dit naar de pi moeten
*/
#define F_CPU     32000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "serialF0.h"
#include "clock.h"

// de pwm stuur een singaal naar de ultrasoon trigger van 10us om de sensor een meeting te laten doen
void init_pwm(void)
{
	PORTD.OUTSET = PIN0_bm;
	PORTD.DIRSET  = PIN0_bm;
	TCD0.CTRLB    = TC0_CCAEN_bm | TC_WGMODE_SINGLESLOPE_gc;
	TCD0.CTRLA    = TC_CLKSEL_DIV64_gc;
	TCD0.PER      = 30000;                                   
	TCD0.CCA      = 375;                         
}

// de input krijgt krijgt de echos terug de lengte van deze bepalen de afstand 
void init_inputcapture(void)
{
	PORTE.DIRCLR   = PIN1_bm;
	EVSYS.CH0MUX   = EVSYS_CHMUX_PORTE_PIN1_gc;
	TCE0.CTRLD     = TC_EVACT_PW_gc | TC_EVSEL_CH0_gc;
	TCE0.CTRLB     = TC0_CCAEN_bm | TC_WGMODE_NORMAL_gc;
	TCE0.CTRLA     = TC_CLKSEL_DIV64_gc;
	TCE0.INTCTRLB  = TC_CCAINTLVL_LO_gc;
}                           // MSB is low, so MSB of CCA holds edge


//volatile ints van de input omdat de waardes van buiten kommen MOETEN VOLATILE ZIJN !
volatile uint8_t  newCapture = 0;
volatile uint16_t period;



int main(void)
{
	/*aanroep functies
	 portf is er dat de rode lamp van de xmega zal mee vlikkeren 
	 met de input om te weten dat deze in werking is dit is debug en mag weg in de end build
	 ook de printf moet weg als deze niet nodig deze vertraagd het programa aanzienlijk
	 */
	init_clock();
	init_stream(F_CPU);
	init_pwm();
	init_inputcapture();
	
	PORTF.DIRSET   = PIN1_bm;				//led
	
	PMIC.CTRL |= PMIC_LOLVLEN_bm;           // uart, inputcapture
	sei();
	
// zal de input printen 
	while (1) {
		while (! newCapture) ;
	
		printf("%u\n", period);
		newCapture = 0;                      // clear newCapture flag
	}
}

// input capture en led flikker
ISR(TCE0_CCA_vect)
{
	PORTF.OUTSET = PIN1_bm;					//led flikker
	period = TCE0.CCA;						//input
	TCE0.CTRLFSET = TC_CMD_RESTART_gc;
	newCapture = 1;
	PORTF.OUTCLR = PIN1_bm;
}
