View this project on [CADLAB.io](https://cadlab.io/project/1066). 

# thermocouple-meter
a open source termocouple meter


Todo:
PCB:
* do a PCB for the 2 sensors 3 and 4
* ESP-01 wlan alternative?
* GPIB?

FW:
* implement a way of selecting between the serial ports, and reply to the correct port
	- use a boolean and accept that wlan and GPIB can't run at the same time?
* automatic switching between the 2 sensor and 4 sensor mode


SCPI Commands:
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
       *    :ERRor?     -> get_err
       *    :PRINT      -> display_print
   *  :UNIT
       *    :TEMPerature -> set_temperature_unit (C/CEL/F/FAR/K/KEL) NOT IMPLEMENTED YET
       *    :TEMPerature? -> get_temprature unit (C/CEL/F/FAR/K/KEL) NOT IMPLEMENTED YET
