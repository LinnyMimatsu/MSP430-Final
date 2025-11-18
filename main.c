/*
Name: Dom
Description: Pocket Simon using theory learned in class for the MSP-EXP430G2ET microcontroller
 */

#include <msp430.h>
#include <stdlib.h>

volatile unsigned char timer_counter = 0;
volatile unsigned char last_button = 0;

/*Hard coded length of the game, can be reduced or lengthened*/
#define MAX_SEQUENCE 10
unsigned char sequence[MAX_SEQUENCE];
unsigned char sequence_length = 0;
unsigned char player_step = 0;

#define DELAY_BEFORE_THE_PLAYER_FAILS_FOR_NOT_PRESSING_A_BUTTON_MEASURED_IN_SECONDS 5

int main(void)
{
    volatile unsigned int i;
    WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer

    /*setting up the LEDS for the Simon game*/
    P2DIR |= (BIT0 | BIT1 | BIT2 | BIT3);
    P2OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
    P1DIR |= (BIT0 | BIT6);
    P1OUT &= ~(BIT0 | BIT6);



    /*Setting up the buttons for the simon game*/
    P1DIR &= ~(BIT1 | BIT2 | BIT4 | BIT5);
    P1OUT |= (BIT1 | BIT2 | BIT4 | BIT5);
    P1REN |= (BIT1 | BIT2 | BIT4 | BIT5);

    /*timer so the sequence is random everytime the user restarts or the code is ran*/
    TA0CTL = TASSEL_2 | MC_2 | TACLR;
    srand(TA0R);

    TA1CCR0 = 12500;        
    TA1CCTL0 = CCIE;            
    TA1CTL = TASSEL_2 | ID_3; 

    /*Enabling the interrupts for the buttons*/
    P1IE |= (BIT1 | BIT2 | BIT4 | BIT5);
    P1IES |= (BIT1 | BIT2 | BIT4 | BIT5);
    P1IFG &= ~(BIT1 | BIT2 | BIT4 | BIT5);

    __bis_SR_register(GIE);

    add_random_step();
    play_sequence();

    while(1)
    {
      
      check_player_input();

   
  }
}

/*interrupt reading the button presses by the user*/
#pragma vector=PORT1_VECTOR
__interrupt void P1_ISR(void) {

  unsigned char flags = P1IFG & (BIT1 | BIT2 | BIT4 | BIT5);

  switch(flags) {

    case BIT1:
    last_button = BIT1;
    break;

    case BIT2:
    last_button = BIT2;
    break;

    case BIT4:
    last_button = BIT4;
    break;

    case BIT5:
    last_button = BIT5;
    break;

    default:
    break;

  }

  /*clear interrupt*/
  P1IFG &= ~(BIT1 | BIT2 | BIT4 | BIT5);

}

/*using a timer, reset the game if the user takes too long*/
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void) {

  if(++timer_counter == (10 * DELAY_BEFORE_THE_PLAYER_FAILS_FOR_NOT_PRESSING_A_BUTTON_MEASURED_IN_SECONDS)){
    TA1CTL &= ~MC0;
    TA1CTL |= TACLR;
    timer_counter = 0;

    sequence_length = 0;
    player_step = 0;
    flash_board_led(BIT6);
    add_random_step();
    play_sequence();


  }
}

/*checks inputs to leds*/
void check_player_input() {

  if(last_button == 0) return;
  
  TA1CTL &= ~MC0;
  TA1CTL |= TACLR;
  timer_counter = 0;

  unsigned char pressed;
  switch(last_button) {
    case BIT1:
    pressed = 0;
    break;

    case BIT2:
    pressed = 1;
    break;

    case BIT4:
    pressed = 2;
    break;

    case BIT5:
    pressed = 3;
    break;

    default:
    return;
  }

  // if user input equals sequence continue
  if(pressed == sequence[player_step]) {
    player_step++;


  if(player_step == sequence_length) {
    flash_board_led(BIT0);
    player_step = 0;
    add_random_step();
    play_sequence();
  }else{

  TA1CTL |= MC0;
  
  }

  


  // if user messes up restart
  } else {
      TA1CTL &= ~MC0;
      TA1CTL |= TACLR;
      timer_counter = 0;

    sequence_length = 0;
    flash_board_led(BIT6);
    player_step = 0;
    add_random_step();
    play_sequence();

  }

  last_button = 0;

}

/*if the user gets the sequence right add to it
but dont go above specificed maximum*/
void add_random_step(){

if(sequence_length < MAX_SEQUENCE) {
  sequence[sequence_length] = rand() % 4;
  sequence_length++;


/*Celebratory flashing and reset the sequence*/
}else{

      TA1CTL &= ~MC0;
      TA1CTL |= TACLR;
      timer_counter = 0;
      unsigned char i;
      for (i = 0; i <= 5; i++){
      P2OUT |= (BIT0 | BIT1 | BIT2 | BIT3);
      
      __delay_cycles(300000);

      P2OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

      __delay_cycles(200000);
      }

      sequence_length = 0;
      player_step = 0;
      add_random_step();
    
}

}

/*function to flash the leds in sequence*/
void play_sequence() {
  unsigned char i;
  for(i = 0; i < sequence_length; i++) {
    flash_led(sequence[i]);
  }

  TA1CTL |= TACLR;
  timer_counter = 0;
  TA1CTL |= MC0;

}


/*function that actually does the flashing*/
void flash_led(unsigned char led) {

switch(led) {

case 0:
P2OUT |= BIT0;
break;

case 1:
P2OUT |= BIT1;
break;

case 2:
P2OUT |= BIT2;
break;

case 3:
P2OUT |= BIT3;
break;

}

/*delays so the user can see the led flashes
and then turn off the leds*/
__delay_cycles(300000);

P2OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

__delay_cycles(100000);


}


/*if the user gets the sequence right flash green, if not
flash red on the board*/
void flash_board_led(unsigned char led) {
  P1OUT |= led;
  __delay_cycles(500000);
  P1OUT &= ~led;
  __delay_cycles(200000);
}
