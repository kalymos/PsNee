//    ATTYNee!!! Universal mod for PS1 & PSOne.
//--------------------------------------------------
//                  Pinouts!
//--------------------------------------------------
//FOR ARDUINO UNO (WITH ATMEGA328):
// - Arduino pin 8  = data    = ATMega pin 14
// - Arduino pin 9  = gate    = ATMega pin 15
// - Arduino pin 10 = lid     = ATMega pin 16
// - Arduino pin 11 = biosA18  = ATMega pin 2
// - Arduino pin 12 = biosD2  = ATMega pin 18

//FOR ATTINY13/25/45/85:
// - PB0 = data     = ATTiny pin 5
// - PB1 = gate     = ATTiny pin 6
// - PB2 = lid      = ATTiny pin 7
// - PB3 = biosA18  = ATTiny pin 2
// - PB4 = biosD2   = ATTiny pin 3


//--------------------------------------------------
//               Chip selection!
//--------------------------------------------------
#define ARDUINO_UNO        //Make that "#define ARDUINO_UNO" if you want to compile for Arduino Uno instead of ATTiny25/45/85

#ifdef ARDUINO_UNO
//Pins
unsigned char data = 8;         //The pin that outputs the SCEE SCEA SCEI string
unsigned char gate = 9;         //The pin that outputs the SCEE SCEA SCEI string
unsigned char lid = 10;         //The pin that gets connected to the internal CD lid signal; active high
unsigned char biosA18 = 11;     //Only used in SCPH-102 PAL mode
unsigned char biosD2 = 12;      //Only used in SCPH-102 PAL mode
unsigned short delay_ntsc = 2350;
boolean injectFlag = 0;
unsigned char delay_between_bits = 4;
unsigned char delay_between_injections = 74;
#endif

#ifdef ATTiny25/45/85
//Pins
unsigned char data = 0;        //The pin that outputs the SCEE SCEA SCEI string
unsigned char gate = 1;
unsigned char lid = 2;         //The pin that gets connected to the internal CD lid signal; active high
unsigned char biosA18 = 3;     //Only used in SCPH-102 PAL mode
unsigned char biosD2 = 4;      //Only used in SCPH-102 PAL mode
unsigned short delay_ntsc = 2400;
boolean injectFlag = 0;
unsigned char delay_between_bits = 4;
unsigned char delay_between_injections = 68;
#endif

#ifdef ATTiny13A
//Pins
unsigned char data = 0;        //The pin that outputs the SCEE SCEA SCEI string
unsigned char gate = 1;
unsigned char lid = 2;         //The pin that gets connected to the internal CD lid signal; active high
unsigned char biosA18 = 3;     //Only used in SCPH-102 PAL mode
unsigned char biosD2 = 4;      //Only used in SCPH-102 PAL mode
unsigned short delay_ntsc = 2400;
boolean injectFlag = 0;
unsigned char delay_between_bits = 4;
unsigned char delay_between_injections = 72;
#endif

bool readBit(int index, const unsigned char *ByteSet)
{
  int byte_index = index >> 3;
  byte bits = pgm_read_byte(&(ByteSet[byte_index]));
  int bit_index = index & 0x7; // same as (index - byte_index<<3) or (index%8)
  byte mask = 1 << bit_index;
  return (0 != (bits & mask));
}
void NTSC_fix()
{
  DDRB = 0x00;
  delay(delay_ntsc);
  while (!(PINB & 0b00001000))
  {
    ;  //Wait
  }
  delayMicroseconds(12);
  PORTB = 0b00000000;
  DDRB = 0b00010000;
  delayMicroseconds(5);
  DDRB = 0x00;
  //Serial.println("NTSC_fixed!");  //Debug only
}

void inject_SCEE()
{
  static const unsigned char ByteSet[] PROGMEM = {0b01011001, 0b11001001, 0b01001011, 0b01011101, 0b11101010, 0b00000010};
  //Serial.println("inject_SCEE");  //Debug only
  for (unsigned char i = 0; i < 44; i++) {
    if (readBit(i, ByteSet) == 0)
    {
      // Serial.print(0);  //Debug only
      pinMode(data, OUTPUT);
      digitalWrite(data, 0);
      delay(delay_between_bits);
    }
    else
    {
      // Serial.print(1);  //Debug only
      pinMode(data, INPUT);
      delay(delay_between_bits);
    }
  }

  pinMode(data, OUTPUT);
  digitalWrite(data, 0);
  delay(delay_between_injections);
}

void inject_SCEA()
{
  static const unsigned char ByteSet[] PROGMEM = {0b01011001, 0b11001001, 0b01001011, 0b01011101, 0b11111010, 0b00000010};
  // Serial.println("inject_SCEA");  //Debug only
  for (unsigned char i = 0; i < 44; i++) {
    if (readBit(i, ByteSet) == 0)
    {
      // Serial.print(0);  //Debug only
      pinMode(data, OUTPUT);
      digitalWrite(data, 0);
      delay(delay_between_bits);
    }
    else
    {
      //  Serial.print(1);  //Debug only
      pinMode(data, INPUT);
      delay(delay_between_bits);
    }
  }

  pinMode(data, OUTPUT);
  digitalWrite(data, 0);
  delay(delay_between_injections);
}

void inject_SCEI()
{
  static const unsigned char ByteSet[] PROGMEM = {0b01011001, 0b11001001, 0b01001011, 0b01011101, 0b11011010, 0b00000010};
  // Serial.println("inject_SCEI");  //Debug only
  for (unsigned char i = 0; i < 44; i++) {
    if (readBit(i, ByteSet) == 0)
    {
      //  Serial.print(0);  //Debug only
      pinMode(data, OUTPUT);
      digitalWrite(data, 0);
      delay(delay_between_bits);
    }
    else
    {
      //  Serial.print(1);  //Debug only
      pinMode(data, INPUT);
      delay(delay_between_bits);
    }

  }

  pinMode(data, OUTPUT);
  digitalWrite(data, 0);
  delay(delay_between_injections);

}


void inject_SCEx()
{
  unsigned char loop_counter;
  NTSC_fix(); //Only for SCPH-102 PAL. For other please, comment  this function, and uncomment next delay.
  //delay(3000);
  delay(1200);
  pinMode(gate, OUTPUT);
  digitalWrite(gate, 0);
  delay(100);

  for (loop_counter = 0; loop_counter < 40; loop_counter = loop_counter + 1)
    if (digitalRead(lid) == HIGH)
    {
      DDRB = 0x00;
      void loop();
    }
    else
    {
      inject_SCEE();
      inject_SCEA();
      inject_SCEI();
    }

  pinMode(gate, INPUT);
  pinMode(data, INPUT);
  injectFlag = 1;
  // Serial.println("inject_DONE!"); //Debug only

}


void setup()
{
  // Serial.begin(9600); //Debug only
  pinMode(lid, INPUT);
  pinMode(biosA18, INPUT);
  pinMode(data, INPUT);
  pinMode(gate, INPUT);

}

void loop()
{
  if (digitalRead(lid) == HIGH && injectFlag == 1)
  {
    // Serial.println("Wait new cycle"); //Debug only
    injectFlag = 0;
  }

  if (digitalRead(lid) == LOW && injectFlag == 0)
  {
    // Serial.println("Start Inject"); //Debug only
    inject_SCEx();
  }
  //  if (digitalRead(lid) == HIGH && injectFlag == 0)
  //  {
  //    Serial.println("Opened before injecting!"); //Debug only
  //  }
}
