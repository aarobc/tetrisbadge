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
int piece[5][2] = {{3,0},
                   {4,0},
                   {5,0},
                   {6,0},
                   {4,1}};

int lastPiece[5][2] = {0};

const int selectPin = 2;  
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


void generatePiece(){
  checkLine();
  delay(10);
  int wat = rand() % 5; 
//int wat = 3;
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
      // rotation center point
      piece[4][1] = 0;  
      piece[4][0] = 5;  
      break;
    case 2:
      // line piece
      for(int p = 0; p < 4; p++){
        piece[p][1] = -1;  
        piece[p][0] = p + 4;  
      }
      // rotation center point
      piece[4][1] = 0;  
      piece[4][0] = 5;  
      break;
    case 3:
      // square block
      piece[0][1] = -1;  
      piece[0][0] =  4;  

      piece[1][1] = -1;  
      piece[1][0] =  5;  

      piece[2][1] = 0;  
      piece[2][0] = 4;  

      piece[3][1] = 0;  
      piece[3][0] = 5;  
      // rotation center point
      piece[4][1] = 0;  
      piece[4][0] = 5;  
      break;
    case 4:
      // square block
      piece[0][1] = -1;  
      piece[0][0] =  5;  

      piece[1][1] = -1;  
      piece[1][0] =  6;  

      piece[2][1] = -1;  
      piece[2][0] = 7;  

      piece[3][1] = 0;  
      piece[3][0] = 7;  
      // rotation center point
      piece[4][1] = 0;  
      piece[4][0] = 6;  
      break;
    default:
      break;
  }
}

void copyPiece(int source[][2], int dest[][2]){
  for(int p = 0; p < 5; p++){
    dest[p][0] = source[p][0];  
    dest[p][1] = source[p][1];  
  }
}

void checkLine(){

  for(int i = 0; i < 16; i++){
    bool line = true;
    for(int p = 0; p < 16; p++){

      if(frameBuffer[i][p] == 0){
        line = false; 
      }
    }
      // if there is a complete line 
      if(line){
        animateLine(i);
        moveDown(i);
        
      }


    // TODO: if there is a complete line, animate it, then remove it, and increment the buffer
  }
}


void moveDown(int address){
    for(int y = address; y > 0; y--){
  for(int x = 0; x < 16; x++){
      frameBuffer[y][x] = frameBuffer[y - 1][x];
      // Serial.print(frameBuffer[y][x]);
    }  
    // Serial.println();
  }
}

void animateLine(int line){

  for(int o = 0; o < 16; o++){
    frameBuffer[line][o] = 0;
    //frameBuffer[line][o] = 1;
  }
  delay(300);
  for(int p = 0; p < 16; p++){
    frameBuffer[line][p] = 1;
    //frameBuffer[line][o] = 1;
  }
  delay(300);
  for(int o = 0; o < 16; o++){
    frameBuffer[line][o] = 0;
    //frameBuffer[line][o] = 1;
  }
}

bool rotatePiece(){

  copyPiece(piece, lastPiece);
  // storing the axis of rotation in the 5th spot of the piece array
  int offset_x = lastPiece[4][0];
  int offset_y = lastPiece[4][1];

  for(int p = 0; p < 4; p++){
    //
    int X = lastPiece[p][0] - offset_x;
    int Y = lastPiece[p][1] - offset_y;

    // preform the translation
    piece[p][0] = (Y * -1) + offset_x;
    piece[p][1] = X + offset_y;
  }
  writePiece();


}

int movePiece(int x, int y){
  if(x != 0){
    for(int p = 0; p < 4; p++){
      // check to make sure it's not off the edge of the screen
      if(piece[p][0] + x > 15) 
        return 0;

      if(piece[p][0] + x < 0) 
        return 0;

    }
    moveXY(x, 0);
  }

  if(y != 0){
    // check if at the bottom of the screen
    for(int p = 0; p < 4; p++){
      if(piece[p][1] > 14){
        generatePiece();
        return 0;
      }
    }
    moveXY(1, 1);

  }

}


int moveXY(int move, int axis){

  
  //  record the current position 
  copyPiece(piece, lastPiece);

  // flag variables to check 
  bool flict = false;
  bool saveMove = true;
  // increment the axis of rotation
  piece[4][axis] += move;
  // increment the address of the piece 
  for(int p = 0; p < 4; p++){
    piece[p][axis] += move;

    // check if it is on top of another piece 
    bool same = false; 
    // compair the last position to remove the entries from the buffer check
    for(int r = 0; r < 4; r++){
      if((lastPiece[r][0] == piece[p][0]) && (lastPiece[r][1] == piece[p][1])){ 
        same = true;
       // break;
      }
    }    
    // check against the buffer
    if(!same && frameBuffer[piece[p][1]][piece[p][0]]){
      // reset the variable back to it's original value
      if(axis){
         generatePiece();       
          return 0;
      }
      copyPiece(lastPiece, piece);
      return 0;
    }
  }
  writePiece();
}

void writePiece(){

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


bool topBP = false;
//static long lastStick millis();
void ohJoy(){

  int joystick_x = map(analogRead(JOY_X), 250, 700, 3, 0);  
  int joystick_y = map(analogRead(JOY_Y), 300, 800, 0, 3);  

  bool topB = digitalRead(BUTTON_TOP);   

  if(oldX != joystick_x){
    if(joystick_x == 3){
      movePiece(1, 0);
    }

    if(joystick_x == 0){
      movePiece(-1, 0);
    }
    oldX = joystick_x;  
  }
  
  if(topB != topBP){
    if(!topB){
      Serial.println(topB);
      rotatePiece();
    }

    topBP = topB;

  }

}


void loop()
{
  static long lastTime = millis();
  ohJoy();
  if(millis() > lastTime + 200)
  {
    lastTime = millis();
  //  drop();
   movePiece(0, 1); 
  }

}
