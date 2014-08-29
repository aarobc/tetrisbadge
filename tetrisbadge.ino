
#include "dc21_badge.h"

#define L_WALL 6
#define DROP_LOC 9
#define STRT_X 1 
#define STRT_Y 2
#define SIDE_SPEED 85
#define P_BRIGHT 1
// initializing the first piece
int piece[5][2] = {0};
int strtPiece[5][2] = {0};

int lastPiece[5][2] = {0};
int scoreSpeed = 1000;

int totScore = 0;

unsigned int onField =  0b0000001111111111;
unsigned int offField = 0b1111110000000000;

// game over array
char gOver[16][16] ={{1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
                     {0,1,1,0,1,0,1,1,1,1,1,1,1,1,0,0},
                     {0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1},
                     {0,1,0,0,0,1,0,1,0,0,1,0,0,1,0,0},
                     {0,1,1,0,0,0,0,1,0,1,0,1,0,1,0,1},
                     {1,0,0,1,0,1,0,1,0,1,1,1,0,1,0,0},
                     {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                     {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                     {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                     {0,1,0,1,0,1,1,1,0,1,0,0,1,0,0,1},
                     {0,1,0,1,0,1,1,1,0,1,0,1,1,0,1,0},
                     {0,1,0,1,1,0,1,0,1,1,0,0,1,0,0,1},
                     {0,1,0,1,1,0,1,0,1,1,0,1,1,0,0,1},
                     {1,0,1,1,1,1,0,1,1,1,0,0,1,0,1,0},
                     {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                     {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};


DC21_badge badge;


// Initialize Stuffs. Mostly the timer interupt. Badge hardware is initialized in the badge library.
void setup() {
  // TIMER1 SETUP
  noInterrupts();				// disable all interrupts
  TCCR1A = 0;					// set entire register to 0
  TCCR1B = 0;					// set entire register to 0
  TCNT1  = 0;					// reset the 16-bit timer counter
  OCR1A = 100;				// compare match register
  TCCR1B |= (1 << WGM12);		// CTC mode (Clear timer on compare match)
  TCCR1B |= (1 << CS11);		// 8 prescaler 
  TIMSK1 |= (1 << OCIE1A);	// enable timer compare interrupt
  interrupts();
 badge.grayscale = false;	
  badge.clear();
 gameBoard();

 generatePiece();
  nextPiece();
}

int bright(int in){
  return (in) ? P_BRIGHT : 0;
}
void gameBoard(){
  
  for(int y = 0; y < 16; y++){
    char br = (y % 2) ? 1 : 0;
    badge.setPixel(L_WALL - 1, y, br);
  }

}

// to imrove the randomness
int rhist[2] = {8, 10};

void generatePiece(){

  int wat;
  do{
    wat = rand() % 7; 
  }while(wat == rhist[0] || wat == rhist[1]);
  // scoot the number down
  rhist[1] = rhist[0];
  rhist[0] = wat;
    
 // int wat = 1;
  switch(wat){
  // Z piece 
    case 0:
      strtPiece[0][1] = STRT_Y;  
      strtPiece[0][0] = STRT_X;  

      strtPiece[1][1] = STRT_Y;  
      strtPiece[1][0] = STRT_X + 1;  

      strtPiece[2][1] =  STRT_Y + 1;  
      strtPiece[2][0] = STRT_X + 1;  

      strtPiece[3][1] = STRT_Y + 1;  
      strtPiece[3][0] = STRT_X + 2;  
      // rotation center point
      strtPiece[4][1] = STRT_Y + 1;  
      strtPiece[4][0] = STRT_X + 1;  
      break;
    case 1:
      // line straight piece
      for(int p = 0; p < 4; p++){
        strtPiece[p][1] = STRT_Y;  
        strtPiece[p][0] = STRT_X + p - 1;  
      }
      // rotation center point
      strtPiece[4][1] = STRT_Y;  
      strtPiece[4][0] = STRT_X + 1;  
      break;
    case 2:
      // square block
      strtPiece[0][1] = STRT_Y;  
      strtPiece[0][0] = STRT_X;  

      strtPiece[1][1] = STRT_Y;  
      strtPiece[1][0] = STRT_X + 1;  

      strtPiece[2][1] = STRT_Y + 1;  
      strtPiece[2][0] = STRT_X;  

      strtPiece[3][1] = STRT_Y + 1;  
      strtPiece[3][0] = STRT_X + 1;  
      // rotation center point
      strtPiece[4][1] = STRT_Y + 1;  
      strtPiece[4][0] = STRT_X;  
      break;
    case 3:
      // L strtPiece
      strtPiece[0][1] = STRT_Y;  
      strtPiece[0][0] = STRT_X;  

      strtPiece[1][1] = STRT_Y;  
      strtPiece[1][0] = STRT_X + 1;  

      strtPiece[2][1] = STRT_Y;  
      strtPiece[2][0] = STRT_X + 2;  

      strtPiece[3][1] = STRT_Y + 1;  
      strtPiece[3][0] = STRT_X + 2;  
      // rotation center point
      strtPiece[4][1] = STRT_Y + 1;  
      strtPiece[4][0] = STRT_X + 1;  
      break;
    case 4:
      // Backwards L strtPiece
      strtPiece[0][1] = STRT_Y + 1;  
      strtPiece[0][0] = STRT_X;  

      strtPiece[1][1] = STRT_Y + 1;  
      strtPiece[1][0] = STRT_X + 1;  

      strtPiece[2][1] = STRT_Y + 1;  
      strtPiece[2][0] = STRT_X + 2;  

      strtPiece[3][1] = STRT_Y;  
      strtPiece[3][0] = STRT_X + 2;  
      // rotation center point
      strtPiece[4][1] = STRT_Y;  
      strtPiece[4][0] = STRT_X + 1;  
      break;
      // angle strtPiece 
    case 5:
      strtPiece[0][1] = STRT_Y + 1;  
      strtPiece[0][0] = STRT_X;  

      strtPiece[1][1] = STRT_Y + 1;  
      strtPiece[1][0] = STRT_X + 1;  

      strtPiece[2][1] = STRT_Y;  
      strtPiece[2][0] = STRT_X + 1;  

      strtPiece[3][1] = STRT_Y;  
      strtPiece[3][0] = STRT_X + 2;  
      // rotation center point
      strtPiece[4][1] = STRT_Y;  
      strtPiece[4][0] = STRT_X + 1; 
      break;
    case 6:
      strtPiece[0][1] = STRT_Y;  
      strtPiece[0][0] = STRT_X;  

      strtPiece[1][1] = STRT_Y;  
      strtPiece[1][0] = STRT_X + 1;  

      strtPiece[2][1] = STRT_Y;  
      strtPiece[2][0] = STRT_X + 2;  

      strtPiece[3][1] = STRT_Y + 1;  
      strtPiece[3][0] = STRT_X + 1;  
      // rotation center point
      strtPiece[4][1] = STRT_Y;  
      strtPiece[4][0] = STRT_X + 1; 
    default:
      break;
  }

}

void nextPiece(){
  
  checkLine();
// copy and translate the next piece
  for(int i = 0; i < 5; i++){
    piece[i][0] = strtPiece[i][0] + L_WALL + 3;
    piece[i][1] = strtPiece[i][1] - 3;

  }

  generatePiece();   
  // clearing out the preview area
  for(int i = 0; i < 4; i++){
    badge.frameBuffer[i][0] &= 0b0000011111111111;
  }
  // drawing the new piece
  for(int i = 0; i < 4; i++){
    badge.setPixel(strtPiece[i][0], strtPiece[i][1], bright(1));
  }
  
}

void score(){
  // decrease the wait time for falling each time
  scoreSpeed = (scoreSpeed < 200) ? scoreSpeed -1 : scoreSpeed - 3;
  // draw in bianary the number of lines cleared
  int one = totScore % 10;
  int ten = (totScore / 10) % 10;
  int hund = totScore / 100;
  //
  // ones place score
  for(int y = 15; y > 4; y--){
    if(one & 0b00000001)
      badge.setPixel(3, y, bright(1));
    else
      badge.setPixel(3, y, 0);
    one >>= 1;
    // tens score
    if(ten & 0b00000001)
      badge.setPixel(2, y, bright(1));
    else
      badge.setPixel(2, y, 0);
    ten >>= 1;
    // the hundreds
    if(hund & 0b00000001)
      badge.setPixel(1, y, bright(1));
    else
      badge.setPixel(1, y, 0);
    hund >>= 1;
      
  }
}

void gameOver(){


  for(int x = 0; x < 16; x++){

    for(int y = 0; y < 16; y++){
      badge.setPixel(x, y, bright(!gOver[y][x])); 
    }
  }
  delay(1000);
  badge.clear();
  score();
  // keeping it here until you press a button
  do{
    delay(100);
  }while(!badge.button2_debounced());

  totScore = 0;
  // clearing the frame buffer
  scoreSpeed = 1000;
  badge.clear();
  gameBoard();
}

void copyPiece(int source[][2], int dest[][2]){
  for(int p = 0; p < 5; p++){
    dest[p][0] = source[p][0];  
    dest[p][1] = source[p][1];  
  }
}

void checkLine(){

  // checking if top is done
  if(badge.frameBuffer[0][0] & onField){
    gameOver();
  }

  for(int i = 0; i < 16; i++){

    if((badge.frameBuffer[i][0] & onField) == onField){
      animateLine(i);
      moveDown(i);
      totScore++;
      score();
    }
  }
}


void moveDown(int address){
  for(int y = address; y > 0; y--){
   // badge.frameBuffer[y][0] = badge.frameBuffer[y - 1][0];
   unsigned int tmpP = badge.frameBuffer[y - 1][0] & onField;
   unsigned int tmpB = badge.frameBuffer[y][0] & offField;

   badge.frameBuffer[y][0] = tmpP | tmpB; 
  }
}

void dropPiece(){

  int check = 0; 
  copyPiece(piece, lastPiece);
  do{  
    check = movePiece(0, 1); 
    delay(10);
  }while(check != 42);
}

void animateLine(int line){
  badge.frameBuffer[line][0] &= offField;
  delay(300);
  badge.frameBuffer[line][0] |= onField;
  delay(300);
  badge.frameBuffer[line][0] &= offField;
}

bool rotatePiece(){

  copyPiece(piece, lastPiece);
  // storing the axis of rotation in the 5th spot of the piece array
  int offset_x = lastPiece[4][0];
  int offset_y = lastPiece[4][1];
  int wall = 0;
  for(int p = 0; p < 4; p++){
    //
    int X = lastPiece[p][0] - offset_x;
    int Y = lastPiece[p][1] - offset_y;

    // preform the translation
    piece[p][0] = (Y * -1) + offset_x;
    piece[p][1] = X + offset_y;

    wall = (piece[p][0] < L_WALL) ? wall + 1 : wall;  
    wall = (piece[p][0] > 15) ? wall - 1 : wall;  
  }
  // if near the wall, do this to scoot it over 
  if(wall != 0){
    for(int i = 0; i < 4; i++){
      piece[i][0] += wall;
    }
  }
  
  // This might cause problems and may need to be heavily modified
  // check if it conflicts with existing pieces
  for(int p = 0; p < 4; p++){
    bool same = false;
    for(int l = 0; l < 4; l++){
      // remove the current location from checking
      if((piece[p][0] == lastPiece[l][0]) && (piece[p][1] == lastPiece[l][1]))
       same = true; 
    }
    // aborting the change if it overlaps
    if(!same && badge.getPixel(piece[p][0], piece[p][1])){
      copyPiece(lastPiece, piece);
      return 0;
    }
  }

  writePiece();


}

int movePiece(int x, int y){
  // if x is not zero, it indicates you're to move the x axis
  if(x != 0){
    for(int p = 0; p < 4; p++){
      // check to make sure it's not off the edge of the screen
      if(piece[p][0] + x > 15) 
        return 0;

      if(piece[p][0] + x < L_WALL) 
        return 0;

    }
    // I'm not proud of this
    return moveXY(x, 0);
  }

  if(y != 0){
    // check if at the bottom of the screen
    for(int p = 0; p < 4; p++){
      if(piece[p][1] > 14){

        nextPiece();
        return 42;
      }
    }
    // I'll fix it eventually i swear
    return moveXY(1, 1);
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
    // compare the last position to remove the entries from the buffer check
    for(int r = 0; r < 4; r++){
      if((lastPiece[r][0] == piece[p][0]) && (lastPiece[r][1] == piece[p][1])){ 
        same = true;
        // break;
      }
    }    
    // check against the buffer
    if(!same && badge.getPixel(piece[p][0], piece[p][1])){
      if(axis){
        nextPiece();       
        // return 42 value to indicate you've hit the bottom
        return 42;
      }
      // reset the variable back to it's original value
      copyPiece(lastPiece, piece);
      return 0;
    }
  }
  writePiece();
  }

  void writePiece(){

    for(int p = 0; p < 4; p++){
      bool flict = false;
      //frameBuffer[piece[p][1]][piece[p][0]] = bright(1);  
      badge.setPixel(piece[p][0], piece[p][1], P_BRIGHT); 
      // checking for conflict 
      for(int o = 0; o < 4; o++){
        if(((piece[o][1] == lastPiece[p][1]) && (piece[o][0] == lastPiece[p][0]))){
          flict = true;
        }
      }
      // if there is no conflict then make the change
      if(!flict){
        badge.setPixel(lastPiece[p][0], lastPiece[p][1], 0);
      }
    }
  }



int oldX = 0;  
unsigned int oldY = 0;  

  void loop()
  {
    static long lastStick = 0;
    static long lastTime = millis();
    int joystick_x = map(analogRead(JOY_X), 250, 700, 3, 0);  


    if(millis() > lastStick){
      if(joystick_x == 3){
        movePiece(1, 0);
      }

      if(joystick_x == 0){
        movePiece(-1, 0);
      }
      // determines the speed side to side of the falling block
      lastStick = millis() + SIDE_SPEED;
    }

    // falling speed determined
    if(millis() > lastTime + scoreSpeed)
    {
      movePiece(0, 1); 
      lastTime = millis();
    }

    // speed up the falling on down joystick
    if(badge.stick_y() > 25){
       if(millis() > oldY){
        movePiece(0, 1); 
        oldY = millis() + 200;  
      }
    }
    
    // check the buttons
    if(badge.button1_debounced()){
      rotatePiece();
    }

    // reading the bottom button 
    if(badge.button2_debounced()){
      dropPiece();
    }
  }

  // Timer1 interupt: pushes the framebuffer to drive the display
  ISR(TIMER1_COMPA_vect) {
    badge.processFB();
  }
