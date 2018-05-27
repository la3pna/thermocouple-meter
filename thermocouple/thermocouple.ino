 #include <U8g2lib.h>
 #include <U8x8lib.h>
 #include <avr/dtostrf.h>
 #include <SPI.h>
  #include <scpiparser.h>
 #include "Adafruit_MAX31855.h"

  //
  //  #define use4 // Undefine for only 2 thermocouple amplifiers
  //  Should make a method using D2 to switch between 2 and 4 probe versions. 
  //
 
    #define DO   7
    #define CS1   9
    #define CS2   10
    #ifdef use4
    #define CS3   11
    #define CS4   12
    #endif
    #define CLK  8
    
#if (!defined(__SAMD21G18A__) && !defined(__SAMD21G16A__) && !defined(__SAMD21G16B__) && !defined(__SAMD21J18A__) && !defined(__SAMD21E17A__) && !defined(__SAMD21E18A__))
 #error "This program will only work on SAMD series boards like Empyrean and Zero"
#endif
    
    Adafruit_MAX31855 thermocouple1(CLK, CS1, DO);
    Adafruit_MAX31855 thermocouple2(CLK, CS2, DO);

    
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

  double temp1;
  double temp2;
  #ifdef use4
  double temp3;
  double temp4;
  #endif
  boolean contTrigger = true;


void draw(float temp1, float temp2, float temp3, float temp4 ) {
  // graphic commands to redraw the complete screen should be placed here

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    // u8g2.setFont(u8g2_font_courB18);
//   u8g2.setFont(u8g2_font_osb21);
    char buf[12] ="";
    #ifdef use4
    u8g2.drawStr(0,14,dtostrf(temp1, 5, 2, buf));
    u8g2.drawStr(70,14,dtostrf(temp2, 5, 2, buf));
    u8g2.drawStr(0,31,dtostrf(temp3, 5, 2, buf));
    u8g2.drawStr(70,31,dtostrf(temp4, 5, 2, buf));
    #else
    
    u8g2.drawStr(0,25,dtostrf(temp1, 4, 2, buf));
    u8g2.drawStr(70,25,dtostrf(temp2, 4, 2, buf));
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,8,"1:");
    u8g2.drawStr(70,8,"2:");
    #endif
  } while ( u8g2.nextPage() );
}


 
    void setup() {
      setup_scpi();
       SerialUSB.begin(9600);
       SerialUSB.println("MAX31855 test");
       // wait for MAX chip to stabilize
       u8g2.begin();
       delay(500);
    }
    
  char line_buffer[256];
  unsigned char read_length;
    
    void loop() {

    // Here we need a routine that selects between the different serial ports, and does some housekeeping so things goes to the correct place
    //
      

      read_length = SerialUSB.readBytesUntil('\n', line_buffer, 256);
    if(read_length > 0)
    {
      scpi_execute_command(&ctx, line_buffer, read_length);
    }
      
       // Initialize variables.
        // Counter for arrays
       double internalTemp1 = thermocouple1.readInternal(); // Read the internal temperature of the MAX31855.
       double rawTemp1 = thermocouple1.readCelsius(); // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
       
  double  correctedTemp = rawTemp1; // calc_temp(internalTemp1, rawTemp1);
 if(contTrigger){  
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
       
   correctedTemp = internalTemp2;// calc_temp(internalTemp, rawTemp);
 if(contTrigger){  
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
 #ifdef use4         
 draw( rawTemp1, rawTemp2, internalTemp1, internalTemp2);
 #else
 draw( rawTemp1, rawTemp2, NAN, NAN);
 //draw( 1200.90f, -1200.023f, NAN, NAN); // used to test display formatting
 #endif
  }
       delay(1000);
 
    }





    

    double calc_temp(double internalTemp,double rawTemp ){

        double thermocoupleVoltage= 0;
       double internalVoltage = 0;
       double correctedTemp = 0;
       double totalVoltage = 0;
       int i = 0;
      
        // Check to make sure thermocouple is working correctly.
       if (isnan(rawTemp)) {
        SerialUSB.println("Something wrong with thermocouple!");
      }
       else {
          // Steps 1 & 2. Subtract cold junction temperature from the raw thermocouple temperature.
          thermocoupleVoltage = (rawTemp - internalTemp)*0.041276;  // C * mv/C = mV
 
          // Step 3. Calculate the cold junction equivalent thermocouple voltage.
 
          if (internalTemp >= 0) { // For positive temperatures use appropriate NIST coefficients
             // Coefficients and equations available from http://srdata.nist.gov/its90/download/type_k.tab
 
             double c[] = {-0.176004136860E-01,  0.389212049750E-01,  0.185587700320E-04, -0.994575928740E-07,  0.318409457190E-09, -0.560728448890E-12,  0.560750590590E-15, -0.320207200030E-18,  0.971511471520E-22, -0.121047212750E-25};
 
             // Count the the number of coefficients. There are 10 coefficients for positive temperatures (plus three exponential coefficients),
             // but there are 11 coefficients for negative temperatures.
             int cLength = sizeof(c) / sizeof(c[0]);
 
             // Exponential coefficients. Only used for positive temperatures.
             double a0 =  0.118597600000E+00;
             double a1 = -0.118343200000E-03;
             double a2 =  0.126968600000E+03;
 
 
             // From NIST: E = sum(i=0 to n) c_i t^i + a0 exp(a1 (t - a2)^2), where E is the thermocouple voltage in mV and t is the temperature in degrees C.
             // In this case, E is the cold junction equivalent thermocouple voltage.
             // Alternative form: C0 + C1*internalTemp + C2*internalTemp^2 + C3*internalTemp^3 + ... + C10*internaltemp^10 + A0*e^(A1*(internalTemp - A2)^2)
             // This loop sums up the c_i t^i components.
             for (i = 0; i < cLength; i++) {
                internalVoltage += c[i] * pow(internalTemp, i);
             }
                // This section adds the a0 exp(a1 (t - a2)^2) components.
                internalVoltage += a0 * exp(a1 * pow((internalTemp - a2), 2));
          }
          else if (internalTemp < 0) { // for negative temperatures
             double c[] = {0.000000000000E+00,  0.394501280250E-01,  0.236223735980E-04, -0.328589067840E-06, -0.499048287770E-08, -0.675090591730E-10, -0.574103274280E-12, -0.310888728940E-14, -0.104516093650E-16, -0.198892668780E-19, -0.163226974860E-22};
             // Count the number of coefficients.
             int cLength = sizeof(c) / sizeof(c[0]);
 
             // Below 0 degrees Celsius, the NIST formula is simpler and has no exponential components: E = sum(i=0 to n) c_i t^i
             for (i = 0; i < cLength; i++) {
                internalVoltage += c[i] * pow(internalTemp, i) ;
             }
          }
 
          // Step 4. Add the cold junction equivalent thermocouple voltage calculated in step 3 to the thermocouple voltage calculated in step 2.
          totalVoltage = thermocoupleVoltage + internalVoltage;
 
          // Step 5. Use the result of step 4 and the NIST voltage-to-temperature (inverse) coefficients to calculate the cold junction compensated, linearized temperature value.
          // The equation is in the form correctedTemp = d_0 + d_1*E + d_2*E^2 + ... + d_n*E^n, where E is the totalVoltage in mV and correctedTemp is in degrees C.
          // NIST uses different coefficients for different temperature subranges: (-200 to 0C), (0 to 500C) and (500 to 1372C).
          if (totalVoltage < 0) { // Temperature is between -200 and 0C.
             double d[] = {0.0000000E+00, 2.5173462E+01, -1.1662878E+00, -1.0833638E+00, -8.9773540E-01, -3.7342377E-01, -8.6632643E-02, -1.0450598E-02, -5.1920577E-04, 0.0000000E+00};
 
             int dLength = sizeof(d) / sizeof(d[0]);
             for (i = 0; i < dLength; i++) {
                correctedTemp += d[i] * pow(totalVoltage, i);
             }
          }
          else if (totalVoltage < 20.644) { // Temperature is between 0C and 500C.
             double d[] = {0.000000E+00, 2.508355E+01, 7.860106E-02, -2.503131E-01, 8.315270E-02, -1.228034E-02, 9.804036E-04, -4.413030E-05, 1.057734E-06, -1.052755E-08};
             int dLength = sizeof(d) / sizeof(d[0]);
             for (i = 0; i < dLength; i++) {
                correctedTemp += d[i] * pow(totalVoltage, i);
             }
          }
          else if (totalVoltage < 54.886 ) { // Temperature is between 500C and 1372C.
             double d[] = {-1.318058E+02, 4.830222E+01, -1.646031E+00, 5.464731E-02, -9.650715E-04, 8.802193E-06, -3.110810E-08, 0.000000E+00, 0.000000E+00, 0.000000E+00};
             int dLength = sizeof(d) / sizeof(d[0]);
             for (i = 0; i < dLength; i++) {
                correctedTemp += d[i] * pow(totalVoltage, i);
             }
          } else { // NIST only has data for K-type thermocouples from -200C to +1372C. If the temperature is not in that range, set temp to impossible value.
             // Error handling should be improved.
             SerialUSB.print("Temperature is out of range. This should never happen.");
             correctedTemp = NAN;
          }
 
         
       }
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
   *    :VOLTage    -> set_voltage
   *    :VOLTage1   -> set_voltage_2
   *  :MEASure
   *    :VOLTage?   -> get_voltage
   *    :VOLTage1?  -> get_voltage_2
   *    :VOLTage2?  -> get_voltage_3
   *  :SYSTem
   *    :VOLTage?   -> get_voltage
   *  :UNIT
   *    :TEMPerature -> set_temperature_unit (C/CEL/F/FAR/K/KEL)
   *    :TEMPerature? -> get_temprature unit (C/CEL/F/FAR/K/KEL)
   */
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*RST", 4, "*RST", 4, reset);

  trigger = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "TRIGger", 7, "TRIG", 4, NULL);
  measure = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "MEASure", 7, "MEAS", 4, NULL);
  systems = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "SYSTem", 6, "SYST", 4, NULL);
  unit = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "UNIT", 4, "UNIT", 4, NULL);


  scpi_register_command(trigger, SCPI_CL_CHILD, "SINGle", 6, "SING", 4, trig_single);
  scpi_register_command(trigger, SCPI_CL_CHILD, "CONTinous", 9, "CONT", 4, trig_cont);

  scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE?", 8, "VOLT?", 5, get_voltage);
//  scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE1?", 9, "VOLT1?", 6, get_voltage_2);
//  scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE2?", 9, "VOLT2?", 6, get_voltage_3);
   
//  scpi_register_command(unit, SCPI_CL_CHILD, "TEMPerature", 11, "TEMP", 4, get_voltage_3);
//  scpi_register_command(unit, SCPI_CL_CHILD, "TEMPerature?", 12, "TEMP?", 5, get_voltage_3);

  scpi_register_command(systems, SCPI_CL_CHILD, "ERRor?", 6, "ERR?", 4, get_err);
  scpi_register_command(systems, SCPI_CL_CHILD, "VOLTage?", 8, "VOLT?", 4, get_voltage);
  scpi_register_command(systems, SCPI_CL_CHILD, "PRINT", 5, "PRINT", 5, display_print);


  }

  scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);

  SerialUSB.println("OIC,Embedded SCPI Example,1,10");
  return SCPI_SUCCESS;
}

scpi_error_t reset(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
  SerialUSB.println("resetting");
   NVIC_SystemReset();
  //Serial.println("OIC,Embedded SCPI Example,1,10");
  return SCPI_SUCCESS;
}

 scpi_error_t trig_single(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
  contTrigger = false;
  return SCPI_SUCCESS;
}
 scpi_error_t trig_cont(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);

  contTrigger = true;
  return SCPI_SUCCESS;
}

scpi_error_t get_voltage(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
  SerialUSB.print(" analog A0:");
           
            
              float volt = analogRead(A0)*((47+5700)/47)*(3.3/1024);
              SerialUSB.println(volt);
  //Serial.println("OIC,Embedded SCPI Example,1,10");
  return SCPI_SUCCESS;
}

scpi_error_t set_voltage(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
 setup();
  //Serial.println("OIC,Embedded SCPI Example,1,10");
  return SCPI_SUCCESS;
}

scpi_error_t get_err(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
 setup();
  //Serial.println("OIC,Embedded SCPI Example,1,10");
  return SCPI_SUCCESS;
}

scpi_error_t display_print(struct scpi_parser_context* context, struct scpi_token* command)
{
  contTrigger = false;
SerialUSB.println("printing");
u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
   u8g2.drawStr(10,18,"test");
    
  } while ( u8g2.nextPage() );
scpi_free_tokens(command);
  return SCPI_SUCCESS;
}




