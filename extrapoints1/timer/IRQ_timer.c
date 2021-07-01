/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"
#include "../led/led.h"

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

extern unsigned int dir;
static unsigned int on1=0;
static unsigned int on2=0;
static unsigned int on3=0;

void TIMER0_IRQHandler (void) // gestisce il led direzionale
{
	if (on1){
		LED_On(dir);
		on1 = 0;
	} else {
		LED_Off(dir);
		on1 = 1;
	}
	
	LPC_TIM0->IR = 1;			/* clear interrupt flag */
	return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void) // gestisce il led di movimento con ostacoli frontali
{
	if (on2){
		LED_On(5);
		on2 = 0;
	} else {
		LED_Off(5);
		on2 = 1;
	}
	
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

void TIMER2_IRQHandler (void) // gestione di tutti i led in caso di vittoria
{
	if (on3){
		LED_On(0);
		LED_On(1);
		LED_On(2);
		LED_On(3);
		LED_On(4);
		LED_On(5);
		LED_On(6);
		LED_On(7);
		on3 = 0;
	} else {
		LED_Off(0);
		LED_Off(1);
		LED_Off(2);
		LED_Off(3);
		LED_Off(4);
		LED_Off(5);
		LED_Off(6);
		LED_Off(7);
		on3 = 1;
	}
	
  LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
