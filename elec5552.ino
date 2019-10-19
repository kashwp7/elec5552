// University of Western Australia
// ELEC5552 - Semester 2 2019 - Project 9
// Vihanga Silva - 21480143
// Vibhuthi Wickramage - 21498423
// Jacob Day - 21489759
// Peter-Jan Boom - 21529168
// Xia Wen - 22452416
// Congjian Sun - 22449928

//Libraries
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

//Initialisation
//--LCD
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//--LCD-END

//--EEPROM
struct storageStruct { //Structure for the stored data
    int debugBaud;
    int comBaud;
    byte add;
};

storageStruct storedData; //initialising a variable to hold data in memory
int dataAddress = 0; //Address where the data is stored

void eepromLoop(void);
//--EEPROM-END

//--Buttons
int btnInputPin = 0;
int lcd_key     = 0;
int adc_key_in  = 0;

//definitions for the diffent buttons
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

int read_LCD_buttons(int pin);
//--Buttons-END

//--Controls
unsigned long conTimer ;
int conMenu = 0;

void displayControl(void);
//--Controls-END

//--Communications
int debugBaud;
int comBaud;

long baudRates[] = {4800, 9600, 14400, 19200, 38400, 57600, 115200};
//--Communications-END

//--Serial
byte serialBuffer[255];
int lenSerialBuffer = 0;

void serialLoop(void);
//--Serial-END

//--GPIB
const int GPIB_DATA[] = {30, 31, 32, 33, 34, 35, 36, 37};

//##Command pins
const int GPIB_EOI = 20;
//##IFC on interupt pin
const int GPIB_IFC = 21; 
const int GPIB_SRQ = 22;
const int GPIB_ATN = 23; 
const int GPIB_REN = 24; 

const int GPIB_DAV = 25;
const int GPIB_NRFD = 26;
const int GPIB_NDAC = 27;	
const int GPIB_CONTROL[] = {GPIB_REN, GPIB_ATN, GPIB_SRQ, GPIB_IFC, GPIB_EOI};
const int GPIB_SHAKE[] = {GPIB_NDAC, GPIB_NRFD, GPIB_DAV};

//##GPIB mode
const int mode_idle = 0;
const int mode_listen = 1;
const int mode_talk = 2;
const int mode_secondary = 3;
int mode = mode_idle;

//##Delay length (in ms)
const long del_t = 10;
const int itr_max = 10;

//##GPIB buffer setup
const int buff_size = 255;
int buff_GPIB[buff_size];
int buff_GPIB_head = 0;
int buff_GPIB_tail = 0;
int buff_GPIB_count = 0;

//##Address
int add_own = 12;

//##Functions
void setup_GPIB(void);
void interupt_setup(void);
void listen_setup(void);
void talk_setup(void);
void main_GPIB(void);
void listen_GPIB(void);
void talk_GPIB(int data);
void process_GPIB(void);
void process_Serial(void);
int read_GPIB(void);
void write_GPIB(int data);
void write_GPIB_buff(int data);
int read_GPIB_buff(void);
void run_IFC(void);
//--GPIB-END

//--General
int gpibErr = 0;
int serialErr = 0;
int genErr = 0;

void errTracking(int errType);
void mb(void); // Main Function used to do the conversion of information
//--General-END

void setup(void) {
    eepromLoop(); // to get the stored values
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    // initialize the serial communications
    Serial.begin(baudRates[debugBaud]);
    Serial1.begin(baudRates[comBaud]);

    setup_GPIB(); //Setup GPIB

}

void loop(void) {
    displayControl();
    eepromLoop();

    if (Serial1.available() > 0)
    {
        serialLoop();
    }
    
    // TODO - GPIB read if available
    // if (/* condition */)
    // {
    //     /* code */
    // }

    mb(); // Main Control Function

    while (Serial.available() > 0)
    {
        int inByte = Serial.read();
        Serial1.write(inByte);
    }

    main_GPIB(); //Contains all GPIB processing
}


//Other Functions 
int read_LCD_buttons(int pin) {
  
 adc_key_in = analogRead(pin);      // read the value from the sensor
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 250)  return btnUP;
 if (adc_key_in < 450)  return btnDOWN;
 if (adc_key_in < 650)  return btnLEFT;
 if (adc_key_in < 850)  return btnSELECT;

 return btnNONE;  // when all others fail, return this...
}

void displayControl(void) {
  lcd.setCursor(0, 0);                   // set the LCD cursor   position

  if (read_LCD_buttons(btnInputPin) != btnNONE) {
    conTimer = millis();

    //Debugging Code
    // Serial.write("KEY PRESS ");
    // switch (read_LCD_buttons(btnInputPin))
    // {
    // case 0:
    //   Serial.write("Right\n");
    //   break;
    // case 1:
    //   Serial.write("Up\n");
    //   break;
    // case 2:
    //   Serial.write("Down\n");
    //   break;
    // case 3:
    //   Serial.write("Left\n");
    //   break;
    // case 4:
    //   Serial.write("Select\n");
    //   break;
    // default:
    //   Serial.write("None\n");
    //   break;
    // }

    // int val = analogRead(0);
    // Serial.println(val);


  }
  
  if (millis() - conTimer < 5000 || conMenu == 3) {

    if (read_LCD_buttons(btnInputPin) == btnSELECT) { //Action for Select Button
      switch (conMenu) {
        case 3:
            conMenu = 0;
            Serial.println("Reset Menu");
            break;
        default:
            conMenu++;
            lcd.clear();
            Serial.write("Menu: ");
            Serial.println(conMenu);
            break;
      }
    }

    if (read_LCD_buttons(btnInputPin) == btnUP) { //Action for Up Button
        switch (conMenu) {
            case 1:
                if (comBaud >= ((sizeof(baudRates)/sizeof(baudRates[0])) - 1)) {
                    comBaud = 0;
                } else {
                    comBaud++;
                }
                Serial.print("Changing Com Baud Rate to ");
                Serial.println(baudRates[comBaud]);
                eepromWrite();
                Serial1.end();
                Serial1.begin(baudRates[comBaud]);
                break;
            case 2:
                if (debugBaud >= ((sizeof(baudRates)/sizeof(baudRates[0])) - 1)) {
                    debugBaud = 0;
                } else {
                    debugBaud++;
                }
                Serial.print("Changing Debug Baud Rate to ");
                Serial.println(baudRates[debugBaud]);
                eepromWrite();
                Serial.end();
                Serial.begin(baudRates[debugBaud]);
                break;
            case 3:
                if (add_own >= 30) {
                    add_own = 0;
                } else {
                    add_own++;
                }
                Serial.print("Changing Address");
                Serial.println(add_own);
                eepromWrite();
                break;
            default:
                break;
        }
    }

    if (read_LCD_buttons(btnInputPin) == btnDOWN) { //Action for Down Button
        switch (conMenu) {
            case 1:
                if (comBaud <= 0) {
                    comBaud = ((sizeof(baudRates)/sizeof(baudRates[0])) - 1);
                } else {
                    comBaud--;
                }
                Serial.print("Changing Com Baud Rate to ");
                Serial.println(baudRates[comBaud]);
                eepromWrite();
                Serial1.end();
                Serial1.begin(baudRates[comBaud]);
                break;
            case 2:
                if (debugBaud <= 0) {
                    debugBaud = ((sizeof(baudRates)/sizeof(baudRates[0])) - 1);
                } else {
                    debugBaud--;
                }
                Serial.print("Changing Debug Baud Rate to ");
                Serial.println(baudRates[debugBaud]);
                eepromWrite();
                Serial.end();
                Serial.begin(baudRates[debugBaud]);
                break;
            case 3:
                if (add_own <= 0) {
                    add_own = 30;
                } else {
                    add_own--;
                }
                Serial.print("Address Change ");
                Serial.println(add_own);
                eepromWrite();
                break;
            default:
                break;
        }
    }

    delay(500);
    
  } else {
    conTimer = 0;
    conMenu = 0;
  }

  switch (conMenu)
  {
  case 0:
    lcd.print("Current Status");
    lcd.setCursor(0,1);
    if (gpibErr > 0) {
      lcd.print("ERR ");
      lcd.print("G:");
      lcd.print(gpibErr);
      lcd.print(" S:");
      lcd.print(serialErr);
      lcd.print(" B:");
      lcd.print(genErr);
    } else {
      lcd.print("Normal");
    }
    break;
  case 1:
    lcd.print("Coms Baud");
    lcd.setCursor(0,1);
    lcd.print(baudRates[comBaud]);
    lcd.print("    ");
    break;
  case 2:
    lcd.print("Debug Baud");
    lcd.setCursor(0,1);
    lcd.print(baudRates[debugBaud]);
    lcd.print("    ");
    break;
  case 3:
    lcd.print("Address GPIB"); //TODO - Convert this over to transmitted data
    lcd.setCursor(0,1);
    lcd.print(add_own , DEC);
    lcd.print("    ");
    break;
  default:
    conMenu = 0;
    break;
  }
}

void eepromLoop(void) {
    EEPROM.get(dataAddress, storedData);

    //Debugging
    // Serial.print("Stored Debug: ");
    // Serial.println(storedData.debugBaud);
    // Serial.print("Stored Debug: ");
    // Serial.println(storedData.comBaud);

    //Safe Gauds for reading data that is out of range
    if ( storedData.debugBaud > ((sizeof(baudRates)/sizeof(baudRates[0])) - 1) || storedData.debugBaud < 0 ) {
        debugBaud = (sizeof(baudRates)/sizeof(baudRates[0])) - 1;
        errTracking(0);
    } else {
        debugBaud = storedData.debugBaud;
    }

    if ( storedData.comBaud > ((sizeof(baudRates)/sizeof(baudRates[0])) - 1) || storedData.comBaud < 0 ) {
        comBaud = (sizeof(baudRates)/sizeof(baudRates[0])) - 1;
        errTracking(0);
    } else {
        comBaud = storedData.comBaud;
    }

    add_own = storedData.add;
}

void eepromWrite(void) {
    storedData.comBaud = comBaud;
    storedData.debugBaud = debugBaud;
    storedData.add = add_own;
    EEPROM.put(dataAddress, storedData);
}

void errTracking(int errType) {
    Serial.println("----ERROR----");
    switch (errType) {
    case 0:
        Serial.println("EEPROM: Values read are out of range");
        Serial.println("EEPROM: Setting Values to defaults");
        genErr++;
        break;
    
    case 1:
        Serial.println("SERIAL: Serial Buffer is Full");
        serialErr++;
        break;
    
    default:
        break;
    }
}

void serialLoop(void) {
    int i = 0;
    int Bt = 0;

    do
    {
        i++;
        Bt = Serial1.read();
        if (Bt == -1)
        {
            break;
        } else {
            serialBuffer[i-1] = (byte)Bt;
        }
    } while (Serial1.available() > 0 && i < 255);

    if (Bt == -1) {
        lenSerialBuffer = i - 1;
    } else {
        lenSerialBuffer = i;
    }
    
    if (Serial1.available() > 0 && i >= 255) { //Checking if Serial Buffer is full
        errTracking(1);
    }
}

void mb(void) {

    if (lenSerialBuffer > 0)
    {
        Serial.println("SERIAL--START");
        Serial.write(serialBuffer, lenSerialBuffer);
        lenSerialBuffer = 0;
        Serial.println("");
        Serial.println("SERIAL--END");
    }
}

//GPIB Functions
void setup_GPIB(void) {
	// pin mode setup
	listen_setup();
	interupt_setup();
	
	// empty buffers
	buff_GPIB_count = 0;
		// empty RS232 buffer	
}

void interupt_setup(void) {
	attachInterrupt(digitalPinToInterrupt(GPIB_IFC), run_IFC, FALLING);
}

void listen_setup(void) {
	// set data and command pins to input
	for (int ii = 7; ii >= 0; ii--) {
		pinMode(GPIB_DATA[ii], INPUT);
	}
	for (int ii = 4; ii >= 0; ii--) {
		pinMode(GPIB_CONTROL[ii], INPUT);
	}
	pinMode(GPIB_SRQ, OUTPUT);
	digitalWrite(GPIB_SRQ,HIGH);
	
	// setup handshakes
	pinMode(GPIB_DAV, INPUT);
	pinMode(GPIB_NRFD, OUTPUT);
	digitalWrite(GPIB_NRFD, LOW);
	pinMode(GPIB_NDAC, OUTPUT);
	digitalWrite(GPIB_NDAC, LOW);
}

void talk_setup(void) {	
	// set data pins to output
	for (int ii = 7; ii >= 0; ii--) {
		pinMode(GPIB_DATA[ii], OUTPUT);
	}
	for (int ii = 4; ii >= 0; ii--) {
		pinMode(GPIB_CONTROL[ii], INPUT);
	}
	
	// set EOI and SRQ to outputs
	pinMode(GPIB_EOI,OUTPUT);
	digitalWrite(GPIB_EOI,HIGH);
	pinMode(GPIB_SRQ,OUTPUT);
	digitalWrite(GPIB_SRQ,HIGH);
	
	// setup handshakes
	pinMode(GPIB_DAV, OUTPUT);
	digitalWrite(GPIB_DAV, HIGH);
	pinMode(GPIB_NRFD, INPUT);
	pinMode(GPIB_NDAC, INPUT);
}

void main_GPIB(void) {
	listen_GPIB();
}

void listen_GPIB(void) {
	int itr_count = 0;
    int dav;
	// set up pin input/output modes
	listen_setup();

	// set not ready for data to FALSE
	digitalWrite(GPIB_NRFD, HIGH);

	// wait until data is validated
	itr_count = 0;
	dav = digitalRead(GPIB_DAV);
	while(dav == HIGH){
		dav = digitalRead(GPIB_DAV);
		delay(del_t);
		// if device times out, process GPIB buffer
		if(itr_count > itr_max) {
			digitalWrite(GPIB_NRFD, LOW);
			process_GPIB();
			itr_count = 0;
			digitalWrite(GPIB_NRFD, HIGH);			
		}
		itr_count++;
	}

	int data = read_GPIB();
	
	int atn = bitRead(data,9);
	int is_process = atn * (bitRead(data,6) | bitRead(data,5));
	
	if(atn == 1 || mode == mode_listen) {
		// data to buffer only if it is either a command or device is listening
		write_GPIB_buff(data);
	}
	
	if(is_process == 1) {
		// process GPIB buffer if any device is addressed (talk or listen) or untalk/unlisten
		process_GPIB();
	}
	
	// set ready for data to false
	digitalWrite(GPIB_NRFD, LOW);
	
	// set data accepted to true
	digitalWrite(GPIB_NDAC, HIGH);
	
	// wait until listener stops validating data
	dav = digitalRead(GPIB_DAV);
	while(dav == LOW) {
		dav = digitalRead(GPIB_DAV);
		delay(del_t);
		// timeout
			// add appropriate timeout
		itr_count++;
	}
	// set data accepted to false
	digitalWrite(GPIB_NDAC, LOW);
}

void talk_GPIB(int data) {
	int nrfd;
    int ndac;
	// set up pin input/output modes
	talk_setup();
	
	// wait until listener is ready for byte, and data not accepted
	nrfd = digitalRead(GPIB_NRFD);
	ndac = digitalRead(GPIB_NDAC);
	while(nrfd == LOW || ndac == HIGH){
		nrfd = digitalRead(GPIB_NRFD);
		ndac = digitalRead(GPIB_NDAC);
		delay(del_t);
	}
	
	// write byte & controls to data lines
	write_GPIB(data);
	
	// set data valid to true
	digitalWrite(GPIB_DAV, LOW);
	
	// wait until data is accepted and listener is not ready for data
	nrfd = digitalRead(GPIB_NRFD);
	ndac = digitalRead(GPIB_NDAC);
	while(ndac == LOW || nrfd == HIGH){
		ndac = digitalRead(GPIB_NDAC);
		delay(del_t);
	}
	
	// set data valid to false
	digitalWrite(GPIB_DAV, HIGH);
}

void process_GPIB(void) {
	// Process all the commands/data present in the GPIB buffer
	while(buff_GPIB_count > 0){
		int buff = read_GPIB_buff();
		int comms = buff / 256;
		int data = buff % 256;
		
		int atn = bitRead(comms, 1);
		if (atn == 1){
			int command_mode = ((data / 32) % 4);
			switch (command_mode) {
				case mode_idle:
					int data_mod = data % 128;
					if(data_mod == 20){
						// device clear
						setup_GPIB();						
					}
					break;
					
				case mode_listen:
					int add = data % 32;
					if(add == add_own){
						// MLA
						mode = mode_listen;
					}
					
					if(add == 31){
						// unlisten
						mode = mode_idle;
					}
					break;
					
				case mode_talk:
					int add = data % 32;
					if(add == add_own){
						// MTA
						process_Serial();
					}
					
					// add untalk if necesaary
					break;
					
				case mode_secondary:
					// No commands here yet
					break;
								
			} 
		} else {
			// Send data straigh thru to Serial
		}
		
	}
}

void process_Serial(void) {
	// Send all the data in the serial buffer
	// for each piece of data in the Serial buffer, use talk_GPIB(data)
	
}

int read_GPIB(void) {
    int data_ii;
	// read control pins
	int data = 0;
	for (int ii = 4; ii >= 0; ii--) {
		data = data * 2;
		data_ii = digitalRead(GPIB_CONTROL[ii]);
		if(data_ii == LOW) data++;
	}	
	
	// read data pins
	for (int ii = 7; ii >= 0; ii--) {
		data = data * 2;
		data_ii = digitalRead(GPIB_DATA[ii]);
		if(data_ii == LOW) data++;
	}
	return data;
}

void write_GPIB(int data) {
	for(int ii = 0; ii <= 7; ii++){
		int data_ii = bitRead(data, ii);
		if(data_ii == 1){
			digitalWrite(GPIB_DATA[ii], LOW);
		}
		else{
			digitalWrite(GPIB_DATA[ii], HIGH);			
		}		
	}
	
	int data_EOI = bitRead(data, 12);
	if(data_EOI == 1){
		digitalWrite(GPIB_EOI, LOW);
	}
	else{
		digitalWrite(GPIB_EOI, HIGH);
	}
	
	int data_SRQ = bitRead(data, 10);
	if(data_SRQ == 1){
		digitalWrite(GPIB_SRQ, LOW);
	}
	else{
		digitalWrite(GPIB_SRQ, HIGH);
	}
}

void write_GPIB_buff(int data) {
	buff_GPIB[buff_GPIB_head] = data;
	buff_GPIB_head = (buff_GPIB_head + 1) % buff_size;
	buff_GPIB_count++;
}

int read_GPIB_buff(void) {
	int data = buff_GPIB[buff_GPIB_tail];
	buff_GPIB_tail = (buff_GPIB_tail + 1) % buff_size;
	buff_GPIB_count--;
	return data;
}

void run_IFC(void) {
	return;
}