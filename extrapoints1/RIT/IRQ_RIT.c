/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../led/led.h"
#include "../timer/timer.h"


#define R 13
#define C 15

char labirinto[R][C] = {
		{2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2},
		{0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
		{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2},
	};
int pos[2];
unsigned int dir; // N=3, E=2, S=1, W=0

unsigned int win=1; // flag per abilitare il tasto START
unsigned int exit_present; // flag che indica la presenza di un'uscita frontalmente
	
extern int button;

unsigned int dist(void){
	int i=0;
	exit_present = 0;
	
	if(dir==3){
		while ((labirinto[pos[0]-i-1][pos[1]] != 1) && ((pos[0]-i) > 0)){
			if (labirinto[pos[0]-i-1][pos[1]] == 2)
				exit_present = 1;
			i++;
		}
	} else if(dir==2){
		while ((labirinto[pos[0]][pos[1]+i+1] != 1) && ((pos[1]+i) < 14)){
			if (labirinto[pos[0]][pos[1]+i+1] == 2)
				exit_present = 1;
			i++;
		}
	} else if(dir==1){
		while ((labirinto[pos[0]+i+1][pos[1]] != 1) && ((pos[0]+i) < 12)){
			if (labirinto[pos[0]+i+1][pos[1]] == 2)
				exit_present = 1;
			i++;
		}
	} else {
		while ((labirinto[pos[0]][pos[1]-i-1] != 1) && ((pos[1]-i) > 0)){
			if (labirinto[pos[0]][pos[1]-i-1] == 2)
				exit_present = 1;
			i++;
		}
	}
	
	return i;
}
void radar(void){
	unsigned int i; // distanza dall'ostacolo
	
	// calcolo la distanza del robot dall'ostacolo frontale
	i = dist();
	
	disable_timer(0);
	reset_timer(0);
	disable_timer(1);
	reset_timer(1);
	disable_timer(2);
	reset_timer(2);
	if(i==0 && !exit_present){
		// 8Hz -> periodo: 0.125s -> 0.0625s
		init_timer(0, 0x0017D784); // 25MHz*0.0625s
		enable_timer(0);
	}
	else if(i<=2 && !exit_present){
		// 4Hz -> periodo: 0.25s -> 0.125s 
		init_timer(0, 0x002FAF08); // 25MHz*0.125s 
		enable_timer(0);
	}
	else if(i<=5 && !exit_present){
		// 2Hz -> periodo: 0.5s -> 0.25s
		init_timer(0, 0x005F5E10); // 25MHz*0.25s
		enable_timer(0);
	}
	else{
		LED_On(dir);
	}
}


void start(void){
	// riporto il robot nella posizione iniziale
	pos[0] = 7;
	pos[1] = 7;
	dir = 2; // E
	
	// riporto il flag di vittoria a zero
	win = 0;
	
	// setup dei led
	LED_Off(0);
	LED_Off(1);
	LED_Off(2);
	LED_Off(3);
	LED_Off(4);
	LED_Off(5);
	LED_Off(6);
	LED_Off(7);
	
	// setup dei timer e accendo il led direzionale
	radar();
}

int check(void){
	if (labirinto[pos[0]][pos[1]] == 2){
		// setto la variabile di vittoria
		win = 1;
		
		disable_timer(0);
		reset_timer(0);
		disable_timer(1);
		reset_timer(1);
		disable_timer(2);
		reset_timer(2);
		
		// 1Hz -> periodo: 1s -> 0.5s
		init_timer(2, 0x00BEBC20); // 25MHz*0.5s
		enable_timer(2);
	}
	
	return win;
}
/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void RIT_IRQHandler (void)
{	
	static int down_start=0;
	static int down_rotate=0;
	static int running=0;
	static int down_run=0;
	
	
/******************************************************************************
** START
******************************************************************************/
	if(button == 0) {
		down_start++; 
		if(!(LPC_GPIO2->FIOPIN & (1<<10)) && !(LPC_PINCON->PINSEL4 & (1<<20))){ // leggi pin 10 (EINT0) e verifica sia premuto
			reset_RIT();
			switch(down_start){
				case 1:
					start();
					break;
				default:
					break;
			}
		}
		else {	/* button released */
			button = -1;
			down_start=0;			
			disable_RIT();
			reset_RIT();
			NVIC_EnableIRQ(EINT0_IRQn);							 /* disable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 20);     // EINT1
		}
	} 
	
/******************************************************************************
** ROTATE
******************************************************************************/
	else if(button == 1) {
		down_rotate++; 
		if(!(LPC_GPIO2->FIOPIN & (1<<11)) && !(LPC_PINCON->PINSEL4 & (1<<22))){ // leggi pin 11 (EINT1) e verifica sia premuto
			reset_RIT();
			switch(down_rotate){
				case 1:
					// spengo il led della precedente direzione
					LED_Off(dir);
						
					// cambia direzione
					if( dir == 3){ // dir vale massimo 3
						dir = 0;
					} else
						dir++;
									
					// rileva ostacoli e accendi il nuovo led
					radar();

					break;
				default:
					break;
			}
		}
		else {	/* button released */
			button = -1;
			down_rotate=0;			
			disable_RIT();
			reset_RIT();
			NVIC_EnableIRQ(EINT1_IRQn);							 /* disable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 22);     // EINT1
		}
	} 
		
/******************************************************************************
** RUN
******************************************************************************/
	else if (button == 2) {
		down_run++;
		if(!(LPC_GPIO2->FIOPIN & (1<<12)) && !(LPC_PINCON->PINSEL4 & (1<<24))){ // leggi pin 12 (EINT2) e verifica sia premuto
			reset_RIT();
			switch(down_run%20){
				case 0: // devo tenere premuto per un secondo (50ms*20) per abilitare lo spostamento
					// se non ho ostacoli frontali proseguo come segue
				
					LED_Off(5); // gestione del led di movimento
					if (dist()) {			
						// sposto il robot
						if(dir==3){
								if ((labirinto[pos[0]-1][pos[1]] != 1) && (pos[0] > 0)){
									pos[0]--;
								}
							} else if(dir==2){
								if ((labirinto[pos[0]][pos[1]+1] != 1) && (pos[1] < 14)){
									pos[1]++;
								}
							} else if(dir==1){
								if ((labirinto[pos[0]+1][pos[1]] != 1) && (pos[0] < 12)){
									pos[0]++;
								}
							} else {
								if ((labirinto[pos[0]][pos[1]-1] != 1) && (pos[1] > 0)){
									pos[1]--;
								}
							}
							
							if (check()){	
								button = -1;
								down_run=0;	
								disable_timer(1);
								reset_timer(1);
								disable_RIT();
								reset_RIT();
								NVIC_EnableIRQ(EINT2_IRQn);							 /* disable Button interrupts			*/
								LPC_PINCON->PINSEL4    |= (1 << 24);     // EINT2
									
								break;
							}								
			
							// aggiorno la frequenza del led di rotazione con la nuova distanza
							radar();
					} else {			
						// ho di fronte un ostacolo, lampeggia a 5Hz -> periodo: 0.2s -> 0.1s
						disable_timer(1);
						reset_timer(1);
						init_timer(1, 0x002625A0); // 25MHz*0.1s
						enable_timer(1);
					}
					break;
				case 10: // gestione del led di movimento
					if (dist())
						LED_On(5);
				break;
				default:
					break;
			}
		}
		else {	/* button released */
			button = -1;
			down_run=0;	
			disable_timer(1);
			reset_timer(1);
			LED_Off(5);
			disable_RIT();
			reset_RIT();
			NVIC_EnableIRQ(EINT2_IRQn);							 /* disable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 24);     // EINT2
		}
	}
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
