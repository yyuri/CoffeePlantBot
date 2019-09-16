#include <Switcher.h>        //Switcher_lib library to operate relays with timers and periods.
#include <Adafruit_SSD1306.h> //OLED SCREEN library
#include <avr/wdt.h>          // We have a watchdog just in case (wdt.h)
#include <EEPROM.h>

#define TENSEC (1000UL * 10)
#define ONEMIN (1000UL * 60 * 1)
#define THIRTYMIN (1000UL * 60 * 30)


Adafruit_SSD1306 display(4); //OLED DECLARATION

                                    // SWITCH DECLARATIONS  //

Switcher lights(lights_p,HIGH);
Switcher r1(r1_p,HIGH);
Switcher r2(r2_p,HIGH);
Switcher soil(moisture_p,HIGH);

// ANALOG INPUTS  //

int moisture_sensor = A2;

// DIGITAL INPUTS  //

int lights_p = 11;  // Lights relay
int r1_p = 10;      // Water pump relay
int moisture_p = 8; // Moisture sensor relay

// SENSORS VARIABLES  //

int moisture;                       //We store here the moisture values

// GLOBAL VARIABLES  //

int   hoursDays = 24;               //Number of Hours per Day
int   hoursLight = 14;              //Number of daily hours per Day
int   hoursDark = 10;               //Number of nightly hours per Day
int   min_moisture=42               //Desired min moisture target
unsigned long checktime30 = THIRTYMIN;
unsigned long checktime10s = TENSEC;
unsigned long checktime1m = ONEMIN;

int last_lights=0;
int last_r1=0;
int act;
bool regar;
int day=2;
int d=1;
int attempt=0;
int eeAddress=0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Arduino Setting up"));
  wdt_disable();        // We disable the watchdog during setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  intro();
  
  pinMode(lights_p,OUTPUT);
  pinMode(r1_p,OUTPUT);
  pinMode(moisture_p,OUTPUT);
  digitalWrite(lights_p,HIGH);
  digitalWrite(r1_p,HIGH);
  digitalWrite(moisture_p,HIGH);

  if ( EEPROM.read(eeAddress) == 255 ) {                              //In case EEPROM is never been initialized
    EEPROM.put(eeAddress,day);
    Serial.println("First case EEPROM");   
    Serial.println(EEPROM.get(eeAddress,day));
  }             
  else if (   EEPROM.get(eeAddress,d) < day ) {                       //In case EEPROM has some int data at frist position inferior than day = 1
    EEPROM.put(eeAddress,day);
    Serial.println("Second case EEPROM");   
    Serial.print(day);
    Serial.print( EEPROM.get(eeAddress,d));     
  }
  else if (   sizeof(EEPROM.get(eeAddress,d)) != sizeof(day) ) {       //In case EEPROM has any random non-int data at frist position
    EEPROM.put(eeAddress,day);
    Serial.println("Third case EEPROM");   
    Serial.print(day);
    Serial.print( EEPROM.get(eeAddress,d));     
  }   
  else {                                                                //Retrive data in case of Powercut
    day = EEPROM.get(eeAddress,d);
    Serial.println("Fourth case EEPROM");   
    Serial.print(day);
    Serial.print( EEPROM.get(eeAddress,d));               

  }
  delay(600);
  moisture = SoilSensor();          //Get first Read from Soil Moisture Sensor
  test_lights();
  test_r1();
  lights.Start();
  delay(1000); 
  wdt_enable(WDTO_8S);  // Watchdog setup set to 8 seconds
  Serial.println(F("End of Setup"));
}



void loop() {
  wdt_reset();                        //Watchdog reset on every loop
  r1.Timer(1,0);                      //We check if water pump 1 was ON, if so, we turn it of after 1Second.
                                                                                        
                                                                                      // STUFF WE CHECK EVERY 10S //                                                                                  
  if((long)(millis() - checktime10s) >= 0) {
    if (regar == 1) {
      moisture = SoilSensor();
      regar = 0;
      watering();                        
    }
  switch (act%3) {
    case 1:
      firstscreen();
      break;
    case 2:
      secondscreen();
      break;
    default:
      intro();
      act = 0;                        //We turn act to 0 when  act%0 = 0 to avoid overflow.
      break;
  }
  act = act+1;
  checktime10s = millis() + TENSEC;
}

                                                                                         // STUFF WE CHECK EVERY 1min //
  if((long)(millis() - checktime1m) >= 0) {

    if (moisture <= min_moisture) {
      r1.Start();
      regar = 1;
      attempt = attempt + 1;
      if (attempt > 7) {
        alert();
      }
      else {
        watering();
      }
    }
    else {
      attempt = 0;
    }
    last_lights=(millis()-lights._previousMillis)/3600000;
    last_r1=(millis()-r1._previousMillis)/60000;    
    if (lights.Period(10,14,2) == 1) {            //We check if it's time to switch light state
      day = day + 1;
      EEPROM.put(eeAddress,day);
    }       
    checktime1m = millis() + ONEMIN;    
    }
        
                                                                                         
                                                                                         
                                                                                         // STUFF WE CHECK EVERY 30min //
  if((long)(millis() - checktime30) >= 0) {
    moisture = SoilSensor();               
    checktime30 = millis() + THIRTYMIN;
    }
}





                                                                          //             SOIL SENSOR READ                         //

int SoilSensor(){
  int a=0;
  soil.Start();
  delay(200);                       //We should be avoiding delays, but here we are. We need to wait to get the analogread lecture. TO-DO: Find something better darling.
  a= analogRead(moisture_sensor);   //We read the analog value of the mositure sensor
  a = constrain(a,400,1023);        //We constrain the values to avoid faults
  a = map(a,1023,400,0,99);         //We map it from 0% to 99% due to display limitations: 100% value does not fit on the little display we use

// 401-440 very wet, water inmersed.
// 480-540 humid
// 1023 dry

  soil.Stop();
  return (a);  
}
                                                                          //             SCREENS DISPLAY FUNCTIONS                 //
void firstscreen() {
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("Dia:");
display.setCursor(24,0); 
display.println(day);
display.setCursor(84,0);
display.println("Luz:");
display.setCursor(110,0);

if (digitalRead(lights_p)) {
  display.println("OFF");
}
else {  
  display.println("ON");
}
display.setCursor(0,8);
display.drawLine(0,8,display.width(),8, WHITE);
display.setTextSize(2);             // Draw 2X-scale text
display.setCursor(4,14);
display.println(moisture);
display.setCursor(30,14);
display.println("%");
display.setTextSize(1);             // Draw 2X-scale text
display.drawLine(46,8,46,46, WHITE);
display.setCursor(51,13);
display.println("Ultimo riego");
display.setCursor(62,23);
display.println("hace");
display.setCursor(90,23);

if (last_r1<59) {
display.println(last_r1);
display.setCursor(102,23);
display.println("min");
}
else {
display.println((last_r1/60));
display.setCursor(102,23);
display.println("h");
}

display.display();
}

void secondscreen() {
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("Dia:");
display.setCursor(24,0); 
display.println(EEPROM.get(eeAddress,day));
display.setCursor(84,0);
display.println("Luz:");
display.setCursor(110,0); 
if (digitalRead(lights_p)) {
  display.println("OFF");
}
else {  display.println("ON");}

display.setCursor(0,8);
display.drawLine(0,8,display.width(),8, WHITE);
display.setTextSize(2);             // Draw 2X-scale text
display.setCursor(4,14);
display.println(moisture);
display.setCursor(30,14);
display.println("%");
display.setTextSize(1);             // Draw 2X-scale text
display.drawLine(46,8,46,46, WHITE);



if (digitalRead(lights_p)) {
display.setCursor(54,13);
display.println("Amanecer en");
display.setCursor(78,23);
display.println(hoursDark-last_lights);
display.setCursor(90,23);
display.println("h");
}
else {
display.setCursor(54,13);
display.println("Anochecer en");
display.setCursor(78,23);
display.println(hoursLight-last_lights);
display.setCursor(90,23);
display.println("h");  

}
display.display();
}

void intro() {
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("     The plantbot");
display.fillCircle(54, 15, 3, WHITE);
display.fillCircle(72, 15, 3, WHITE);
display.drawPixel(46, 23, WHITE);
display.drawPixel(45, 23, WHITE);
display.drawPixel(44, 23, WHITE);
display.drawPixel(44, 22, WHITE);
display.drawPixel(43, 22, WHITE);
display.drawPixel(43, 21, WHITE);
display.drawPixel(42, 21, WHITE);

display.drawPixel(82, 23, WHITE);
display.drawPixel(83, 23, WHITE);
display.drawPixel(83, 22, WHITE);
display.drawPixel(84, 22, WHITE);
display.drawPixel(84, 21, WHITE);
display.drawPixel(85, 21, WHITE);

display.drawLine(45,24,82,24, WHITE);
display.display();
}

void alert() {
display.clearDisplay();
display.setTextSize(2);             // Draw 2X-scale text
display.setCursor(35,8);
display.println("ERROR");
display.display();
}
void watering() {
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("     The plantbot");
display.setCursor(0,8);
display.drawLine(0,8,display.width(),8, WHITE);
display.setTextSize(2);             // Draw 2X-scale text
display.setCursor(4,14);
display.println(moisture);
display.setCursor(30,14);
display.println("%");
display.setTextSize(1);             // Draw 2X-scale text
display.drawLine(46,8,46,46, WHITE);
display.setCursor(51,13);
display.println(" Serie riego");
display.setCursor(70,23);
display.println("num:");
display.setCursor(102,23);
display.println(attempt);

display.display();
}

///////////////////////////////////////////////////////////////////////////////////////////////TESTING LIGHTS/////////////////////////////////////////////////////////////////////////////////////////////////////



void test_lights() {
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("      Lights test ");
display.drawLine(0,8,display.width(),8, WHITE);
display.setCursor(0,18);
display.println("     ---START---");
display.display();
lights.Start();
delay(600);
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("      Lights test ");
display.drawLine(0,8,display.width(),8, WHITE);

display.setCursor(0,18);
display.println("     ---STOP---");
display.display();
lights.Stop();
delay(600);
}


///////////////////////////////////////////////////////////////////////////////////////////////TESTING WATER PUMP/////////////////////////////////////////////////////////////////////////////////////////////////////


void test_r1() {
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("    Watering test ");
display.drawLine(0,8,display.width(),8, WHITE);
display.setCursor(0,18);
display.println("     ---START---");
display.display();
r1.Start();
delay(400);

display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("    Watering test ");
display.drawLine(0,8,display.width(),8, WHITE);
display.setCursor(0,18);
display.println("     ---STOP---");
display.display();
r1.Stop();
delay(800);
}


///////////////////////////////////////////////////////////////////////////////////////////////TESTING SOILSENSOR/////////////////////////////////////////////////////////////////////////////////////////////////////


void test_soil() {
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("  The plantbot test");
display.drawLine(0,8,display.width(),8, WHITE);
display.setCursor(0,18);
display.println("     ---START---");
display.display();
//soil.start();
delay(600);
display.clearDisplay();
display.setTextSize(1);             // Draw 2X-scale text
display.setTextColor(WHITE);
display.setCursor(0,0); 
display.println("  The plantbot test");
display.drawLine(0,8,display.width(),8, WHITE);
display.setCursor(0,18);
display.println("     ---STOP---");
display.display();
//soil.stop();
delay(600);
}
