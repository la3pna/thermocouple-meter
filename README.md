View this project on [CADLAB.io](https://cadlab.io/project/1066). 

# thermocouple-meter
a open source thermocouple meter

This project runs on the Empyrean series ARM microcontroller boards. It may run on Arduino Zero and other SAMD series, but it will not run on ATMEGA series boards!


Todo:
PCB:
* do a PCB for the 2 sensors 3 and 4
* ESP-01 wlan alternative? (as of now there is a extra serial on the board.)
* GPIB? (using the extra serial port?)


SCPI Commands:
   *  *IDN?                  -> identify
   *  *RST                   -> reset to local
   *  :TRIGger
       *    :SINGle          -> trigger single
       *    :CONTinous       -> trigger continous
   *  :MEASure
       *    :TEMPerature?    -> get all temp
       *    :TEMPerature1?   -> get temp1
       *    :TEMPerature2?   -> get temp2
       *    :TEMPerature3?   -> get temp3
       *    :TEMPerature4?   -> get temp4
       *    :INTernal1?      -> get int1 (compensation temperature)
       *    :INTernal2?      -> get int2
       *    :INTernal3?      -> get int3
       *    :INTernal4?      -> get int4
   *  :SYStem
       *    :VOLTage?        -> get supply voltage
       *    :ERRor?          -> get/clears error from error handling
       *    :PRINT           -> display print string
   *  :UNIT (getting these to work will require rewriting the adafruit library from scratch)
       *    :TEMPerature     -> set_temperature_unit (C/CEL/F/FAR/K/KEL) NOT IMPLEMENTED YET 
       *    :TEMPerature?    -> get_temprature unit (C/CEL/F/FAR/K/KEL) NOT IMPLEMENTED YET
       
 Libraries needed: 
   - U8g2lib (Available in arduino library manager)
   - Open Instrument control [scpiparser] ( https://github.com/LachlanGunn/oic )
   - Adafruit MAX31855 (available in arduino library manager) (intend to rewrite this to use some SAMD features in getting the data) 
