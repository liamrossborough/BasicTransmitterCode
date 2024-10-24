#include <Arduino.h>

#define SERIAL_PORT_SPEED 115200 //have to change speed in platformio.ini

//includes NO Failsafe (should be included with transmitter and receiver), no transforming values
//6 Channel Receiver
//for use adjust channel pins, deadzone values, default and extrema values of transmitter, channel mode

//defines receiver channels
#define RC_NUM_CHANNELS 6
#define RC_CH1 0 //right up down 
#define RC_CH2 1 //Throttle
#define RC_CH3 2 //Yaw
#define RC_CH4 3
#define RC_CH5 4
#define RC_CH6 5

//defines channel pins on arduino (should be interrupt pins for reliable functionality)
#define RC_CH1_INPUT 1
#define RC_CH2_INPUT 2
#define RC_CH3_INPUT 3
#define RC_CH4_INPUT 4
#define RC_CH5_INPUT 5
#define RC_CH6_INPUT 6

//arrays for pulse start and width
uint16_t RC_VALUES[RC_NUM_CHANNELS]; //uint16_t different way to say unsighned short/int/long etc. 8-char, 16-short, 32-int, 64-long long
uint32_t RC_START[RC_NUM_CHANNELS];
volatile uint16_t RC_SHARED[RC_NUM_CHANNELS];

//rc channel mode which type of joystick, throttle, button
// 0 = a joystick with a centrepoint
// 1 = a throttle that goes from low to high
// 2 = a switch either on or off
uint16_t RC_CHANNEL_MODE[RC_NUM_CHANNELS] = {0, 1, 0, 0, 2, 2};

//value range of transmitter (must be adjusted for each transmitter)
uint16_t RC_LOW[RC_NUM_CHANNELS] = {780, 992, 1032, 992, 992, 992};
uint16_t RC_MID[RC_NUM_CHANNELS] = {1284, 0, 1490, 1490, 0, 0};
uint16_t RC_HIGH[RC_NUM_CHANNELS] = {1836, 1990, 1955, 1948, 1948, 1948};

//What percentage deadzone is allowed
uint16_t RC_DZPERCENT[RC_NUM_CHANNELS] = {5, 5, 5, 5, 5, 5};

void READ_RC1();
void READ_RC2();
void READ_RC3();
void READ_RC4();
void READ_RC5();
void READ_RC6();
void Read_Input(uint8_t channel, uint8_t input_pin);
void rc_read_values();
void rc_deadzone_adjust();



void setup() 
{
  Serial.begin(SERIAL_PORT_SPEED);
  pinMode(RC_CH1, INPUT);
  pinMode(RC_CH2, INPUT);
  pinMode(RC_CH3, INPUT);
  pinMode(RC_CH4, INPUT);
  pinMode(RC_CH5, INPUT);
  pinMode(RC_CH6, INPUT);
  attachInterrupt(digitalPinToInterrupt(RC_CH1_INPUT), READ_RC1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH2_INPUT), READ_RC2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH3_INPUT), READ_RC3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH4_INPUT), READ_RC4, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH5_INPUT), READ_RC5, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH6_INPUT), READ_RC6, CHANGE);
}



void loop() 
{
  //read values from rc receiver
  rc_read_values();
  rc_deadzone_adjust();

  //output our values to the serial port in a format the plotter can use
  Serial.print(RC_VALUES[RC_CH1]); Serial.print(",");
  Serial.print(RC_VALUES[RC_CH2]); Serial.print(",");
  Serial.print(RC_VALUES[RC_CH3]); Serial.print(",");
  Serial.print(RC_VALUES[RC_CH4]); Serial.print(",");
  Serial.print(RC_VALUES[RC_CH5]); Serial.print(",");
  Serial.print(RC_VALUES[RC_CH6]); Serial.print("\n");
}



//called by interrupt sends all channels to pwm calculator
void READ_RC1()
{
  Read_Input(RC_CH1, RC_CH1_INPUT);
}
void READ_RC2()
{
  Read_Input(RC_CH2, RC_CH2_INPUT);
}
void READ_RC3()
{
  Read_Input(RC_CH3, RC_CH3_INPUT);
}
void READ_RC4()
{
  Read_Input(RC_CH4, RC_CH4_INPUT);
}
void READ_RC5()
{
  Read_Input(RC_CH5, RC_CH5_INPUT);
}
void READ_RC6()
{
  Read_Input(RC_CH6, RC_CH6_INPUT);
}

//calculates values from pwm signals (pulse start and width)
void Read_Input(uint8_t channel, uint8_t input_pin)
{
  if (digitalRead(input_pin) == HIGH)
  {
    RC_START[channel] = micros();
  }
  else 
  {
    uint16_t rc_compare = (uint16_t) (micros() - RC_START[channel]);
    RC_SHARED[channel] = rc_compare;
  }
}

//pulls values from aray for use in code
void rc_read_values()
{
  noInterrupts();
  memcpy(RC_VALUES, (const void *)RC_SHARED, sizeof(RC_SHARED));
  interrupts();
}

//makes it so minor value changes from default get set back to default (problem default values have to be really exact)
void rc_deadzone_adjust()
{
  //convert range into -100 to 100 to compare against deadzone percent
  for (int i = 0; i < RC_NUM_CHANNELS; i++)
  {
    //no division by zero
    
    float newval = 0;
    if (RC_CHANNEL_MODE[i] == 0)
    {
      //if this is a joystick with a midpoint, our deadzone is aroudn the middle
      newval = map((float)RC_VALUES[i], (float)RC_HIGH[i], (float)RC_LOW[i], 100.0, -100.0);

      if (abs(newval) < RC_DZPERCENT[i])
      {
        //reset to midpoint
        RC_VALUES[i] = RC_MID[i];
      }
    }
    else if (RC_CHANNEL_MODE[i] == 1)
    {
      newval = map((float)RC_VALUES[i], (float)RC_HIGH[i], (float)RC_LOW[i], 100.0, 0.0);

      if (abs(newval) < RC_DZPERCENT[i])
      {
        RC_VALUES[i] = RC_LOW[i];
      }
    }
  }
}