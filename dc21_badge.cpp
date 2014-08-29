/*
  dc21_badge.cpp - Library
  created July 20, 2013
*/

#include "Arduino.h"
#include "dc21_badge.h"

// Initialize Hardware (default constructor)
DC21_badge::DC21_badge(){
	currentCol = 0;
	currentIteration = 0;
	rowBuffer = 0;
	grayscale = true;
	lastButton1 = 0;
	lastButton2 = 0;
	clear(); // Clear the frameBuffer

	// Setup the Analog Stick, Buttons, Buzzer
	pinMode(BUZZER, OUTPUT);
	pinMode(BUTTON_TOP, INPUT);
	pinMode(JOY_X, INPUT);
	pinMode(JOY_Y, INPUT);
	digitalWrite(BUZZER, HIGH);
	digitalWrite(BUTTON_TOP, HIGH);
	DDRD = DDRD & PORT5_MASK;    // PORTD5 input - Bottom Button
	PORTD = PORTD | PORT5_SET;   // PORTD5 pull-up

	// Setup the Shift Registers
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
	
	digitalWrite(COL_OUTPUT_ENABLE, HIGH);	// Active is LOW
	digitalWrite(ROW_OUTPUT_ENABLE, HIGH);	// Active is LOW
	digitalWrite(COL_DATA, LOW);			// Default to low
	digitalWrite(COL_CLOCK, LOW);			// Default to low
	digitalWrite(ROW_DATA, LOW);			// Default to low
	digitalWrite(ROW_CLOCK, LOW);			// Default to low
	
	// Reset the Shift Registers
	digitalWrite(COL_MEM_RESET, LOW);		// Active is LOW
	digitalWrite(COL_MEM_RESET, HIGH);		// Active is LOW
	digitalWrite(ROW_MEM_RESET, LOW);		// Active is LOW
	digitalWrite(ROW_MEM_RESET, HIGH);		// Active is LOW
	digitalWrite(COL_OUTPUT_ENABLE, LOW);	// Active is LOW
	digitalWrite(ROW_OUTPUT_ENABLE, LOW);	// Active is LOW
	
	// For bitwise maths to set a bit
	bit16_set[0] = 0x0001;bit16_set[1] = 0x0002;bit16_set[2] = 0x0004;bit16_set[3] = 0x0008;
	bit16_set[4] = 0x0010;bit16_set[5] = 0x0020;bit16_set[6] = 0x0040;bit16_set[7] = 0x0080;
	bit16_set[8] = 0x0100;bit16_set[9] = 0x0200;bit16_set[10] = 0x0400;bit16_set[11] = 0x0800;
	bit16_set[12] = 0x1000;bit16_set[13] = 0x2000;bit16_set[14] = 0x4000;bit16_set[15] = 0x8000;
	// For bitwise maths to clear a bit
	bit16_clear[0] = 0xFFFE;bit16_clear[1] = 0xFFFD;bit16_clear[2] = 0xFFFB;bit16_clear[3] = 0xFFF7;
	bit16_clear[4] = 0xFFEF;bit16_clear[5] = 0xFFDF;bit16_clear[6] = 0xFFBF;bit16_clear[7] = 0xFF7F;
	bit16_clear[8] = 0xFEFF;bit16_clear[9] = 0xFDFF;bit16_clear[10] = 0xFBFF;bit16_clear[11] = 0xF7FF;
	bit16_clear[12] = 0xEFFF;bit16_clear[13] = 0xDFFF;bit16_clear[14] = 0xBFFF;bit16_clear[15] = 0x7FFF;
}


// Return the state of Button 1
// true if button1 is pressed (top button)
bool DC21_badge::button1() {
	return(!(PINE & PORT6_SET));
}


// Return the state of Button 2
// true if button1 is pressed (bottom button)
bool DC21_badge::button2() {
	return(!(PIND & PORT5_SET));
}


// Return the X position of the analog stick
int DC21_badge::stick_x(unsigned int scaler) {
	return (1024-analogRead(JOY_X))/scaler;	// Invert and Scale
}


// Return the Y position of the analog stick
int DC21_badge::stick_y(unsigned int scaler) {
	return analogRead(JOY_Y)/scaler;		// Scale
}


// Return the state of Button 1
// true if button1 is pressed for more then DEBOUNCEMILLIS (top button)
bool DC21_badge::button1_debounced(unsigned int dtime) {
	if(!(PINE & PORT6_SET)){
		if(lastButton1>0){
			if(millis() > lastButton1 + dtime) {
				lastButton1 = 0; //Stop reporting button pressed, until button is reset
				return true;
			} else {
				return false;
			}
		} else {
			if(lastButton1== -1) lastButton1 = millis(); //Wait for reset, before counting again...
			return false;
		}
	} else {
		lastButton1 = -1; // Reset / Stage for next press
		return false;
	}
}


// Return the state of Button 2
// true if button2 is pressed for more then DEBOUNCEMILLIS (bottom button)
bool DC21_badge::button2_debounced(unsigned int dtime) {
	if(!(PIND & PORT5_SET)){
		if(lastButton2>0){
			if(millis() > lastButton2 + dtime) {
				lastButton2 = 0; //Stop reporting button pressed, until button is reset
				return true;
			} else {
				return false;
			}
		} else {
			if(lastButton2== -1) lastButton2 = millis(); //Wait for reset, before counting again...
			return false;
		}
	} else {
		lastButton2 = -1; // Reset / Stage for next press
		return false;
	}
}


// Both Buttons pressed at the same time for [dtime] ms or more
// Most likely for menu use
// true if both buttons are pressed for more then [dtime]
bool DC21_badge::bothButtons_debounced(unsigned int dtime) {
	if(!(PINE & PORT6_SET) && !(PIND & PORT5_SET)){
		if(lastButtonBoth>0){
			if(millis() > lastButtonBoth + dtime) {
				lastButtonBoth = 0; //Stop reporting button pressed, until button is reset
				return true;
			} else {
				return false;
			}
		} else {
			if(lastButtonBoth== -1) lastButtonBoth = millis(); //Wait for reset, before counting again...
			return false;
		}
	} else {
		lastButtonBoth = -1; // Reset / Stage for next press
		return false;
	}
}


// Set an individual pixel
void DC21_badge::setPixel(unsigned int x, unsigned int y, unsigned int brightness) {
	if(x < 16 && y < 16 && brightness < 4){
		switch(brightness){
			case (1): //33% brightness
				frameBuffer[y][0] = frameBuffer[y][0] | bit16_set[15-x];
				frameBuffer[y][1] = frameBuffer[y][1] & bit16_clear[15-x];
				break;
			case (2): //66% brightness
				frameBuffer[y][0] = frameBuffer[y][0] & bit16_clear[15-x];
				frameBuffer[y][1] = frameBuffer[y][1] | bit16_set[15-x];
				break;
			case(3): //100% brightness
				frameBuffer[y][0] = frameBuffer[y][0] | bit16_set[15-x];
				frameBuffer[y][1] = frameBuffer[y][1] | bit16_set[15-x];
				break;
			default: //0% brightness
				frameBuffer[y][0] = frameBuffer[y][0] & bit16_clear[15-x];
				frameBuffer[y][1] = frameBuffer[y][1] & bit16_clear[15-x];
				break;
		}
	}
}


// Returns the value of an individual pixel
unsigned int DC21_badge::getPixel(unsigned int x, unsigned int y){
	unsigned int temp = 0;
	if(x < 16 && y < 16){
		if(frameBuffer[y][0] & bit16_set[15-x]) temp++;
		if(frameBuffer[y][1] & bit16_set[15-x]) temp++;
		if((frameBuffer[y][0] & bit16_set[15-x]) && (frameBuffer[y][1] & bit16_set[15-x])) temp++;
	}
	return temp;
}



// Invert the frameBuffer
void DC21_badge::invert() {
	for(int temp = 0; temp < 16; temp++) {
		frameBuffer[temp][0] = frameBuffer[temp][0] ^ 0b1111111111111111;
		frameBuffer[temp][1] = frameBuffer[temp][1] ^ 0b1111111111111111;
	}
}


// Clear the frameBuffer
void DC21_badge::clear() {
	for(int temp = 0; temp < 16; temp++) {
		frameBuffer[temp][0] = 0b0000000000000000;
		frameBuffer[temp][1] = 0b0000000000000000;
	}
}


// Process frameBuffer to the badge
void DC21_badge::processFB(){
	// Select the row we are manipulating this time
	if(currentCol > 15){
		currentCol = 0;
		PORTB = (PORTB & PORT6_MASK);	// Set COL DATA Low
	
		// Current Iteration for Shading support
		currentIteration++;
		if(currentIteration > 20) currentIteration = 0;
	} else {
		PORTB = (PORTB | PORT6_SET);	// Set COL DATA High
	}
	PORTC = PORTC | PORT7_SET;  // Set COL CLOCK High
	PORTC = PORTC & PORT7_MASK; // Set COL CLOCK Low
	
	// Shift in the data for this column
	if(currentIteration < 15 && grayscale) {
	
		for(int i = 15; i >=0; i--) {
			if(frameBuffer[i][0] & (1 << currentCol) & frameBuffer[i][1] & (1 << currentCol)) {
				PORTB = (PORTB | PORT5_SET);	// Set ROW DATA High (LED On)
			} else {
				PORTB = (PORTB & PORT5_MASK);	// Set ROW DATA Low (LED Off)
			}
			PORTD = PORTD | PORT6_SET;			// Set ROW CLOCK High
			PORTD = PORTD & PORT6_MASK;			// Set ROW CLOCK Low
		}

	} else if (currentIteration > 16 && grayscale) {
		for(int i = 15; i >=0; i--) {
			if(frameBuffer[i][1] & (1 << currentCol)) {
				PORTB = (PORTB | PORT5_SET);	// Set ROW DATA High (LED On)
			} else {
				PORTB = (PORTB & PORT5_MASK);	// Set ROW DATA Low (LED Off)
			}
			PORTD = PORTD | PORT6_SET;			// Set ROW CLOCK High
			PORTD = PORTD & PORT6_MASK;			// Set ROW CLOCK Low
		}

	} else {
		for(int i = 15; i >=0; i--) {
			if(frameBuffer[i][0] & (1 << currentCol) | frameBuffer[i][1] & (1 << currentCol)) {
				PORTB = (PORTB | PORT5_SET);	// Set ROW DATA High (LED On)
			} else {
				PORTB = (PORTB & PORT5_MASK);	// Set ROW DATA Low (LED Off)
			}
			PORTD = PORTD | PORT6_SET;			// Set ROW CLOCK High
			PORTD = PORTD & PORT6_MASK;			// Set ROW CLOCK Low
		}

	}

	// Strobe
	PORTF = PORTF | PORT7_SET;		// Set COL LATCH (STCP) high
	PORTF = PORTF & PORT7_MASK;		// Set COL LATCH (STCP) low
	PORTD = PORTD | PORT7_SET;		// Set ROW LATCH (STCP) high
	PORTD = PORTD & PORT7_MASK;		// Set ROW LATCH (STCP) low
	
	currentCol++;
}


