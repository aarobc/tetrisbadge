//DONE: Scan from frame buffer
//DONE: Interupt based timing
//DONE: Replace Arduino digitalWrites with direct writes to ports (for speed!)
//digitalWrite code: about 10ms. Direct writes < 3ms!



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

static int currentCol = 0;
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


//piece[x][y]
int piece[4][2] = {{3,0},
  {4,0},
  {5,0},
  {6,0}};

int lastPiece[4][2] = {0};

const int selectPin = 2;  
const int joystick_xPin = A2;  
const int joystick_yPin = A3;  
int oldX = 0;  
int oldY = 0;  
int oldSelect = 0;

void setup()
{
  Serial.begin(9600);
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

  //digitalWrite(BUZZER, HIGH);


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

bool top = true;

void generatePiece(){
  int wat = rand() % 4; 
//int wat = 1;
  switch(wat){
    // angle piece 
    case 1:
      piece[0][1] = -1;  
      piece[0][0] =  4;  

      piece[1][1] = -1;  
      piece[1][0] =  5;  

      piece[2][1] = 0;  
      piece[2][0] = 5;  

      piece[3][1] = 0;  
      piece[3][0] = 6;  
      break;
    case 2:
      // line piece
      for(int p = 0; p < 4; p++){
        piece[p][1] = -1;  
        piece[p][0] = p + 4;  
      }
      break;
    case 3:
      piece[0][1] = -1;  
      piece[0][0] =  4;  

      piece[1][1] = -1;  
      piece[1][0] =  5;  

      piece[2][1] = 0;  
      piece[2][0] = 4;  

      piece[3][1] = 0;  
      piece[3][0] = 5;  
      break;
    default:
      break;
  }


  top = true;
}

void checkLine(){

  for(int i = 0; i < 16; i++){
    bool line = true;
    for(int p = 0; p < 16; p++){

      if(!frameBuffer[i][p]){
       line = false; 
      }
      // TODO: if there is a complete line, animate it, then remove it, and increment the buffer
    }
  }

}

int movePiece(int x, int y){
  Serial.println(x);
  moveX(x);

}

int moveX(int move){

  for(int p = 0; p < 4; p++){
    // check to make sure it's not off the edge of the screen
    if(piece[p][0] + move > 15) 
      return 0;

    if(piece[p][0] + move < 0) 
      return 0;
    
  }

  //  record the current position 
  for(int p = 0; p < 4; p++){
    lastPiece[p][0] = piece[p][0];  
    lastPiece[p][1] = piece[p][1];  
  }
  // variable to check 
  bool flict = false;
  // increment the address of the piece
  for(int p = 0; p < 4; p++){
    piece[p][0] += move;


    // check if it is on top of another piece 
    bool same = false; 
    // compair the last position to remove the entries from the buffer check
    for(int r = 0; r < 4; r++){
      if((lastPiece[r][0] == piece[p][0]) && (lastPiece[r][1] == piece[p][1])){ 
        same = true;
      }
    }    
    // check against the buffer
    if(!same && frameBuffer[piece[p][1]][piece[p][0]]){
      return 0;
    }
  }

  // y, x 
  for(int p = 0; p < 4; p++){
    bool flict = false;
    frameBuffer[piece[p][1]][piece[p][0]] = 1;  

    // checking for conflict 
    for(int o = 0; o < 4; o++){
      if(((piece[o][1] == lastPiece[p][1]) && (piece[o][0] == lastPiece[p][0]))){
        flict = true;
      }
    }
    // if there is no conflict then make the change
    if(!flict){
      frameBuffer[lastPiece[p][1]][lastPiece[p][0]] = 0;  
    }
  }
}

int drop(){

  //  record the current position 
  for(int p = 0; p < 4; p++){
    lastPiece[p][0] = piece[p][0];  
    lastPiece[p][1] = piece[p][1];  
  }
  // variable to check 
  bool flict = false;
  // increment the address of the piece
  for(int p = 0; p < 4; p++){
    piece[p][1] += 1;  

    // check if at the bottom of the screen
    if(piece[p][1] == 16){
      generatePiece();
      return 0;
    }

    // check if it is on top of another piece 
   bool same = false; 
   // compair the last position to remove the entries from the buffer check
  for(int r = 0; r < 4; r++){
    if((lastPiece[r][0] == piece[p][0]) && (lastPiece[r][1] == piece[p][1])){ 
      same = true;
    }
  }    
    // check against the buffer
    // make sure that the new position doesn't interfere with the existing blocks
    if(!same && frameBuffer[piece[p][1]][piece[p][0]]){
      return 0;
    }
  }
  
  // write the new position to the buffer
  for(int p = 0; p < 4; p++){
    bool flict = false;
    frameBuffer[piece[p][1]][piece[p][0]] = 1;  

    // clearing the top after the move
    // checking for conflict with the new position of the piece 
    for(int o = 0; o < 4; o++){
      if(((piece[o][1] == lastPiece[p][1]) && (piece[o][0] == lastPiece[p][0]))){
        flict = true;
      }
    }
    // if there is no conflict then make the change
    if(!flict){
      frameBuffer[lastPiece[p][1]][lastPiece[p][0]] = 0;  
    }
  }
}

int joystick_x;  
int joystick_y;  
int select;  

void ohJoy(){

  joystick_x = map(analogRead(joystick_xPin), 250, 700, 3, 0);  
  joystick_y = map(analogRead(joystick_yPin), 300, 800, 0, 3);  

  if(oldX != joystick_x){
     if(joystick_x == 3){
      movePiece(1, 0);
     }

     if(joystick_x == 0){
      movePiece(-1, 0);
     }
    oldX = joystick_x;  
  }
}


void loop()
{
  static long lastTime = millis();
  ohJoy();
  if(millis() > lastTime + 300)
  {
    lastTime = millis();
    drop();
  }

}
