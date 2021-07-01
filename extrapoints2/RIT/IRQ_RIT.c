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
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"
#include "../timer/timer.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

#define COLOR_E Yellow
#define COLOR_M Green
#define COLOR_O White
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
unsigned int distanza;

unsigned int win=1; // flag di vittoria
unsigned int wall; // flag che indica la presenza di un'uscita frontalmente

unsigned int mode; // 0 = explore, 1 = move

extern void clearMatrix();
	
void triangle(int x, int y, uint16_t color){
	int lines=0;
	int x0, x1, y0, y1;
	
	if (dir == 3){
		// nord
		
		x0 = x+3;
		x1 = x+13;
		y0 = y+13;
		
		while(x1 >= x0){
			if (lines == 2){
				x0++;
				x1--;
				lines = 0;
			} else {
				LCD_DrawLine(x0, y0, x1, y0, color);
				y0--;
				lines++;
			}
		}
	} else if (dir == 2){
		// est
		
		x0 = x+3;
		y0 = y+3;
		y1 = y+13;
		
		while(y1 >= y0){
			if (lines == 2){
				y0++;
				y1--;
				lines = 0;
			} else {
				LCD_DrawLine(x0, y0, x0, y1, color);
				x0++;
				lines++;
			}
		}		
	} else if (dir == 1){
		// sud
		
		x0 = x+3;
		x1 = x+13;
		y0 = y+3;
		
		while(x1 >= x0){
			if (lines == 2){
				x0++;
				x1--;
				lines = 0;
			} else {
				LCD_DrawLine(x0, y0, x1, y0, color);
				y0++;
				lines++;
			}
		}	
	} else {
		// ovest
		
		x0 = x+13;
		y0 = y+3;
		y1 = y+13;
		
		while(y1 >= y0){
			if (lines == 2){
				y0++;
				y1--;
				lines = 0;
			} else {
				LCD_DrawLine(x0, y0, x0, y1, color);
				x0--;
				lines++;
			}
		}	
	}
}

unsigned int dist(unsigned int new_dir){
	int i=0;
	wall = 0;
	
	if(new_dir==3){
		while ((labirinto[pos[0]-i-1][pos[1]] != 1) && ((pos[0]-i) > 0))
			i++;
		if ((labirinto[pos[0]-i-1][pos[1]]) == 1)
				wall = 1;
	} else if(new_dir==2){
		while ((labirinto[pos[0]][pos[1]+i+1] != 1) && ((pos[1]+i) < (C-1)))
			i++;
		if ((labirinto[pos[0]][pos[1]+i+1]) == 1)
			wall = 1;
	} else if(new_dir==1){
		while ((labirinto[pos[0]+i+1][pos[1]] != 1) && ((pos[0]+i) < (R-1)))
			i++;
		if ((labirinto[pos[0]+i+1][pos[1]]) == 1)
			wall = 1;
	} else {
		while ((labirinto[pos[0]][pos[1]-i-1] != 1) && ((pos[1]-i) > 0))
			i++;
		if ((labirinto[pos[0]][pos[1]-i-1]) == 1)
			wall = 1;
	}
	
	return i;
}
int check(void){
	if (labirinto[pos[0]][pos[1]] == 2){
		// setto la variabile di vittoria
		win = 1;
		
		clearMatrix();
		GUI_Text(88, 150, (uint8_t *) "YOU WIN!", Green, Black);
	}
	
	return win;
}
void matrice(void){
	int i;
	
	// orizzontali
	for (i=0; i<=(R*16); i+=16){
		if(i!=R*16)
			LCD_DrawLine(0, 56+i, 240, 56+i, White);
		if(i)
			LCD_DrawLine(0, 56+i-1, 240, 56+i-1, White);			
	}
	// verticali
	for (i=0; i<=(C*16); i+=16){
		LCD_DrawLine(i, 56, i, 263, White);
		if(i)
			LCD_DrawLine(i-1, 56, i-1, 263, White);
	}
}
void start(void){
	int i;
	
	// riporto il robot nella posizione iniziale
	pos[0] = 7;
	pos[1] = 7;
	dir = 2; // E
	mode = 1; // move
		
	// riporto il flag di vittoria a zero
	win = 0;
	
	// disegna la schermata iniziale
	LCD_Clear(Black);
	
	for (i=0; i<4; i++)
		GUI_Text(30, 40*(i+1), (uint8_t *) "LOADING...", Green, Black);
	
	GUI_Text(98, 240, (uint8_t *) "TOUCH TO START", Green, Black);
	
	init_timer(0, 0x4E2 ); 						    /* 500us * 25MHz = 1.25*10^3 = 0x4E2 */

	enable_timer(0);
	
}



void obst(){
	int i;
	
	if (wall && distanza<=5 && dir==3){
			for (i=1; i<15; i++)
				LCD_DrawLine(16*pos[1]+1, 56+16*(pos[0]-distanza-1)+i, 16*pos[1]+14, 56+16*(pos[0]-distanza-1)+i, COLOR_O);
	} 
	else if (wall && distanza<=5 && dir==2){
		for (i=1; i<15; i++)
			LCD_DrawLine(16*(pos[1]+distanza+1)+1, 56+16*pos[0]+i, 16*(pos[1]+distanza+1)+14, 56+16*pos[0]+i, COLOR_O);
	} 
	else if (wall && distanza<=5 && dir==1){
		for (i=1; i<15; i++)
			LCD_DrawLine(16*pos[1]+1, 56+16*(pos[0]+distanza+1)+i, 16*pos[1]+14, 56+16*(pos[0]+distanza+1)+i, COLOR_O);
	} 
	else if (wall && distanza<=5 && dir==0){
		for (i=1; i<15; i++)
			LCD_DrawLine(16*(pos[1]-distanza-1)+1, 56+16*pos[0]+i, 16*(pos[1]-distanza-1)+14, 56+16*pos[0]+i, COLOR_O);
	} 
}

/******************************************************************************/

void RIT_IRQHandler (void)
{					
	static int select=0; // pin 25
	static int down=0; // pin 26
	static int left=0; // pin 27
	static int right=0; // pin 28
	static int up=0; // pin 29
	static int position=0;	
	static int i;
	
	if (!win){
		// select 
		if((LPC_GPIO1->FIOPIN & (1<<25)) == 0){	
			select++;
			//reset_RIT();
			switch(select){
				case 1:
					mode = (mode) ? 0 : 1;
					triangle(16*pos[1], 56+16*pos[0], (!mode) ? COLOR_E : COLOR_M);
					break;
				default:
					break;
			}
		}
		else{
				select=0;
		}
		
		// down
		if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	
			down++;
			switch(down){
				case 1:
					distanza = dist(1);
				
					if (!mode) {
						// se sono in explore, coloro eventuali ostacoli
						triangle(16*pos[1], 56+16*pos[0], Black);
						dir = 1;
						triangle(16*pos[1], 56+16*pos[0], COLOR_E);		
						obst();
					} else {
						if (dir!=1){
							// sono in move e la direzione è cambiata, ruoto il robot
							triangle(16*pos[1], 56+16*pos[0], Black);
							dir = 1;
							triangle(16*pos[1], 56+16*pos[0], COLOR_M);							
						} else {
							// sono in move e la direzione non cambia, se posso avanzo
							if (distanza){
								triangle(16*pos[1], 56+16*pos[0], Black);
								pos[0]++;
								triangle(16*pos[1], 56+16*pos[0], COLOR_M);
							}
						}
					}
					break;
				default:
					break;
			}
		}
		else{
				down=0;
		}
		
		// left
		if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	
			left++;
			switch(left){
				case 1:
					distanza = dist(0);
				
					if (!mode) {
						// se sono in explore e la direzione è cambiata allora coloro eventuali ostacoli
						triangle(16*pos[1], 56+16*pos[0], Black);
						dir = 0;
						triangle(16*pos[1], 56+16*pos[0], COLOR_E);		
						obst();
					} else {
						if (dir!=0){
							// sono in move e la direzione è cambiata, ruoto il robot
							triangle(16*pos[1], 56+16*pos[0], Black);
							dir = 0;
							triangle(16*pos[1], 56+16*pos[0], COLOR_M);							
						} else {
							// sono in move e la direzione non cambia, se posso avanzo
							if (distanza){
								triangle(16*pos[1], 56+16*pos[0], Black);
								pos[1]--;
								triangle(16*pos[1], 56+16*pos[0], COLOR_M);
							}
						}
					}
					break;
				default:
					break;
			}
		}
		else{
				left=0;
		}
		
		// right
		if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){	
			right++;
			switch(right){
				case 1:
					distanza = dist(2);
				
					if (!mode) {
						// se sono in explore e la direzione è cambiata allora coloro eventuali ostacoli
						triangle(16*pos[1], 56+16*pos[0], Black);
						dir = 2;
						triangle(16*pos[1], 56+16*pos[0], COLOR_E);		
						obst();
					} else {
						if (dir!=2){
							// sono in move e la direzione è cambiata, ruoto il robot
							triangle(16*pos[1], 56+16*pos[0], Black);
							dir = 2;
							triangle(16*pos[1], 56+16*pos[0], COLOR_M);							
						} else {
							// sono in move e la direzione non cambia, se posso avanzo
							if (distanza){
								triangle(16*pos[1], 56+16*pos[0], Black);
								pos[1]++;
								triangle(16*pos[1], 56+16*pos[0], COLOR_M);
							}
						}
					}
					break;
				default:
					break;
			}
		}
		else{
				right=0;
		}
		
		// up
		if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){	
			up++;
			switch(up){
				case 1:
					distanza = dist(3);
				
					if (!mode) {
						// se sono in explore e la direzione è cambiata allora coloro eventuali ostacoli
						triangle(16*pos[1], 56+16*pos[0], Black);
						dir = 3;
						triangle(16*pos[1], 56+16*pos[0], COLOR_E);		
						obst();
					} else {
						if (dir!=3){
							// sono in move e la direzione è cambiata, ruoto il robot
							triangle(16*pos[1], 56+16*pos[0], Black);
							dir = 3;
							triangle(16*pos[1], 56+16*pos[0], COLOR_M);							
						} else {
							// sono in move e la direzione non cambia, se posso avanzo
							if (distanza){
								triangle(16*pos[1], 56+16*pos[0], Black);
								pos[0]--;
								triangle(16*pos[1], 56+16*pos[0], COLOR_M);
							}
						}
					}
					break;
				default:
					break;
			}
		}
		else{
				up=0;
		}
		
		if(mode && distanza)
			check();
		
		disable_RIT();
		reset_RIT();
		enable_RIT();
	}
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
