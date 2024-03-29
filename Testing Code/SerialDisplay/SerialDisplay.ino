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

//Initialisation
//--LCD
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//--LCD-END

//--Buttons
int btnInputPin = 0;
int lcd_key     = 0;
int adc_key_in  = 0;

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
long debugBaud = 9600;
long comBaud = 9600;
//--Communications-END

//--General
int gpibErr = 0;
int serialErr = 0;
int genErr = 0;
//--General-END

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // initialize the serial communications
  Serial.begin(debugBaud);
  Serial1.begin(comBaud);
  //TODO Need to change this over to flash memory based
}

void loop() {
  displayControl();
    

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
    /*Serial.write("KEY PRESS ");
    switch (read_LCD_buttons(btnInputPin))
    {
    case 0:
      Serial.write("Right\n");
      break;
    case 1:
      Serial.write("Up\n");
      break;
    case 2:
      Serial.write("Down\n");
      break;
    case 3:
      Serial.write("Left\n");
      break;
    case 4:
      Serial.write("Select\n");
      break;
    default:
      Serial.write("None\n");
      break;
    }*/

    //int val = analogRead(0);
    //Serial.println(val);


  }
  
  if (millis() - conTimer < 5000 || conMenu == 3) {

    if (read_LCD_buttons(btnInputPin) == btnSELECT) { //Action for Select Button
      switch (conMenu) {
      case 3:
        conMenu = 0;
        Serial.println("Reset conMenu")
        break;
      default:
        conMenu++;
        lcd.clear();
        Serial.write("Select ");
        Serial.println(conMenu);
        break;
      }
    }

    if (read_LCD_buttons(btnInputPin) == btnUP) { //Action for Up Button
      switch (conMenu) { //TODO
      case 3:
        conMenu = 0;
        Serial.println("Reset conMenu")
        break;
      default:
        conMenu++;
        lcd.clear();
        Serial.write("Select ");
        Serial.println(conMenu);
        break;
      }
    }

    if (read_LCD_buttons(btnInputPin) == btnDOWN) { //Action for Down Button
      switch (conMenu) { //TODO
      case 3:
        conMenu = 0;
        Serial.println("Reset conMenu")
        break;
      default:
        conMenu++;
        lcd.clear();
        Serial.write("Select ");
        Serial.println(conMenu);
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
    //code
    break;
  case 2:
    //code
    break;
  case 3: // Serial 1 Mon
    //code
    break;
  default:
    conMenu = 0;
    break;
  }
}