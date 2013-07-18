#include "badgeLib.h"

// initializing the first piece
int piece[5][2] = {{3,0},
  {4,0},
  {5,0},
  {6,0},
  {4,1}};

int lastPiece[5][2] = {0};

int oldX = 0;  
int oldY = 0;  

void setup()
{
  setupBadge();
  Serial.begin(9600);
}


void generatePiece(){
  checkLine();
  delay(10);
  int wat = rand() % 5; 
  //int wat = 3;
  int offset = 4;
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
      piece[4][1] = -1;  
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
      piece[4][0] = 4;  
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

void dropPiece(){

  int deep = 0; 
  copyPiece(piece, lastPiece);
  for(int i = 0; i < 4; i++){

    for(int o = piece[i][0]; o < 16; o++){
      deep = (frameBuffer[o][i] && (deep > i)) ? i : deep; 
    }
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
  int wall = 0;
  for(int p = 0; p < 4; p++){
    //
    int X = lastPiece[p][0] - offset_x;
    int Y = lastPiece[p][1] - offset_y;

    // preform the translation
    piece[p][0] = (Y * -1) + offset_x;
    piece[p][1] = X + offset_y;

    wall = (piece[p][0] < 0) ? wall + 1 : wall;  
    wall = (piece[p][0] > 15) ? wall - 1 : wall;  
  }
  // if near the wall, do this to scoot it over 
  if(wall != 0){
    for(int i = 0; i < 4; i++){
      piece[i][0] += wall;
    }
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
static long lastStick = 0;
void ohJoy(){

  int joystick_x = map(analogRead(JOY_X), 250, 700, 3, 0);  
  int joystick_y = map(analogRead(JOY_Y), 300, 800, 0, 3);  

  bool topB = digitalRead(BUTTON_TOP);   

  if(millis() > lastStick){


    if(joystick_x == 3){
      movePiece(1, 0);
    }

    if(joystick_x == 0){
      movePiece(-1, 0);
    }

    lastStick = millis() + 100;
  }

  if(oldX != joystick_x){

    oldX = joystick_x;  
  }

  if(topB != topBP){
    if(!topB){
      rotatePiece();
    }
    topBP = topB;
  }
}


void loop()
{
  static long lastTime = millis();
  ohJoy();
  if(millis() > lastTime + 300)
  {
    lastTime = millis();
    //  drop();
    movePiece(0, 1); 
  }
}
