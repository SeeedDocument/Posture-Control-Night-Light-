
#include "Wire.h"
#include "adxl372.h"
#include "Adafruit_NeoPixel.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SERIALUSB
  #define SYS_VOL   3.3
#else
  #define SERIAL Serial
  #define SYS_VOL   5
#endif

unsigned int  Delay_Ms;
unsigned int  Delay_Init;
unsigned char Led_State;
#define LED_PIN    2
#define LED_OFF    0
#define LED_ON     1
#define LED_DELAY_5S_OFF 2
#define LED_DELAY_10S_OFF 3
#define LED_DELAY_15S_OFF 4
#define LED_DELAY_20S_OFF 5
#define DELAY_5S 5*1000
#define DELAY_10S 10*1000
#define DELAY_15S 15*1000
#define DELAY_20S 20*1000
#define LED_COUNT 20

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
uint32_t Off_All = strip.Color(0, 0, 0);
uint32_t Red = strip.Color(255, 0, 0);
uint32_t Yellow = strip.Color(0, 255, 0);
uint32_t Bule = strip.Color(0, 0, 255);
uint32_t Colour_Box[3] = {Red,Yellow,Bule};
uint32_t Led_Colour;
float cali_data[3];
unsigned char i;


float cali_buf[3];

#define BUFFER_SIZE   (100*6)
uint8_t buffer[BUFFER_SIZE] = {0,};
ADXL372 acc;
xyz_t xyz,xyz_acc;

void Led_Control(uint32_t Led_num,uint32_t Color,bool Switch)
{
    
    for(int i=0; i<Led_num; i++) { 
      if(Switch)
      {
        strip.setPixelColor(i, Color);
        strip.show();
      }  
      else
      {
        strip.setPixelColor(i, Off_All);
        strip.show();    
    }
  }
   
}

void calibration(void)
{

  SERIAL.println("Please Place the module horizontally!");
  delay(1000);
  SERIAL.println("Start calibration........");
    uint16_t j;
    uint16_t samples = acc.samples_in_fifo();
    // To ensure that data is not overwritten and stored out of order, 
    // at least one sample set must be left in the FIFO after every read        
    if (samples > 12) {
      samples = (samples > BUFFER_SIZE)? BUFFER_SIZE : (samples / 6 - 1) * 6;
    
    acc.fifo_read(buffer, samples);
    for (j=0; j<samples; j+=6) {
      // convert raw data
      xyz_t *xyz = acc.format(buffer + j);
      Serial.print(samples);
      Serial.print('\t');
      Serial.print(xyz->x);
      xyz_acc.x = xyz->x + xyz_acc.x;
      Serial.print('\t');
      Serial.print(xyz->y);
      xyz_acc.y = xyz->y + xyz_acc.y;      
      Serial.print('\t');
      Serial.println(xyz->z);
      xyz_acc.z = xyz->z + xyz_acc.z;
    }
     xyz_acc.x = xyz_acc.x * 6.0 / j;
     Serial.println(xyz_acc.x);
     xyz_acc.y = xyz_acc.y * 6.0 / j;
     Serial.println(xyz_acc.y);
     xyz_acc.z = xyz_acc.z * 6.0 / j;
     Serial.println(xyz_acc.z);    
    cali_buf[0] = xyz_acc.x;
    cali_buf[1] = xyz_acc.y;
    cali_buf[2] = xyz_acc.z-10;
    SERIAL.println("Calibration OK!!");
    }
}

void setup() {

  strip.begin();
  strip.setPixelColor(10, strip.Color(0, 150, 0));
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(200);
  
  acc.begin();
  SERIAL.begin(115200);
  Serial.println(acc.id(), HEX);
  acc.timing_ctrl(RATE_400);
  acc.measurement_ctrl(BW_200, true);
  acc.fifo_ctrl(STREAMED, FIFO_XYZ);
  acc.power_ctrl(MEASUREMENT_MODE);
  acc.setActiveTime(10);
  
  calibration();
  Led_Colour = Colour_Box[0];
}

void loop() {
  uint16_t j;
  uint16_t samples = acc.samples_in_fifo();
  // To ensure that data is not overwritten and stored out of order, 
  // at least one sample set must be left in the FIFO after every read        
  if (samples > 12) {
    samples = (samples > BUFFER_SIZE)? BUFFER_SIZE : (samples / 6 - 1) * 6;

    acc.fifo_read(buffer, samples);
    for (j=0; j<samples; j+=6) {
      // convert raw data
      xyz_t *xyz = acc.format(buffer + j);
      Serial.print(samples);
      Serial.print('\t');
      Serial.print(xyz->x);
      xyz_acc.x = xyz->x + xyz_acc.x;
      Serial.print('\t');
      Serial.print(xyz->y);
      xyz_acc.y = xyz->y + xyz_acc.y;      
      Serial.print('\t');
      Serial.println(xyz->z);
      xyz_acc.z = xyz->z + xyz_acc.z;
    }
     xyz_acc.x = xyz_acc.x * 6.0 / j;

     xyz_acc.y = xyz_acc.y * 6.0 / j;

     xyz_acc.z = xyz_acc.z * 6.0 / j;
     
     xyz_acc.x = cali_buf[0] - xyz_acc.x;
     xyz_acc.y = cali_buf[1] - xyz_acc.y;
     xyz_acc.z = cali_buf[2] - xyz_acc.z;
     
     Serial.println(xyz_acc.x);
     Serial.println(xyz_acc.y);
     Serial.println(xyz_acc.z);
     
  }  
  if(xyz_acc.z   + xyz_acc.y   + xyz_acc.x < 50)
  {
      if((abs(xyz_acc.z) > abs(xyz_acc.y))&&(abs(xyz_acc.z) > abs(xyz_acc.x)))
      {
        
         if(xyz_acc.z > 0)
         {
            SERIAL.println(" -Z ");
            Led_State = LED_OFF;
         }
         else
         {
           SERIAL.println(" Z ");
           Led_State = LED_ON;
           if(xyz_acc.z <= -10)
           {
             if(abs(xyz_acc.y) > 2 ||abs(xyz_acc.x) > 2)
             {
                i ++ ;
                if(i > 2) i = 0;
                Led_Colour = Colour_Box[i];               
             }
           }
         }
         Delay_Ms = 0;
         
      }
      else if((abs(xyz_acc.x) > abs(xyz_acc.y))&&(abs(xyz_acc.x) > abs(xyz_acc.z)))
      {
        if(xyz_acc.x > 0)
        {
          SERIAL.println(" x ");
          if(Led_State != LED_DELAY_5S_OFF) Delay_Ms = 0;
          Led_State = LED_DELAY_5S_OFF;
        }
        else 
        {
          SERIAL.println(" -x ");
          if(Led_State != LED_DELAY_15S_OFF) Delay_Ms = 0;
          Led_State = LED_DELAY_15S_OFF;   
        }
        
      }
      else if((abs(xyz_acc.y) > abs(xyz_acc.z))&&(abs(xyz_acc.y) > abs(xyz_acc.x)))
      {
          if(xyz_acc.y >0)
          {
            SERIAL.println(" y ");
            if(Led_State != LED_DELAY_10S_OFF) Delay_Ms = 0;
            Led_State = LED_DELAY_10S_OFF;
          }
          else
          {
            SERIAL.println(" -y ");
            if(Led_State != LED_DELAY_20S_OFF) Delay_Ms = 0;
            Led_State = LED_DELAY_20S_OFF;   
          }
      }
  }
  switch(Led_State)
  {
    case LED_ON:
    
      Led_Control(LED_COUNT,Led_Colour,1);
      
    break;
    case LED_OFF:
    
      Led_Control(LED_COUNT,Led_Colour,0);
      
    break;
    case LED_DELAY_5S_OFF:
      
      if(Delay_Ms == 0)
      {
        
        Led_Control(LED_COUNT,Led_Colour,1);
        Delay_Init = millis();
        Delay_Ms = 1;
        
      }
      else if (Delay_Ms < DELAY_5S)
      {
        
        Delay_Ms = millis() - Delay_Init;
        SERIAL.println(" x State");
        
      }
      else
      {
         Led_Control(LED_COUNT,Led_Colour,0);
      }
      
    break;
    case LED_DELAY_10S_OFF:
    
      if(Delay_Ms == 0)
      {
        
        Led_Control(LED_COUNT,Led_Colour,1);
        Delay_Init = millis();
        Delay_Ms = 1;
        
      }
      else if (Delay_Ms < DELAY_10S)
      {
        
        Delay_Ms = millis() - Delay_Init;
        SERIAL.println(" y State");
        
      }
      else
      {
          Led_Control(LED_COUNT,Led_Colour,0);
      }
      
    break;   
    case LED_DELAY_15S_OFF:
    
      if(Delay_Ms == 0)
      {
        
        Led_Control(LED_COUNT,Led_Colour,1);
        Delay_Init = millis();
        Delay_Ms = 1;
        
      }
      else if (Delay_Ms < DELAY_15S)
      {
        
        Delay_Ms = millis() - Delay_Init;
        SERIAL.println(" y State");
        
      }
      else
      {
          Led_Control(LED_COUNT,Led_Colour,0);
      }
      
    break;  
    case LED_DELAY_20S_OFF:
    
      if(Delay_Ms == 0)
      {
        
        Led_Control(LED_COUNT,Led_Colour,1);
        Delay_Init = millis();
        Delay_Ms = 1;
        
      }
      else if (Delay_Ms < DELAY_20S)
      {
        
        Delay_Ms = millis() - Delay_Init;
        SERIAL.println(" y State");
        
      }
      else
      {
          Led_Control(LED_COUNT,Led_Colour,0);
      }
      
    break; 
  }
  
  delay(100);
  
}
