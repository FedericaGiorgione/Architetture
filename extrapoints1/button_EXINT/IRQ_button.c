#include "button.h"
#include "lpc17xx.h"

//	#include "../led/led.h" 					/* do not needed anymore, make your project clean */
#include "../RIT/RIT.h"		  			/* you now need RIT library 			 */

extern unsigned int win;

int button = -1;

void EINT0_IRQHandler (void)	  	/* INT0														 */
{		
	if(win){
		button = 0;
		// anti rimbalzo
		enable_RIT();										/* enable RIT to count 50ms				 */
		NVIC_DisableIRQ(EINT0_IRQn);		/* disable Button interrupts			 */
		LPC_PINCON->PINSEL4    &= ~(1 << 20);     /* GPIO pin selection */
	}
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	if(!win){
		button = 1;
		// anti rimbalzo
		enable_RIT();										/* enable RIT to count 50ms				 */
		NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button interrupts			 */
		LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */
	}
	
	// rilascia l'interrupt
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	if(!win){
		button = 2;
		// mi assicuro di tenere premuto il tasto per almeno un secondo
		enable_RIT();										
		NVIC_DisableIRQ(EINT2_IRQn);		/* disable Button interrupts			 */
		LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */
	}
	// rilascia l'interrupt	
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


