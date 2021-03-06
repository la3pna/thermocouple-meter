/*
 * Thermocouple SCPI Meter by Thomas S. Knutsen LA3PNA. 
 * 
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 */
 
 #include <U8g2lib.h>
 #include <U8x8lib.h>
 #include <avr/dtostrf.h>
 #include <SPI.h>
  #include <scpiparser.h>
 #include "Adafruit_MAX31855.h"

// In this list you define the input voltage resistors in ohm so it measures the input voltage correctly:
float r44 = 2700; // resistor to gnd
float r43 = 30100; // resistor to Vin
 
 
    #define DO   7
    #define CS1   10
    #define CS2   9
    #define CS3   11
    #define CS4   12
    #define CLK  8
    
#if (!defined(__SAMD21G18A__) && !defined(__SAMD21G16A__) && !defined(__SAMD21G16B__) && !defined(__SAMD21J18A__) && !defined(__SAMD21E17A__) && !defined(__SAMD21E18A__))
 #error "This program will only work on SAMD series boards like Empyrean and Zero"
#endif
    
    Adafruit_MAX31855 thermocouple1(CLK, CS1, DO);
    Adafruit_MAX31855 thermocouple2(CLK, CS2, DO);
    Adafruit_MAX31855 thermocouple3(CLK, CS3, DO);
    Adafruit_MAX31855 thermocouple4(CLK, CS4, DO);
    

    
U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C

struct scpi_parser_context ctx;

scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t reset(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_voltage(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_temp_1(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_temp_2(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_temp_3(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_temp_4(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t set_voltage_2(struct scpi_parser_context* context, struct scpi_token* command);

  float temp1;
  float temp2;
  float temp3;
  float temp4;
 
  boolean contTrigger = true;
  String idString = "LA3PNA,Thermocouple temperature meter,1,0.1A";
  String sendresponse = "";
  String errorstring = "";
  boolean use2 = false;
  int selectpin = 2;
  boolean _verbose = false;

void draw(float temp1, float temp2, float temp3, float temp4 ) {
  // graphic commands to redraw the complete screen should be placed here

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    char buf[12] ="";
    if(use2){
     u8g2.drawStr(0,25,dtostrf(temp1, 4, 2, buf));
    u8g2.drawStr(70,25,dtostrf(temp2, 4, 2, buf));
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,8,"1:");
    u8g2.drawStr(70,8,"2:");
    } else{
     u8g2.drawStr(0,14,dtostrf(temp1, 5, 2, buf));
    u8g2.drawStr(70,14,dtostrf(temp2, 5, 2, buf));
    u8g2.drawStr(0,31,dtostrf(temp3, 5, 2, buf));
    u8g2.drawStr(70,31,dtostrf(temp4, 5, 2, buf));
    }
  } while ( u8g2.nextPage() );
}


 
    void setup() {

      pinMode(selectpin,INPUT);
      digitalWrite(selectpin,HIGH); // put on the pullups
      delay(10);
      if(digitalRead(selectpin))
      {
        use2 = true; // select between 2 and 4 sensors
        }
      
      setup_scpi();
       SerialUSB.begin(9600);
       SerialUSB.println("MAX31855 test");
       // wait for MAX chip to stabilize
       Serial1.begin(115200);
       u8g2.begin();
       delay(500);
    }
    
  char line_buffer[256];
  unsigned char read_length;
  int serialport = 0; // 1 = USB, 2 = Serial1, 3 = Serial5;
    
    void loop() {

    // Here we need a routine that selects between the different serial ports, and does some housekeeping so things goes to the correct place
    //
      if (SerialUSB.available() > 0)   // see if incoming serial data:
  { 
    read_length = SerialUSB.readBytesUntil('\n', line_buffer, 256);
    serialport = 1; // 1 for USB
 
  }
      if (Serial1.available() > 0)   // see if incoming serial data:
  { 
    read_length = Serial1.readBytesUntil('\n', line_buffer, 256);
    serialport = 2; // 2 for wlan/gpib

  }

    if(read_length > 0)
    {
       
      scpi_execute_command(&ctx, line_buffer, read_length);
      read_length = NULL;
    }
      
       // Initialize variables.
        // Counter for arrays
       double internalTemp1 = thermocouple1.readInternal(); // Read the internal temperature of the MAX31855.
       double rawTemp1 = thermocouple1.readCelsius(); // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
       
  double  correctedTemp = rawTemp1; // calc_temp(internalTemp1, rawTemp1);
 if(_verbose){  
  SerialUSB.print("Corrected Temp1 = ");
          SerialUSB.print(correctedTemp, 5);
          SerialUSB.print(" internal temp: ");
          SerialUSB.print(internalTemp1, 5);
          SerialUSB.print(" raw temp: ");
          SerialUSB.print(rawTemp1, 5);
          SerialUSB.print(" err: ");
          SerialUSB.println(thermocouple1.readError());
 }    
          
       double    internalTemp2 = thermocouple2.readInternal(); // Read the internal temperature of the MAX31855.
      double  rawTemp2 = thermocouple2.readCelsius(); // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
       
   correctedTemp = rawTemp2;// calc_temp(internalTemp, rawTemp);
 if(_verbose){  
  SerialUSB.print("Corrected Temp2 = ");
          SerialUSB.print(correctedTemp, 5);
          SerialUSB.print(" internal temp: ");
          SerialUSB.print(internalTemp2, 5);
          SerialUSB.print(" raw temp: ");
          SerialUSB.print(rawTemp2, 5);
         SerialUSB.print(" err: ");
          SerialUSB.print(thermocouple2.readError());
          SerialUSB.print("  ");
           SerialUSB.print(" analog A0:");
           
            
              float volt = analogRead(A0)*((47+5700)/47)*(3.3/1024);
              SerialUSB.println(volt);
              SerialUSB.println(contTrigger);

    }

  if(contTrigger){            
if(use2){        
 draw( rawTemp1, rawTemp2, internalTemp1, internalTemp2);
} else{
 draw( rawTemp1, rawTemp2, NAN, NAN);
}
  }
     //  delay(1000);
 
    }
    

void setup_scpi(){
  
  struct scpi_command* trigger;
  struct scpi_command* measure;
  struct scpi_command* systems;
  struct scpi_command* unit;
  
  /* First, initialise the parser. */
  scpi_init(&ctx);

  /*
   * After initialising the parser, we set up the command tree.  Ours is
   *
   *  *IDN?         -> identify
   *  *RST          -> reset to local
   *  :TRIGger
   *    :SINGle    -> trig_single
   *    :CONTinous   -> trig_cont
   *  :MEASure
   *    :TEMPerature?   -> get_alltemp
   *    :TEMPerature1?   -> get_temp1
   *    :TEMPerature2?   -> get_temp2
   *    :TEMPerature3?   -> get_temp3
   *    :TEMPerature4?   -> get_temp4
   *    :INTernal1?      -> get_int1
   *    :INTernal2?      -> get_int2
   *    :INTernal3?      -> get_int3
   *    :INTernal4?      -> get_int4
   *  :SYStem
   *    :VOLTage?   -> get_voltage
   *  :UNIT
   *    :TEMPerature -> set_temperature_unit (C/CEL/F/FAR/K/KEL)
   *    :TEMPerature? -> get_temprature unit (C/CEL/F/FAR/K/KEL)
   */
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*RST", 4, "*RST", 4, reset);

  trigger = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "TRIGger", 7, "TRIG", 4, NULL);
  measure = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "MEASure", 7, "MEAS", 4, NULL);
  systems = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "SYStem", 6, "SYS", 3, NULL);
  unit = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "UNIT", 4, "UNIT", 4, NULL);


  scpi_register_command(trigger, SCPI_CL_CHILD, "SINGle", 6, "SING", 4, trig_single);
  scpi_register_command(trigger, SCPI_CL_CHILD, "CONTinous", 9, "CONT", 4, trig_cont);

  scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE?", 8, "VOLT?", 5, get_voltage); //this works?
  scpi_register_command(measure, SCPI_CL_CHILD, "TEMPerature?", 12, "TEMP?", 5, get_temp);
  scpi_register_command(measure, SCPI_CL_CHILD, "TEMPerature1?", 13, "TEMP1?", 6, get_temp1);
  scpi_register_command(measure, SCPI_CL_CHILD, "TEMPerature2?", 13, "TEMP2?", 6, get_temp2);
  
  scpi_register_command(measure, SCPI_CL_CHILD, "INTernal1?", 10, "INT1?", 5, get_int1);
  scpi_register_command(measure, SCPI_CL_CHILD, "INTernal2?", 10, "INT2?", 5, get_int2);
  
  // NEED A DEFINE FOR 4 MEASUREMENTS, NEED TO IMPLEMENT THE SWITCHING IN SETUP
if(!use2){
  scpi_register_command(measure, SCPI_CL_CHILD, "TEMPerature3?", 13, "TEMP3?", 6, get_temp3);
  scpi_register_command(measure, SCPI_CL_CHILD, "TEMPerature4?", 13, "TEMP4?", 6, get_temp4);
  scpi_register_command(measure, SCPI_CL_CHILD, "INTernal3?", 10, "INT3?", 5, get_int3);
  scpi_register_command(measure, SCPI_CL_CHILD, "INTernal4?", 10, "INT4?", 5, get_int4);
}

  scpi_register_command(systems, SCPI_CL_CHILD, "ERRor?", 6, "ERR?", 4, get_err);
  scpi_register_command(systems, SCPI_CL_CHILD, "VOLTage?", 8, "VOLT?", 5, get_voltage); 
  scpi_register_command(systems, SCPI_CL_CHILD, "PRINT", 5, "PRINT", 5, display_print); 
  }

  // from here down is the different command definitions.

  scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
if(serialport == 1){
  SerialUSB.println(idString);
} else if (serialport == 2){
  Serial1.println(idString);
  }else if (serialport == 3){
    Serial.println(idString);
    }else{
      }
   serialport = 0; // 2 for wlan/gpib
  return SCPI_SUCCESS;
}

scpi_error_t reset(struct scpi_parser_context* context, struct scpi_token* command)
{
if(serialport == 1){
  sendresponse = "Resetting ";
  
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
   NVIC_SystemReset();
  return SCPI_SUCCESS;
}

 scpi_error_t trig_single(struct scpi_parser_context* context, struct scpi_token* command)
{

  // needs to do a single trigger here
  scpi_free_tokens(command);
  _verbose = false;
  contTrigger = false;
  return SCPI_SUCCESS;
}
 scpi_error_t trig_cont(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);

  contTrigger = true;
  _verbose = true;
  return SCPI_SUCCESS;
}

scpi_error_t get_voltage(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
  SerialUSB.println("test to see if its a logic fault or this does not run");
  
  float volt = analogRead(A0)*((r44)/(r44+r43))*(3.3/1024);
 float divisor = ((r44)/(r44+r43));
  sendresponse = "Power supply voltage: " + String(volt) + " V";          
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
              
  return SCPI_SUCCESS;
}

scpi_error_t get_temp(struct scpi_parser_context* context, struct scpi_token* command)
{

if(use2)
{
  sendresponse = String(thermocouple1.readCelsius())+ "; " + String(thermocouple2.readCelsius())+";"; 

  } else 
  { sendresponse = String(thermocouple1.readCelsius())+"; "+String(thermocouple2.readCelsius())+"; "+String(thermocouple3.readCelsius())+"; "+String(thermocouple4.readCelsius())+";"; 

    }
  
  
        
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t get_temp1(struct scpi_parser_context* context, struct scpi_token* command)
{
   // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
    sendresponse = String(thermocouple1.readCelsius());            
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t get_temp2(struct scpi_parser_context* context, struct scpi_token* command)
{
    sendresponse = String(thermocouple2.readCelsius());            
 
          
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t get_temp3(struct scpi_parser_context* context, struct scpi_token* command)
{
 if(use2){sendresponse = "No 4 sensor board attached!";}
 else{
    sendresponse = String(thermocouple3.readCelsius());  
 }          
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t get_temp4(struct scpi_parser_context* context, struct scpi_token* command)
{
 if(use2){sendresponse = "No 4 sensor board attached!";}
 else{
    sendresponse = String(thermocouple4.readCelsius());         
 }   
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t get_int1(struct scpi_parser_context* context, struct scpi_token* command)
{
   // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
    sendresponse = String(thermocouple1.readInternal());            
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t get_int2(struct scpi_parser_context* context, struct scpi_token* command)
{
   // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
    sendresponse = String(thermocouple2.readInternal());            
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t get_int3(struct scpi_parser_context* context, struct scpi_token* command)
{
   // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
 if(use2){sendresponse = "No 4 sensor board attached!";}
 
 else{
    sendresponse = String(thermocouple3.readInternal());  
 }
         
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t get_int4(struct scpi_parser_context* context, struct scpi_token* command)
{
   // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
    if(use2){sendresponse = "No 4 sensor board attached!";}
 else{
    sendresponse = String(thermocouple4.readInternal());            
 }
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      errorstring = errorstring + " Error: Unknown port reply";
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
   
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}


scpi_error_t get_err(struct scpi_parser_context* context, struct scpi_token* command)
{
      sendresponse = String(errorstring);
      errorstring = "";            
if(serialport == 1){
  SerialUSB.println(sendresponse);
} else if (serialport == 2){
  Serial1.println(sendresponse);
  }else if (serialport == 3){
    Serial.println(sendresponse);
    }else{
      }
   serialport = 0; // 2 for wlan/gpib
   sendresponse = "";
  
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t display_print(struct scpi_parser_context* context, struct scpi_token* command)
{
  contTrigger = false;
  struct scpi_token* args;
  args = command;
 char* val = args->value;
 int l = strlen(val);
 int start = 0;
char mychar[20];

for(int m=0;m<20;m++){
  mychar[m]=NULL;
  }

 for (int i = 0; i < l; i++){
  if(val[i] == ' ')
    { start = i;
    break;
      }
  }
   if((start > 0)&& (l > start)){
    for(int j = start+1; j < l; j++){
     mychar[j-start-1]=val[j];
      }
   }
u8g2.firstPage();
  do {
    
    u8g2.setFont(u8g2_font_ncenB10_tr);
   u8g2.drawStr(10,18,mychar);
    
  } while ( u8g2.nextPage() );
  
scpi_free_tokens(command);
  return SCPI_SUCCESS;
}




