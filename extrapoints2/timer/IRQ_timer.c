/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "lpc17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

#define R 13
#define COLOR_E Yellow
#define COLOR_M Green

extern int pos[2];
extern unsigned int mode; // 0 = explore, 1 = run

extern unsigned int win;

extern void matrice(void);
extern void triangle(int x, int y, uint16_t color);
extern void start(void);

extern void clearMatrix();

int but_r=0;
int but_c=0;

void TIMER0_IRQHandler (void)
{
	// rilevo il tocco sullo schermo
	getDisplayPoint(&display, Read_Ads7846(), &matrix ) ;
	#ifdef SIMULATOR
	if(display.x <= 240 && display.x > 0 && display.y < 0x013D){
	#elif
	if(display.x <= 240 && display.x > 0 && display.y != 0){
	#endif
		// disegn0 la schermata di gioco
		LCD_Clear(Black);
	
		GUI_Text(60, 20, (uint8_t *) "BLIND LABIRINTH", Green, Black);
		
		matrice();
		
		// disegno il tasto restart
		LCD_DrawLine(30, 274, 102, 274, Green); // alto
		LCD_DrawLine(30, 274, 30, 306, Green); // sx
		LCD_DrawLine(30, 306, 102, 306, Green); // basso
		LCD_DrawLine(102, 274, 102, 306, Green); // dx
		GUI_Text(38, 282, (uint8_t *) "RESTART", Green, Black);
		
		// disegno il tasto clear
		LCD_DrawLine(138, 274, 210, 274, Green);
		LCD_DrawLine(138, 274, 138, 306, Green);
		LCD_DrawLine(138, 306, 210, 306, Green);
		LCD_DrawLine(210, 274, 210, 306, Green);
		GUI_Text(154, 282, (uint8_t *) "CLEAR", Green, Black);
		
		// disegno il triangolo dando le coordinate dell'angolo in alto a sinistra della cella
		triangle(pos[0]*16, pos[1]*16+56, (!mode) ? COLOR_E : COLOR_M);
		
		// attivo timer1 per i tasti di restart e clear
		init_timer(1, 0x4E2 ); 						    /* 500us * 25MHz = 1.25*10^3 = 0x4E2 */
		enable_timer(1);
		
		disable_timer(0);
	}	
	else{
		//do nothing if touch returns values out of bounds
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
void TIMER1_IRQHandler (void)
{
	// rilevo il tocco sullo schermo
	getDisplayPoint(&display, Read_Ads7846(), &matrix ) ;
	#ifdef SIMULATOR
	if(display.x <= 240 && display.x > 0 && display.y < 0x013D){
	#elif
	if(display.x <= 240 && display.x > 0 && display.y != 0){
	#endif
		// area del tasto restart
		if(display.x <= 102 && display.x >= 30 && display.y <= 306 && display.y >=274 ){
			start();
		}
		else{			
			// area del tasto clear
			if(display.x <= 210 && display.x >= 138 && display.y <= 306 && display.y >=274 && !win){		
				// pulisco l'area della matrice con una versione modificata della clear di libreria
				clearMatrix();
				
				// ridisegno matrice e robot
				matrice();
				triangle(pos[1]*16, pos[0]*16+56, (!mode) ? COLOR_E : COLOR_M);
			}
		}
	}
	else{
		//do nothing if touch returns values out of bounds
	}
		
		
		
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
