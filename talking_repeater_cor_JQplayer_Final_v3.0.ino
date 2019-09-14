
//Talking Repeater COR with RTC Clock
//Samuel Rebosura
  void(* resetFunc) (void) = 0; //software reset vector
  //#include <DFRobotDFPlayerMini.h>
 
  #include "Arduino.h"
  #include <SoftwareSerial.h>
   #include <JQ6500_Serial.h>
  #include <DS3231.h>
  #include <DTMF.h>
  #include <EEPROM.h>
// Init the DS3231 using the hardware interface
  DS3231 rtc(SDA, SCL); // RTC connected to Analog I/O A4,A5
//  #define rxPin   6  // Serial input DF player
 // #define txPin   7  // Serial output DF player
 
// set up a new serial port
    //SoftwareSerial mySoftwareSerial(6, 7); // RX, TX
    JQ6500_Serial mp3(6,7);
   // DFRobotDFPlayerMini mp3; //set Serial for DFPlayer-mini mp3 module 
    const int rx_signal =  10;
    const int tx_control =  11;
    const int tone_out = 12;
    const int trigger_led =  13;
    const int aux_control = 8;
    const int mute_control = 5; //mute control pin
    const int step_up_control = 3;
    const int step_down_control = 2;
    const int busy_pin = 4; // connected to busy pin of DFplayer
    int rx_active = 1;
    int tone_signal = 0;
    int tone_ready = 0;
    int tx_hangtime = 500; //TX hangtime in milli secs
    long TimeLastCheck = millis();
    long trigger_timeout = 90;//repeater time out variable in secs
    long TimeoutTime = 0;
    long time_var = 0;
    int trigger_loop = 0;
    int dtmf_detected = 0;
    int tx_enable = 1;
    int voice_enable = 1;
    int auto_id_active = 1; //auto ID is active only from 5AM to 12MN
    int EE_addr1 = 0; //tone select eeprom address
    int EE_addr2 = 2; //passcode eeprom address
    int EE_addr3 = 4; //aux status eeprom address
    int EEPROM_read_beep = 0;
    int admin_passcode = 0;
    int default_passcode = 2468; // set default passcode here
    int tone_enable = 0;
    int set_time_enable = 0;
    int set_date_enable = 0;
    int set_day_enable = 0;
    int set_year_enable = 0;
    int set_passcode_enable = 0;
    int announcement_enable = 0;//Club announcement disable by default 
    int admin_enable = 0; 
    int voltageInput = 1; // voltage monitor
    float vout = 0.0;
    float vin = 0.0;
    float R1 = 101500.0; // resistance of R1 (100K)
    float R2 = 9995.0; // resistance of R2 (10K)
    int value = 0;
    int low_voltage = 0;
    int aux_status = 0;                 
  int sensorPin = A0;
  int dtmf_led = 9;
  String s_hash = "#";
  char thischar;
  float n=128.0;
  // sampling rate in Hz
  float sampling_rate=8926.0;

// Instantiate the dtmf library with the number of samples to be taken
// and the sampling rate.
DTMF dtmf = DTMF(n,sampling_rate);
float d_mags[8];
String readString;
    
void setup() {
  // define pin modes DFplayer
  delay(3000);
  //pinMode(rxPin, INPUT);
  //pinMode(txPin, OUTPUT); 
  // set the data rate for the SoftwareSerial port
  //mySoftwareSerial.begin (9600);
  mp3.begin(9600);
  delay(2000);
  mp3.reset();
  delay(3000);
  mp3.setVolume(25);
  rtc.begin();
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(TUESDAY);     // Set Day-of-Week to Tuesday
  //rtc.setTime(08, 55, 0);     // Set the time to 08:55:00 (24hr format)
  //rtc.setDate(7, 8, 2019);   // Set the date to August 7, 2019 
  delay(2000); // additional delay for setup
  Serial.begin(9600);
  pinMode(tx_control,OUTPUT);
  pinMode(trigger_led, OUTPUT);
  pinMode(rx_signal, INPUT_PULLUP);
  pinMode(dtmf_led, OUTPUT);
  pinMode(aux_control, OUTPUT);
  pinMode(mute_control, OUTPUT); //set mute control as output
  pinMode(step_up_control, OUTPUT);
  pinMode(step_down_control, OUTPUT);
  pinMode(busy_pin, INPUT_PULLUP); // busy pin input no internal pull-up
  EEPROM.get(EE_addr3,aux_status); //read from aux status  in eeprom
  //if (aux_status != 0 && aux_status != 1){aux_status = 0;}//For initial start-up, assuming EEPROM data is FF hex.
  digitalWrite(aux_control,aux_status); 
  digitalWrite(tx_control,LOW);//make sure all output pins are low after boot  
  digitalWrite(trigger_led,LOW);
  digitalWrite(mute_control,LOW); //set mute control initially to low at startup
  digitalWrite(step_up_control,LOW);
  digitalWrite(step_down_control,LOW);
  pinMode(voltageInput, INPUT);
  EEPROM.get(EE_addr1,EEPROM_read_beep); //read from beep option in eeprom
  if (EEPROM_read_beep > 5){EEPROM_read_beep = 1;}//For initial start-up, assuming EEPROM data is FF hex.
  tone_enable =  EEPROM_read_beep;  
  EEPROM.get(EE_addr2,admin_passcode); //read from passcode saved in eeprom
  //Serial.println();
  //Serial.println(F("DFPlayer"));
  //Serial.println(F("Initializing"));
  //delay(6000); // delay for mcard to load up

  //if (!mp3.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    //Serial.println(F("Unable to begin:"));
    //Serial.println(F("1.Re-check the connection!"));
    //Serial.println(F("2.Insert the SD card!"));
    //while(true);
    //int tone_enable = 0;
    //int voice_enable = 0;
  //}
  //Serial.println(F("DFPlayer ready"));
  //mp3.volume(25);  //Set volume value. From 0 to 30
  
  Serial.println("COR Ready!");
  //Serial.println(rtc.getDateStr());
  //Serial.println(rtc.getTimeStr());
  //Serial.println(rtc.getDOWStr());
  delay(2000);
  tx_on();
   mp3.playFileByIndexNumber(96);
  delay(5000);
   mp3.playFileByIndexNumber(77); 
  delay(1000);
  tx_off();
}

void loop() {
  
  if (trigger_loop <=74)  {
  dtmf_decode();
  read_voltage();
  trigger_control();
 
  }
  if (trigger_loop == 75)  {
  trigger_loop = 0;
  }
  check_time();
}

void trigger_control() {
  if (digitalRead(rx_signal) == 0 && tone_ready == 1)  {
    check_timeout();
    return;
  }
  if (digitalRead(rx_signal) == 0 && tx_enable == 1)  {
    mute_on(); //COS received, mute is ON
    tx_on();
    Serial.println("RX detected");
    tone_ready = 1;
    set_timeout();       
   }
  if (digitalRead(rx_signal) == 1 && tone_ready == 1)  {
    mute_off(); //COS off, mute signal off
    if (trigger_loop == 25)  {
    delay(250);
     mp3.playFileByIndexNumber(62); //Club ID every 25 trigger
    delay(4500); //length of club ID in msec
    }
    if (trigger_loop == 49 && voice_enable == 1)  {
    delay(150);
    time_check();
    }
    if (trigger_loop == 74 && voice_enable == 1)  {
    delay(150);
     mp3.playFileByIndexNumber(96); //Club ID next 25 trigger
    delay(4500); //length of club ID2 in msec
    }
    delay(500);
    if (tone_enable == 1) {//rotation beep 
    send_tone();
    }
    if (tone_enable == 0) {//beep off
    delay(600);
    tone_ready = 0;
    }
    if (tone_enable == 2) {//change tone number below if you want to re-order
    tone4();
    }
    if (tone_enable == 3) {
    tone2();
    }
    if (tone_enable == 4) {
    tone1();
    }
    if (tone_enable == 5) {
    tone7();
    }
    delay(100);
      if ( dtmf_detected == 1)  {
      delay(250);
      tx_on();
      dtmf_detected = 0;
       mp3.playFileByIndexNumber(95);
      delay(2500);
      if (tone_enable == 0) {
       mp3.playFileByIndexNumber(98);
      delay(1500);
      }
      if (tone_enable == 1) {
       mp3.playFileByIndexNumber(97);
      delay(1500);
      }
      if (tone_enable == 2) {
       mp3.playFileByIndexNumber(94);
      delay(1500);
       mp3.playFileByIndexNumber(99);
      delay(1500);
       mp3.playFileByIndexNumber(1);
      delay(750);
      }
      if (tone_enable == 3) {
       mp3.playFileByIndexNumber(94);
      delay(1500);
       mp3.playFileByIndexNumber(99);
      delay(1500);
       mp3.playFileByIndexNumber(2);
      delay(750);
      }
      if (tone_enable == 4) {
       mp3.playFileByIndexNumber(94);
      delay(1500);
       mp3.playFileByIndexNumber(99);
      delay(1500);
       mp3.playFileByIndexNumber(3);
      delay(750);
      }
      if (tone_enable == 5) {
       mp3.playFileByIndexNumber(94);
      delay(1500);
       mp3.playFileByIndexNumber(99);
      delay(1500);
       mp3.playFileByIndexNumber(4);
      delay(750);
      }
      
      }
      
     if ( dtmf_detected == 2)  {
      delay(250);
      tx_on();
      dtmf_detected = 0;
       mp3.playFileByIndexNumber(95);
      delay(2500);
      if (tx_enable == 0) {
        mp3.playFileByIndexNumber(56);
      delay(2000);
      }
      if (tx_enable == 1) {
        mp3.playFileByIndexNumber(55);
      delay(2000);
     }
     delay(250);
     }
    tx_off();
    readString="";
    trigger_loop = trigger_loop+1;
    //Serial.println(trigger_loop);
    tone_signal = tone_signal+1;
    if (tone_signal == 8) {
      tone_signal = 0;
    }
   
    set_timeout();
   } 
  //clear DTMF String on RX stanby / TX is off 
  if (digitalRead(rx_signal) == 0 && tx_enable == 0)  {
    Serial.println("RX detected");
    Serial.println("TX disabled!");
     if ( dtmf_detected == 2)  {
      delay(250);
      if (tx_enable == 1) {
        tx_on();
         mp3.playFileByIndexNumber(95);
        delay(2500);
         mp3.playFileByIndexNumber(55);
        delay(2500);
        tx_off();
     }
     dtmf_detected == 0;
     tone_ready == 0;
     }     
  }
  if (digitalRead(rx_signal) == 1 && tone_ready == 0)  {
    return;        
  } 
}


void send_tone() {//this is the part where the multiple beep seaquence is ordered
 switch (tone_signal) {
      case 0:
        tone0();
        break;
      case 1:
        tone1();
        break;
      case 2:
        tone2();
        break;
      case 3:
        tone3();
        break;
      case 4:
        tone4();
        break;
      case 5:
        tone5();
        break;
      case 6:
        tone6();
        break;
      case 7:
        tone7();
        break;
 }
}

void tone0() {//you can change the beep files from below functions
       mp3.playFileByIndexNumber(76);
      delay(750);
      tone_ready = 0;
      //Serial.println("Tone_0");    
}
void tone1() {
      mp3.playFileByIndexNumber(67);
      delay(750);
      tone_ready = 0;
      //Serial.println("Tone_1");
}
void tone2() {
       mp3.playFileByIndexNumber(68);
      delay(750);
      tone_ready = 0;
      //Serial.println("Tone_2");
}
void tone3() {
      mp3.playFileByIndexNumber(70);
     delay(750);
     tone_ready = 0;
      //Serial.println("Tone_3");
}
void tone4() {
       mp3.playFileByIndexNumber(72);
      delay(650);
      tone_ready = 0;
      //Serial.println("Tone_4");    
}
void tone5() {
       mp3.playFileByIndexNumber(85);
      delay(750);
      tone_ready = 0;
      //Serial.println("Tone_5");
}
void tone6() {
       mp3.playFileByIndexNumber(71);
      delay(650);
      tone_ready = 0;
      //Serial.println("Tone_6");
}
void tone7() {
       mp3.playFileByIndexNumber(64);
      delay(750);
      tone_ready = 0;
      //Serial.println("Tone_7");
}
void tone8() {
       mp3.playFileByIndexNumber(84);
      delay(750);
      tone_ready = 0;
      
}
 void set_timeout() {
   
    TimeoutTime = trigger_timeout*1000;
    TimeLastCheck = millis();
    //Serial.println("Set_time_out");
             
  }

  void check_timeout() {
    TimeoutTime -= ((millis() - TimeLastCheck));
    TimeLastCheck = millis();
    //Serial.println(TimeoutTime);
    if (TimeoutTime <= 0) {
      delay(640);
      
       mp3.playFileByIndexNumber(84);//warning beep
      delay(750);
       mp3.playFileByIndexNumber(56);//say repeater timeout
      delay(2500);
      tx_off();
      delay(30000); // Repeater rest time when a timeout happen
      }
    }

void dtmf_decode()  {

      /* while(1) */dtmf.sample(sensorPin);
 
  dtmf.detect(d_mags,512);

  thischar = dtmf.button(d_mags,1800.);
  if(thischar) {
    Serial.println(thischar);
    
    char C = thischar;
    digitalWrite(dtmf_led,HIGH);
    delay(150);
    digitalWrite(dtmf_led,LOW);
    readString += C;
    if (readString == "#") {
      digitalWrite(tx_control,LOW);
      digitalWrite(trigger_led,LOW);
    }
  }
  if (readString.length() == 4 && set_time_enable == 1) {
    mute_off();
    //Serial.println(readString);
    set_hour();
    readString="";  
  }
  else if (readString.length() == 4 && set_date_enable == 1) {
    mute_off();
    //Serial.println(readString);
    set_date();
    readString="";  
  }
  else if (readString.length() == 4 && set_year_enable == 1) {
    mute_off();
    //Serial.println(readString);
    set_year();
    readString="";  
  }
  else if (readString.length() == 4 && set_day_enable == 1) {
    mute_off();
    //Serial.println(readString);
    set_day();
    readString="";  
  }
  else if (readString.length() == 4 && set_passcode_enable == 1) {
    mute_off();
    //Serial.println(readString);
    set_passcode();
    readString="";  
  }
  if (readString.length() == 5) {
    if (readString == "#6470") {//Beep Select 0-4
       mute_off();
       tone_enable = 0;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone off");
       dtmf_detected = 1;
    }
    if (readString == "#6471") {
       mute_off();
       tone_enable = 1;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       trigger_loop = 0;
       //Serial.println("tone on");
       dtmf_detected = 1;
    }
    if (readString == "#6472") {
       mute_off();
       tone_enable = 2;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone4 on");
       dtmf_detected = 1;
    }
    if (readString == "#6473") {
       mute_off();
       tone_enable = 3;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone2 on");
       dtmf_detected = 1;
    }
    if (readString == "#6474") {
       mute_off();
       tone_enable = 4;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone1 on");
       dtmf_detected = 1;
    }
    if (readString == "#6475") {
       mute_off();
       tone_enable = 5;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone7 on");
       dtmf_detected = 1;
    }
    if (readString == "#7530" && admin_enable == 1) { // repeater off
       mute_off();
       tx_enable = 0;
       //Serial.println("tx off");
       dtmf_detected = 2;
    }
    if (readString == "#7531" && admin_enable == 1) {
       mute_off();
       tx_enable = 1;
       //Serial.println("tx on");
       dtmf_detected = 2;
    }
    if (readString == "#1234") {
       mute_off();
       //Serial.println("TimeCheck");
       say_time();
    }
    if (readString == "#5678") {
       mute_off();
       //Serial.println("Date");
       tx_on();
       say_date();
       tx_off();   
    }
    if (readString == "#4321") {//Welcome Club ID 1
       mute_off();
       tx_on();
       delay (1000);
       mp3.playFileByIndexNumber(93);
       //delay(5000);
       check_busy(); 
       tx_off();
    }
    if (readString == "#5930") {//Club ID2
       mute_off();
       tx_on();
       delay (1000);
       mp3.playFileByIndexNumber(60);
       //delay(6000);
       check_busy();
       tx_off();
    }
    if (readString == "#5931") {//Club ID3
       mute_off();
       tx_on();
       delay (1000);
       mp3.playFileByIndexNumber(61);
       //delay(5000);
       check_busy();
       tx_off();
    }
    if (readString == "#5932") {//Club ID3
       mute_off();
       tx_on();
       delay (1000);
       mp3.playFileByIndexNumber(62);
       //delay(5000);
       check_busy();
       tx_off();
    }
    if (readString == "#5933") {//Club announcement
       tx_on();
       delay (1000);
       mp3.playFileByIndexNumber(115);
      //delay(5000);
      check_busy();
      tx_off();
    }
    if (readString == "#5934") {//Club ID5
       mute_off();
       tx_on();
       delay (1000);
       mp3.playFileByIndexNumber(96);
      //delay(5000);
      check_busy();
      tx_off();
    }
    if (readString == "#9430") {//Club announcement disable
       mute_off();
       tx_on();
       delay (1000);
       announcement_enable = 0;
       mp3.playFileByIndexNumber(116);
       delay(2000);
       tx_off();
    }  
    if (readString == "#9431") {//Club announcement enable
       mute_off();
       tx_on();
       delay (1000);
       announcement_enable = 1;
       mp3.playFileByIndexNumber(117);
       delay(2000);
       tx_off();
    }  
    if (readString == "#1590") {//auto ID disable
       mute_off();
       tx_on();
       delay (1000);
       voice_enable = 0;
       mp3.playFileByIndexNumber(110);
       delay(2500);
       tx_off();
    }  
    if (readString == "#1591") {//auto ID enabled by default
       mute_off();
       tx_on();
       delay (1000);
       voice_enable = 1;
       mp3.playFileByIndexNumber(111);
       delay(2000);
       tx_off();
    }
    if (readString == "#8520" && admin_enable == 1) {//auxiliary off
       mute_off();
       tx_on();
       digitalWrite(aux_control,LOW);
       aux_status = 0;
       EEPROM.put(EE_addr3, aux_status);//store aux status in EEPROM
       delay (1000);
       mp3.playFileByIndexNumber(108);
       delay(2500);
       tx_off();
       admin_enable = 0;
    }
    if (readString == "#8521" && admin_enable == 1) {//auxiliary on
       mute_off();
       tx_on();
       digitalWrite(aux_control,HIGH);
       aux_status = 1;
       EEPROM.put(EE_addr3, aux_status);//store aux status in EEPROM
       delay (1000);
       mp3.playFileByIndexNumber(109);//if you want to add voice confirmation insert command below.
       delay(2500);
       tx_off();
       admin_enable = 0;
    }
    if (readString == "#8522" && admin_enable == 1) {//step_up pulse on
       digitalWrite(step_up_control,HIGH);
       delay(1000);
       digitalWrite(step_up_control,LOW);
       readString="";  
       mute_off();
       tx_on();
       delay (1000);
       mp3.playFileByIndexNumber(113);//if you want to add voice confirmation insert command below.
       delay(2700);
       tx_off();
       admin_enable = 0;
    }
    if (readString == "#8523" && admin_enable == 1) {//step_down pulse on
       digitalWrite(step_down_control,HIGH);
       delay(1000);
       digitalWrite(step_down_control,LOW);
       readString="";  
       mute_off();
       tx_on();
       delay (1000);
       mp3.playFileByIndexNumber(114);//if you want to add voice confirmation insert command below.
       delay(2700);
       tx_off();
       admin_enable = 0;
    }                
    if (readString == "#1179") {//reboot and set passcode to default by dtmf
      EEPROM.put(EE_addr2,default_passcode);//Set passcode to default in EEPROM
      mute_off();
      tx_on();
      tx_enable = 1;
       //Serial.println("reboot");
       delay(640);
      mp3.playFileByIndexNumber(79);
      delay(2000);
      tx_off();
      resetFunc();
    }
    if (readString == "#7931" && admin_enable == 1) {
      mute_off();
      tx_on();
      set_time_enable = 1;
      delay(640);
      mp3.playFileByIndexNumber(79); // say access granted
      delay(2000);
      mp3.playFileByIndexNumber(100); // say enter new time
      delay(5000);
      tx_off();
      admin_enable = 0;
    }
    if (readString == "#7932" && admin_enable == 1) {
      mute_off();
      tx_on();
      set_date_enable = 1;
      delay(640);
      mp3.playFileByIndexNumber(79); // say access granted
      delay(2000);
      mp3.playFileByIndexNumber(102); // say enter new date
      delay(5000);
      tx_off();
      admin_enable = 0;
    }
    if (readString == "#7933" && admin_enable == 1) {
      mute_off();
      tx_on();
      set_year_enable = 1;
      delay(640);
      mp3.playFileByIndexNumber(79); // say access granted
      delay(2000);
      mp3.playFileByIndexNumber(104); // say enter new year
      delay(5000);
      tx_off();
      admin_enable = 0;
    }
    if (readString == "#7934" && admin_enable == 1) {
      mute_off();
      tx_on();
      set_day_enable = 1;
      delay(640);
      mp3.playFileByIndexNumber(79); // say access granted
      delay(2000);
      mp3.playFileByIndexNumber(106); // say enter new day
      delay(5000);
      tx_off();
      admin_enable = 0;
    }
    if (readString == "#1380") { // force voltage check
      mute_off();
      tx_on();
      mp3.playFileByIndexNumber(59); // say power supply
      check_busy();
      voltage_check();
      tx_off();
    }
    if (readString == s_hash + admin_passcode) { // Admin passcode
      admin_enable = 1; 
      mute_off();
      tx_on();
      mp3.playFileByIndexNumber(118); // say admin access granted
      check_busy();
      tx_off();
    }
    if (readString == "#3281" && admin_enable == 1) { // Change Admin passcode
      set_passcode_enable = 1;
      mute_off();
      tx_on();
      mp3.playFileByIndexNumber(119); // say please enter 4 digit new admin passcode
      check_busy();
      tx_off();
    }
    
  Serial.println(readString);

  readString="";
  }
  //Serial.println("DTMF listening ...."); 
    }
 

 
void say_date() 
{
  delay(500);
   mp3.playFileByIndexNumber(85);
  delay (650);
   mp3.playFileByIndexNumber(53);
  delay (1100);
  String today = rtc.getDOWStr();
  delay (200);
  if (today == "Monday") {
     mp3.playFileByIndexNumber(41);
    delay(1000);       
    }
  else if (today == "Tuesday") {
     mp3.playFileByIndexNumber(42);
    delay(1000);       
    }
  else if (today == "Wednesday") {
     mp3.playFileByIndexNumber(43);
    delay(1000);       
    }
   else if (today == "Thursday") {
     mp3.playFileByIndexNumber(44);
    delay(1000);       
    }
   else if (today == "Friday") {
     mp3.playFileByIndexNumber(45);
    delay(1000);       
    }
   else if (today == "Saturday") {
     mp3.playFileByIndexNumber(46);
    delay(1000);       
    }
   else if (today == "Sunday") {
     mp3.playFileByIndexNumber(47);
    delay(1000);       
   }
   
     
  String moString = rtc.getDateStr();
  //Serial.println(moString);
  moString.remove(0,3);
  //Serial.print(moString);
  moString.remove(2);
    if (moString == "01") {
       mp3.playFileByIndexNumber(29);
      delay(1000);  
    }
    if (moString == "02") {
       mp3.playFileByIndexNumber(30);
      delay(1000);  
    }  
    if (moString == "03") {
       mp3.playFileByIndexNumber(31);
      delay(1000);   
    }
    if (moString == "04") {
       mp3.playFileByIndexNumber(32);
      delay(1000);   
    }
    if (moString == "05") {
       mp3.playFileByIndexNumber(33);
      delay(1000);    
    }
    if (moString == "06") {
       mp3.playFileByIndexNumber(34);
      delay(1000);    
    }
    if (moString == "07") {
       mp3.playFileByIndexNumber(35);
      delay(1000);  
    }
    if (moString == "08") {
       mp3.playFileByIndexNumber(36);
      delay(1000);   
    }
    if (moString == "09") {
       mp3.playFileByIndexNumber(37);
      delay(1000);   
    }
    if (moString == "10") {
       mp3.playFileByIndexNumber(38);
      delay(1000); 
    }
    if (moString == "11") {
       mp3.playFileByIndexNumber(39);
      delay(1000);    
    }
    if (moString == "12") {
       mp3.playFileByIndexNumber(40);
      delay(1000);  
    }    
  
  String dtString = rtc.getDateStr();
  dtString.remove(2);
  int dates = dtString.toInt();
  if (dates < 21){ mp3.playFileByIndexNumber(dates); delay(750);}
  else if (dates == 20){ mp3.playFileByIndexNumber(20); delay(650);}
  else if (dates > 20 && dates <30){ mp3.playFileByIndexNumber(20); delay(600); mp3.playFileByIndexNumber(dates-20); delay(650);}
  else if (dates == 30){ mp3.playFileByIndexNumber(21); delay(650);}
  else if (dates > 30 && dates <40){ mp3.playFileByIndexNumber(21); delay(600); mp3.playFileByIndexNumber(dates-30); delay(650);}
  delay (500);
   mp3.playFileByIndexNumber(77);
  delay (650);
}

void say_time()
{
 tx_on();
 time_check();
 tx_off();
}

void time_check()
{
  delay(750);
   mp3.playFileByIndexNumber(85);
  delay(1000);
   mp3.playFileByIndexNumber(87);
  delay(1000);
  String hrString = rtc.getTimeStr();
  hrString.remove(3);
  String minString = rtc.getTimeStr();
  minString.remove(6);
  minString.remove(0,3);
  int mins = minString.toInt();
  int hrs = hrString.toInt();
  if (hrs == 0){ mp3.playFileByIndexNumber(12);delay(750);}
  else if (hrs < 12){ mp3.playFileByIndexNumber(hrs);delay(850);}
  else if (hrs == 12){ mp3.playFileByIndexNumber(12);delay(850);}
  else if (hrs > 12){ mp3.playFileByIndexNumber(hrs-12);delay(850);}
  if (mins == 0){delay(650);}
  else if (mins < 10){ mp3.playFileByIndexNumber(91); delay(850);  mp3.playFileByIndexNumber(mins); delay(850);}
  else if (mins == 10){ mp3.playFileByIndexNumber(10); delay(750);}
  else if (mins > 10 && mins < 20){ mp3.playFileByIndexNumber(mins); delay(850);}
  else if (mins == 20){ mp3.playFileByIndexNumber(20); delay(750);}
  else if (mins > 20 && mins <30){ mp3.playFileByIndexNumber(20); delay(750); mp3.playFileByIndexNumber(mins-20); delay(750);}
  else if (mins == 30){ mp3.playFileByIndexNumber(21); delay(750);}
  else if (mins > 30 && mins <40){ mp3.playFileByIndexNumber(21); delay(750); mp3.playFileByIndexNumber(mins-30); delay(750);}
  else if (mins == 40){ mp3.playFileByIndexNumber(22); delay(750);}
  else if (mins > 40 && mins <50){ mp3.playFileByIndexNumber(22); delay(750); mp3.playFileByIndexNumber(mins-40); delay(750);}
  else if (mins == 50){ mp3.playFileByIndexNumber(23); delay(750);}
  else if (mins > 50 && mins <60){ mp3.playFileByIndexNumber(23); delay(750); mp3.playFileByIndexNumber(mins-50); delay(750);}
  if (hrs < 12){ mp3.playFileByIndexNumber(89);delay(1100);}//say AM
  else if (hrs == 12){ mp3.playFileByIndexNumber(90);delay(1100);}//say PM
  else if (hrs > 12){ mp3.playFileByIndexNumber(90);delay(1100);}//say PM
   mp3.playFileByIndexNumber(77);//play bleep
  delay (750);
}
void check_time ()
{
 if (digitalRead(rx_signal) == 0 || tx_enable == 0)  {//If COS is available or TX is disable, no time check and autoID
    return;        
  } 
 String minString = rtc.getTimeStr();
  //minString.remove(6);
  minString.remove(0,3);
  //int mins = minString.toInt(); 
  if ((minString == "15:00" || minString == "30:00" || minString == "45:00" || minString == "59:00") && voice_enable == 1 && auto_id_active == 1){
    //if auto ID is disabled will still perform hourly time check.
      tx_on();
      delay(500);
       mp3.playFileByIndexNumber(60);//Club ID every 15 mins
      delay(5500); 
      tx_off();     
 }
 String hrString = rtc.getTimeStr();
 if (hrString == "05:00:00") { 
  auto_id_active = 1;
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(48); //say good morning
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(5); // say 5
  delay(1100);
    mp3.playFileByIndexNumber(89); // say AM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "06:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(48); //say good morning
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(6); // say 6
  delay(1100);
    mp3.playFileByIndexNumber(89); // say AM
  delay(1000);
  say_date(); //date of the day announcement
  check_announcement();
  tx_off();
 }
 if (hrString == "07:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(48); //say good morning
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(7); // say 7
  delay(1100);
    mp3.playFileByIndexNumber(89); // say AM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "08:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(48); //say good morning
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(8); // say 8
  delay(1100);
    mp3.playFileByIndexNumber(89); // say AM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "09:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85);
  delay(1000);
   mp3.playFileByIndexNumber(54);
  delay(2500);
   mp3.playFileByIndexNumber(9);
  delay(1100);
    mp3.playFileByIndexNumber(89);
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "10:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(48); //say good morning
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(10); // say 10
  delay(1100);
    mp3.playFileByIndexNumber(89); // say AM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "11:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(48); //say good morning
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(11); // say 11
  delay(1100);
    mp3.playFileByIndexNumber(89); // say AM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "12:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85);
  delay(1000);
   mp3.playFileByIndexNumber(88);
  delay(1000);
   mp3.playFileByIndexNumber(87);
  delay(1000);
   mp3.playFileByIndexNumber(12);
  delay(750);
    mp3.playFileByIndexNumber(52); //say o'clock
  delay(1000);
  say_date(); //say date of the day
  check_announcement();
  tx_off();
 }
 if (hrString == "13:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(49); //say good afternoon
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(1); //say 1
  delay(750);
    mp3.playFileByIndexNumber(90); //say PM
  delay(1000);
  check_announcement();
  tx_off();
 }
  if (hrString == "14:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(49); //say good afternoon
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(2); //say 2
  delay(750);
    mp3.playFileByIndexNumber(90); //say PM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "15:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85);
  delay(1000);
   mp3.playFileByIndexNumber(49);
  delay(1000);
   mp3.playFileByIndexNumber(87);
  delay(1000);
   mp3.playFileByIndexNumber(3);
  delay(750);
   mp3.playFileByIndexNumber(90);
  delay(1000);
  check_announcement();
  tx_off();
 }
  if (hrString == "16:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(49); //say good afternoon
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(4); //say 4
  delay(750);
    mp3.playFileByIndexNumber(90); //say PM
  delay(1000);
  say_date(); //say date of the day
  check_announcement();
  tx_off();
 }
 if (hrString == "17:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(49); //say good afternoon
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(5); //say 5
  delay(750);
    mp3.playFileByIndexNumber(90); //say PM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "18:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85);
  delay(1000);
   mp3.playFileByIndexNumber(50);
  delay(1000);
   mp3.playFileByIndexNumber(87);
  delay(1000);
   mp3.playFileByIndexNumber(6);
  delay(750);
    mp3.playFileByIndexNumber(90);
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "19:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(50); //say good evening
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(7); // say 7
  delay(1100);
    mp3.playFileByIndexNumber(90); // say PM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "20:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(50); //say good evening
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(8); // say 8
  delay(1100);
    mp3.playFileByIndexNumber(90); // say PM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "21:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(50); //say good evening
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(9); // say 9
  delay(1100);
    mp3.playFileByIndexNumber(90); // say PM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "22:00:00") { 
  delay(500);
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(50); //say good evening
  delay(1000);
   mp3.playFileByIndexNumber(87); //say time is
  delay(1000);
   mp3.playFileByIndexNumber(10); // say 10
  delay(1100);
    mp3.playFileByIndexNumber(90); // say PM
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "23:00:00") {
  delay(500); 
  tx_on();
   mp3.playFileByIndexNumber(85);
  delay(1000);
   mp3.playFileByIndexNumber(51);
  delay(3000);
   mp3.playFileByIndexNumber(87);
  delay(1000);
   mp3.playFileByIndexNumber(11);
  delay(1000);
    mp3.playFileByIndexNumber(90);
  delay(1000);
  check_announcement();
  tx_off();
 }
 if (hrString == "00:00:00") {
  delay(500); 
  tx_on();
   mp3.playFileByIndexNumber(85); //play bleep
  delay(1000);
   mp3.playFileByIndexNumber(48); // say good morning
  delay(1000);
   mp3.playFileByIndexNumber(87); // say time is
  delay(1000);
   mp3.playFileByIndexNumber(12); //say 12
  delay(1000);
    mp3.playFileByIndexNumber(89);//say AM
  delay(1000);
  auto_id_active = 0;
  check_announcement();
  int announcement_enable = 0; //disabled after 12 midnight until enabled again by dtmf
  tx_off();
 }
 if (hrString == "01:00:00" || hrString == "02:00:00" || hrString == "03:00:00" || hrString == "04:00:00") {
  auto_id_active = 0;  
 }
}  

void voice_msg(int msg) //not in use for futute update custom message
{   
   mp3.playFileByIndexNumber(85);
  delay(1500);
   mp3.playFileByIndexNumber(msg);
  delay(5000);
}
void mute_on(){
  digitalWrite(mute_control,HIGH);
}
void mute_off(){
  digitalWrite(mute_control,LOW);
}

void tx_on(){//trigger control
  digitalWrite(tx_control,HIGH);
  digitalWrite(trigger_led,HIGH); 
}
void tx_off(){
  if (low_voltage == 1){
     mp3.playFileByIndexNumber(59); //Change MP3 file to say LOW Voltage!
     delay(1000);
  }
  delay(tx_hangtime);//hang time 
  digitalWrite(tx_control,LOW);
  digitalWrite(trigger_led,LOW);
}
void set_hour(){
//Serial.println("Setting Hour");
    //rtc.setDOW(TUESDAY);     // Set Day-of-Week to Tuesday
    //rtc.setTime(08, 55, 0);     // Set the time to 08:55:00 (24hr format)
    //rtc.setDate(7, 8, 2019);   // Set the date to August 7, 2019 
    String New_Hr = readString;       
    String New_Min = readString;
    New_Hr.remove(2);
    int Set_Hr = New_Hr.toInt();  
    New_Min.remove(0,2);
    int Set_Min = New_Min.toInt();
    rtc.setTime(Set_Hr, Set_Min, 0); 
    set_time_enable = 0;
    //Serial.println("Setting hour ....");
    //Serial.println(rtc.getDateStr());
    //Serial.println(rtc.getTimeStr());
    //Serial.println(rtc.getDOWStr());
    delay (1000);  
    tx_on();
    delay (1000);
     mp3.playFileByIndexNumber(101);//say set new time confirmed
    delay(5000);
    say_time(); 
    tx_off();
}

void set_date(){
  //Serial.println("Setting Hour");
    //rtc.setDOW(TUESDAY);     // Set Day-of-Week to Tuesday
    //rtc.setTime(08, 55, 0);     // Set the time to 08:55:00 (24hr format)
    //rtc.setDate(7, 8, 2019);   // Set the date to August 7, 2019
    String YearString = rtc.getDateStr();
    YearString.remove(0,6);
    int Set_Year = YearString.toInt();
    String New_Mo = readString;       
    String New_Day = readString;
    New_Mo.remove(2);
    int Set_Mo = New_Mo.toInt();  
    New_Day.remove(0,2);
    int Set_Day = New_Day.toInt();
    rtc.setDate(Set_Day, Set_Mo, Set_Year); 
    set_date_enable = 0;
    //Serial.println("Setting date ....");
    //Serial.println(rtc.getDateStr());
    //Serial.println(rtc.getTimeStr());
    //Serial.println(rtc.getDOWStr());
    //delay (1000);  
    tx_on();
    delay (1000);
     mp3.playFileByIndexNumber(103);//say set new date confirmed
    delay(5000); 
    say_date();
    tx_off();   
}
void set_year(){
  //Serial.println("Setting Hour");
    //rtc.setDOW(TUESDAY);     // Set Day-of-Week to Tuesday
    //rtc.setTime(08, 55, 0);     // Set the time to 08:55:00 (24hr format)
    //rtc.setDate(7, 8, 2019);   // Set the date to August 7, 2019
    String MoString = rtc.getDateStr();
    MoString.remove(0,3);
    MoString.remove(2);
    int Set_Mo = MoString.toInt();
    String DayString = rtc.getDateStr();
    DayString.remove(2);
    int Set_Day = DayString.toInt();
    String New_Year = readString;       
    int Set_Year = New_Year.toInt();
    rtc.setDate(Set_Day, Set_Mo, Set_Year); 
    set_year_enable = 0;
    //Serial.println("Setting year ....");
    //Serial.println(rtc.getDateStr());
    //Serial.println(rtc.getTimeStr());
    //Serial.println(rtc.getDOWStr());
    delay (1000);  
    tx_on();
    delay (1000);
     mp3.playFileByIndexNumber(105);//say set new date confirmed
    delay(5000); 
    tx_off();   
}
void set_day(){
    
  String New_Day = readString;
  
  if (New_Day == "0001") {
    rtc.setDOW(MONDAY);         
    }
  else if (New_Day == "0002") {
    rtc.setDOW(TUESDAY);         
    }
  else if (New_Day == "0003") {
    rtc.setDOW(WEDNESDAY);         
    }
  else if (New_Day == "0004") {
    rtc.setDOW(THURSDAY);         
    }
  else if (New_Day == "0005") {
    rtc.setDOW(FRIDAY);         
    }    
  else if (New_Day == "0006") {
    rtc.setDOW(SATURDAY);         
    }
  else if (New_Day == "0007") {
    rtc.setDOW(SUNDAY);         
    }
    set_day_enable = 0;
    //Serial.println("Setting day ....");
    //Serial.println(rtc.getDateStr());
    //Serial.println(rtc.getTimeStr());
    //Serial.println(rtc.getDOWStr());  
    delay (1000);  
    tx_on();
    delay (1000);
     mp3.playFileByIndexNumber(107);//say set new day confirmed
    delay(5000);
    say_date(); 
    tx_off();   
}
void set_passcode(){
  String new_passcode = readString;
  admin_passcode = new_passcode.toInt();
  EEPROM.put(EE_addr2, admin_passcode);//store new passcode in EEPROM
  set_passcode_enable = 0;
  delay (1000);  
  tx_on();
  delay (1000);
  mp3.playFileByIndexNumber(120);//say passcode change confirmed 
  check_busy();
  tx_off(); 
  admin_enable = 0;
}
void voltage_check(){
  read_voltage();
  //delay(750);
  //mp3.playFileByIndexNumber(85);
  delay(1000);
  mp3.playFileByIndexNumber(59);
  delay(1000);
  String s_tens = String(vin,2);
   s_tens.remove(2);
   //Serial.println(s_tens);
  String s_dec = String(vin,2);
   
   if (s_dec.length() == 4){ 
    s_dec.remove(0,2);
    //Serial.println("SLength = 3");
    //Serial.println(s_dec);
   }
   if (s_dec.length() == 5){ 
    s_dec.remove(0,3);
     //Serial.println("SLength = 4");
     //Serial.println(s_dec);
   }
   
   
   int tens = s_tens.toInt();
   int dec = s_dec.toInt();
   //Serial.println(tens);
   //Serial.println(dec);
   
  if (tens == 0){mp3.playFileByIndexNumber(91);delay(750);}
  else if (tens < 21){mp3.playFileByIndexNumber(tens); delay(750);}
  else if (tens > 20 && tens <30){mp3.playFileByIndexNumber(20); delay(600);mp3.playFileByIndexNumber(tens-20); delay(650);}
  else if (tens == 30){mp3.playFileByIndexNumber(30); delay(650);}
  else if (tens > 30 && tens <40){mp3.playFileByIndexNumber(21); delay(600);mp3.playFileByIndexNumber(tens-30); delay(650);}
  mp3.playFileByIndexNumber(57);//play point
  delay (750);
  if (dec == 0){mp3.playFileByIndexNumber(91);delay(750);}
  else if (dec < 10){mp3.playFileByIndexNumber(91); delay(850); mp3.playFileByIndexNumber(dec); delay(850);}
  else if (dec == 10){mp3.playFileByIndexNumber(10); delay(750);}
  else if (dec > 10 && dec < 20){mp3.playFileByIndexNumber(dec); delay(850);}
  else if (dec == 20){mp3.playFileByIndexNumber(20); delay(750);}
  else if (dec > 20 && dec <30){mp3.playFileByIndexNumber(20); delay(750);mp3.playFileByIndexNumber(dec-20); delay(750);}
  else if (dec == 30){mp3.playFileByIndexNumber(21); delay(750);}
  else if (dec > 30 && dec <40){mp3.playFileByIndexNumber(21); delay(750);mp3.playFileByIndexNumber(dec-30); delay(750);}
  else if (dec == 40){mp3.playFileByIndexNumber(22); delay(750);}
  else if (dec > 40 && dec <50){mp3.playFileByIndexNumber(22); delay(750);mp3.playFileByIndexNumber(dec-40); delay(750);}
  else if (dec == 50){mp3.playFileByIndexNumber(23); delay(750);}
  else if (dec > 50 && dec <60){mp3.playFileByIndexNumber(23); delay(750);mp3.playFileByIndexNumber(dec-50); delay(750);}
  else if (dec == 60){mp3.playFileByIndexNumber(24); delay(750);}
  else if (dec > 60 && dec <70){mp3.playFileByIndexNumber(24); delay(750);mp3.playFileByIndexNumber(dec-60); delay(750);}
  else if (dec == 70){mp3.playFileByIndexNumber(25); delay(750);}
  else if (dec > 70 && dec <80){mp3.playFileByIndexNumber(25); delay(750);mp3.playFileByIndexNumber(dec-70); delay(750);}
  else if (dec == 80){mp3.playFileByIndexNumber(26); delay(750);}
  else if (dec > 80 && dec <90){mp3.playFileByIndexNumber(26); delay(750);mp3.playFileByIndexNumber(dec-80); delay(750);}
  else if (dec == 90){mp3.playFileByIndexNumber(27); delay(750);}
  else if (dec > 90 && dec <100){mp3.playFileByIndexNumber(27); delay(750);mp3.playFileByIndexNumber(dec-90); delay(750);}
 
  mp3.playFileByIndexNumber(58);//play volt
  delay (750);
 }

void read_voltage(){ // read the value at analog input
      if (tx_enable == 0){
      return;
      }
       float min_voltage = 10.6; //change here to set the low voltage limit
       value = analogRead(voltageInput);
       vout = ((value) * 5.0) / 1024.0; 
       vin = vout / (R2/(R1+R2)); 
       if (vin < min_voltage){
       low_voltage = 1;
       }
       else if (vin > min_voltage){
       low_voltage = 0;
       }
}
void check_busy(){//will just loop into checking the busy pin every 200msec and exit
  delay(100);
  while (digitalRead (busy_pin) == 0){
    delay(200); 
  }
  delay (100);
}

void check_announcement(){
  if (announcement_enable == 1){       
       delay(2000);
       mp3.playFileByIndexNumber(115); //Play Anouncement file when enabled by DTMF code
       check_busy();
       tx_off();
  }
}

