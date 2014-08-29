/*
  dc21_badge.h - Library
  created July 20, 2013
*/

#ifndef DC21_BADGE
#define DC21_BADGE

// HARDWARE DEFINES
#define COL_DATA			10	//P B6
#define COL_OUTPUT_ENABLE	A1	//P F6
#define COL_STROBE			A0	//P F7 Strobe (STCP)
#define COL_CLOCK			13	//P C7 Clock (SHCP)
#define COL_MEM_RESET		5	//P C6

#define ROW_DATA			9	//P B5
#define ROW_OUTPUT_ENABLE	8	//P B4
#define ROW_STROBE			6	//P D7
#define ROW_CLOCK			12	//P D6
#define ROW_MEM_RESET		4	//P D4

#define BUZZER				A4	//P F1
#define JOY_X				A2	//P F5
#define JOY_Y				A3	//P F4
#define BATTERY_VOLTS		A5	//P F0
#define BUTTON_TOP			7	//P E6
//#define BUTTON_BOTTOM			//P D5
#define DEBOUNCEMILLIS		2	//Default # of milliseconds to debounce button presses
#define JOY_SCALE			32	//Default scaler for analog stick input

// For easy / speedy setting of individual bits
#define PORT0_SET			0b00000001
#define PORT1_SET			0b00000010
#define PORT2_SET			0b00000100
#define PORT3_SET			0b00001000
#define PORT5_SET			0b00100000
#define PORT4_SET			0b00010000
#define PORT6_SET			0b01000000
#define PORT7_SET			0b10000000
#define PORT0_MASK			0b11111110
#define PORT1_MASK			0b11111101
#define PORT2_MASK			0b11111011
#define PORT3_MASK			0b11110111
#define PORT4_MASK			0b11101111
#define PORT5_MASK			0b11011111
#define PORT6_MASK			0b10111111
#define PORT7_MASK			0b01111111


class DC21_badge {
	public:
		DC21_badge();						// Default constructor
		
		bool button1();						// true if button1 is pressed (top button)
		bool button2();						// true if button1 is pressed (bottom button)
		int stick_x(unsigned int scaler = JOY_SCALE);	// returns 0-15 for the position of the joystick left-right
		int stick_y(unsigned int scaler = JOY_SCALE);	// returns 0-15 for the position of the joystick up-down

		bool button1_debounced(unsigned int dtime = DEBOUNCEMILLIS);	// true if button1 is pressed for more then 5ms (top button)
		bool button2_debounced(unsigned int dtime = DEBOUNCEMILLIS);	// true if button2 is pressed for more then 5ms (top button)
		bool bothButtons_debounced(unsigned int dtime);	// true if both buttons are pressed for more then [dtime]

		bool grayscale;						// Set to false for B/W (ORs both bits for each pixel), default true
		unsigned int frameBuffer[16][2];	// FrameBuffer for LED Array
		void processFB();					// Redraw the frameBuffer to the badge

		void setPixel(unsigned int x, unsigned int y, unsigned int brightness);	// Set a pixel
		unsigned int getPixel(unsigned int x, unsigned int y);					// Returns the value of a pixel
		void invert();						// Inverts the frameBuffer
		void clear();						// Clears the frameBuffer
		
	private:
		int currentCol;						// currentRow (0 to 15)
		unsigned int currentIteration;		// dimming interation count
		unsigned int rowBuffer;				// Buffer for column data to be shifted each round.
		long lastButton1;					// button1 debounce counter
		long lastButton2;					// button2 debounce counter
		long lastButtonBoth;				// button[1 && 2] debounce counter (probably for menus, etc)
		unsigned int bit16_set[16];			// For bitwise maths to set a bit
		unsigned int bit16_clear[16];		// For bitwise maths to clear a bit
};

#endif
