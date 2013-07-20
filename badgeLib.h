//DONE: Scan from frame buffer
//DONE: Interupt based timing
//DONE: Replace Arduino digitalWrites with direct writes to ports (for speed!)
//digitalWrite code: about 10ms. Direct writes < 3ms!

#include <Arduino.h>

#define BUZZER A4            //PF1
#define COL_DATA 10          //PB6
#define COL_OUTPUT_ENABLE A1 //PF6
#define COL_STROBE A0        //PF7 Strobe basically STCP 
#define COL_CLOCK 13         //PC7 Clock input SHCP
#define COL_MEM_RESET 5      //PC6
#define ROW_DATA 9           //PB5
#define ROW_OUTPUT_ENABLE 8  //PB4
#define ROW_STROBE 6         //PD7
#define ROW_CLOCK 12         //PD6
#define ROW_MEM_RESET 4      //PD4
#define JOY_X A2             //PF5
#define JOY_Y A3             //PF4 maybe swapped with above, test it!
#define BATTERY_VOLTS A5     //PF0
#define BUTTON_TOP 7         //PE6
//#define BUTTON_BOTTOM      //PD5 No Arduino Pin
#define PORT5_SET         0b00100000

static int currentCol = 0;
const int selectPin = 2; 
//int colBit = 0;
//int rowBit = 0;

//frameBuffer[y][x]
int frameBuffer[16][16] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

void strobe()
{
  //COL_STROBE is PF7
  //ROW_STROBE is PD7
  PORTD = PORTD | 0b10000000; 
  PORTF = PORTF | 0b10000000; 
  PORTD = PORTD & 0b01111111;
  PORTF = PORTF & 0b01111111;

  //digitalWrite(ROW_STROBE, HIGH);
  //digitalWrite(COL_STROBE, HIGH);
  //digitalWrite(COL_STROBE, LOW);
  //digitalWrite(ROW_STROBE, LOW);
}

void writeCol(int data)
{
  for(int i = 0; i < 16; i++)
  {
    //COL_DATA is PB6
    PORTB = (PORTB & 0b10111111) | (((data >> i) & 1) << 6);
    //digitalWrite(COL_DATA, (data >> i) & 1);

    //COL_CLOCK is PC7
    PORTC = PORTC | 0b10000000;
    PORTC = PORTC & 0b01111111;
    //digitalWrite(COL_CLOCK, HIGH);
    //digitalWrite(COL_CLOCK, LOW);  
  } 
}

void writeRow(int data)
{
  for(int i = 0; i < 16; i++)
  {
    //ROW_DATA is PB5
    PORTB = (PORTB & 0b11011111) | (((data >> i) & 1) << 5);
    //digitalWrite(ROW_DATA, (data >> i) & 1);

    //ROW_CLOCK is PD6
    PORTD = PORTD | 0b01000000; 
    PORTD = PORTD & 0b10111111;    
    //digitalWrite(ROW_CLOCK, HIGH); 
    //digitalWrite(ROW_CLOCK, LOW);  
  } 
}

void writeRowBuffer(int col)
{
  for(int i = 0; i < 16; i++)
  {
    //ROW_DATA is PB5
    PORTB = (PORTB & 0b11011111) | (((frameBuffer[15-i][col]) & 1) << 5);
    //digitalWrite(ROW_DATA, (data >> i) & 1);

    //ROW_CLOCK is PD6
    PORTD = PORTD | 0b01000000; 
    PORTD = PORTD & 0b10111111;    
    //digitalWrite(ROW_CLOCK, HIGH); 
    //digitalWrite(ROW_CLOCK, LOW);  
  } 
}
void setupTimer()
{
  //http://letsmakerobots.com/node/28278  

  // initialize timer5 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;        //
  TCCR1B = 0;        //
  TCNT1  = 0;        //

  //OCR1A = 31250;            // compare match register 16MHz/256/2Hz //or 8Mhz/256/1hz or 8Mhz/256/1hz
  OCR1A = 20;            // compare match register  1ms
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_COMPA_vect)
{
  //pinMode(1, OUTPUT); //For Frame Rate measurement. Currently about 400FPS!
  //digitalWrite(1, HIGH);
  //for(int i = 0; i < 16; i++)
  //{
  currentCol++;
  if(currentCol > 15) currentCol = 0;

  writeCol((1 << currentCol) ^ 0b1111111111111111);
  writeRowBuffer(currentCol);//(frameBuffer[0]);
  strobe();
  //}
  //digitalWrite(1, LOW);
  //delayMicroseconds(10);
}


void setupBadge()
{
  pinMode(BUZZER, OUTPUT);
  pinMode(COL_DATA, OUTPUT);
  pinMode(COL_OUTPUT_ENABLE, OUTPUT);
  pinMode(COL_STROBE, OUTPUT);
  pinMode(COL_CLOCK, OUTPUT);
  pinMode(COL_MEM_RESET, OUTPUT);
  pinMode(ROW_DATA, OUTPUT);
  pinMode(ROW_OUTPUT_ENABLE, OUTPUT);
  pinMode(ROW_STROBE, OUTPUT);
  pinMode(ROW_CLOCK, OUTPUT);
  pinMode(ROW_MEM_RESET, OUTPUT);
  pinMode(BUTTON_TOP, INPUT);
  //digitalWrite(BUZZER, HIGH);
  digitalWrite(BUTTON_TOP, HIGH);

  //Joystick
  pinMode(selectPin, INPUT);  
  digitalWrite(selectPin, HIGH);  

  digitalWrite(COL_OUTPUT_ENABLE, LOW);
  digitalWrite(ROW_OUTPUT_ENABLE, LOW);
  digitalWrite(COL_MEM_RESET, HIGH);
  digitalWrite(ROW_MEM_RESET, HIGH);

  digitalWrite(COL_DATA, LOW);
  digitalWrite(COL_CLOCK, LOW);
  digitalWrite(ROW_DATA, LOW);
  digitalWrite(ROW_DATA, LOW);
  digitalWrite(BUZZER, LOW);

  setupTimer();
}






