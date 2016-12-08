/*
Arduino iRacing display by Samuli Kumo

Receives data from iRacing serial-client and displays it on TFT display.

*/

#include <TFT.h>  // Arduino LCD library
#include <SPI.h>

// pin definition for the Uno
#define cs   10
#define dc   9
#define rst  8

// pin definition for the Leonardo
// #define cs   7
// #define dc   0
// #define rst  1

// create an instance of the library
TFT TFTscreen = TFT(cs, dc, rst);

String data;
String dataBuffer;
boolean dataComplete = false;
boolean dataValid = false;
boolean update = true;
char dataPrintout[7]; // char array to print to the screen

					  // variables for delay
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long delayMillis = 64;

void setup() {

	// Put this line at the beginning of every sketch that uses the GLCD:
	TFTscreen.begin();

	// clear the screen with a black background
	TFTscreen.background(0, 0, 0);

	// write the static text to the screen
	// set the font color to white
	TFTscreen.stroke(255, 255, 255);
	// set the font size
	TFTscreen.setTextSize(2);
	// write the text to the top left corner of the screen
	TFTscreen.text("RPM :\n ", 0, 0);
	// ste the font size very large for the loop
	TFTscreen.setTextSize(4);

	//initialize serial:
	Serial.begin(57600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}
}

void loop() {
	currentMillis = millis(); // saving current time at the start of loop

	if (update) //we only want to update TFT in specific intervals 
	{
		if (dataComplete) // is there data available to display
		{
			if (data.charAt(0) == '!' && data.charAt(data.length() - 1) == ';') //is the data packet valid, rpm datapacket should look like "!nnnn;" with variable length of numbers in middle
			{
				dataValid = true;
			}
		}

		if (dataValid) //only update display with valid data
		{
			data.toCharArray(dataPrintout, 8); //converting data-string to char array for TFTscreen
			char *p = dataPrintout; //pointer to beginning of the array
			p[strlen(p) - 1] = 0; //remove trailing symnol ;
			p++; //remove leading symbol ! by moving pointer over by one
			TFTscreen.stroke(255, 255, 255); // set text to white
			TFTscreen.text(p, 0, 20);
			previousMillis = currentMillis;
			dataComplete = false;
			dataValid = false;
			update = false;
		}
	}
	if (currentMillis - previousMillis > delayMillis) //delay timer that does not interrupt whole process
	{
		//display needs to be cleared before new text is drawn, fast way to do it is by re-drawing the text using background color (black)
		char *p = dataPrintout;
		p++;
		TFTscreen.stroke(0, 0, 0);  //set text to black
		TFTscreen.text(p, 0, 20);
		update = true;
	}
}

void serialEvent() {
	while (Serial.available()) {
		// get the new byte:
		char inChar = (char)Serial.read();
		// add it to the inputString:
		dataBuffer += inChar;
		// if the incoming character is a newline, set a flag
		// so the main loop can do something about it:
		if (inChar == ';') {
			data = dataBuffer; //once whole datastring has been received, move it from buffer to data variable
			dataBuffer = ""; //clear buffer
			dataComplete = true; //indicate that data has been received
		}
	}
}