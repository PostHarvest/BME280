/////////////////////////////////////////////////////////////////////////
////                            EX_CAN.C                             ////
////                                                                 ////
//// Example of CCS's CAN library, using the PIC18Fxx8.  This        ////
//// example was tested using MCP250xxx CAN Developer's Kit.         ////
////                                                                 ////
//// Connect pin B2 (CANTX) to the CANTX pin on the open NODE A of   ////
//// the developer's kit, and connect pin B3 (CANRX) to the CANRX    ////
//// pin on the open NODE A.                                         ////
////                                                                 ////
//// NODE B has an MCP250xxx which sends and responds certan canned  ////
//// messages.  For example, hitting one of the GPX buttons on       ////
//// the development kit causes the MCP250xxx to send a 2 byte       ////
//// message with an ID of 0x290.  After pressing one of those       ////
//// buttons with this firmware you should see this message          ////
//// displayed over RS232.                                           ////
////                                                                 ////
//// NODE B also responds to certain CAN messages.  If you send      ////
//// a request (RTR bit set) with an ID of 0x18 then NODE B will     ////
//// respond with an 8-byte message containing certain readings.     ////
//// This firmware sends this request every 2 seconds, which NODE B  ////
//// responds.                                                       ////
////                                                                 ////
//// If you install Microchip's CANKing software and use the         ////
//// MCP250xxx , you can see all the CAN traffic and validate all    ////
//// experiments.                                                    ////
////                                                                 ////
//// For more documentation on the CCS CAN library, see can-18xxx8.c ////
////                                                                 ////
////  Jumpers:                                                       ////
////     PCM,PCH    pin C7 to RS232 RX, pin C6 to RS232 TX           ////
////                                                                 ////
////  This example will work with the PCM and PCH compilers.         ////
/////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2003 Custom Computer Services         ////
//// This source code may only be used by licensed users of the CCS  ////
//// C compiler.  This source code may only be distributed to other  ////
//// licensed users of the CCS C compiler.  No other use,            ////
//// reproduction or distribution is permitted without written       ////
//// permission.  Derivative programs created using this software    ////
//// in object code form are not restricted in any way.              ////
/////////////////////////////////////////////////////////////////////////

#include <18F248.h>
#fuses HS,NOPROTECT,NOLVP,NOWDT
#use delay(clock=20000000)
#use rs232(baud=38400, xmit=PIN_C6, rcv=PIN_C7)
#include <can-18xxx8.c>
#include <BME280.c>
//#include <sht71.c>
//#include <MAX31855.c>
#include<stdlib.h>
//#include <LCDdriver.c>
int16 ms;

#int_timer2
void isr_timer2(void) {
   ms++; //keep a running timer that increments every milli-second
}

void main() {
   //setup_spi(SPI_MASTER | SPI_MODE_1 | SPI_CLK_DIV_64, ); 
   struct rx_stat rxstat;
   int32 rx_id;
   int in_data[8];
   int rx_len;

//send a request (tx_rtr=1) for 8 bytes of data (tx_len=8) from id 24 (tx_id=24)
   int out_data[8];
   int out_data_hu[8];
   int32 tx_id=23; // id = 0x23 for temperature and pressure
   int32 tx_idh =25; // id= 0x25 for humidity
  
   int1 tx_rtr=0;// CU LA 1
   int1 tx_ext=1;// CU LA 0
   int tx_len=8;
   int tx_pri=3;

   int i;
   int j = 0;
  
   for (i=0;i<8;i++) {
      out_data[i]=0x22;
      in_data[i]=0;
      out_data_hu[i]=0;
     
   }

 ////////////////////
union conv { 
    float f; 
    int8 b[4]; 
  };
  union conv tempp, pressp, hup;  // p is alias pointer

out_data[0] = tempp.b[0];
out_data[1] = tempp.b[1];
out_data[2] = tempp.b[2];
out_data[3] = tempp.b[3];

out_data[4] = pressp.b[0];
out_data[5] = pressp.b[1];
out_data[6] = pressp.b[2];
out_data[7] = pressp.b[3];

out_data_hu[0] = hup.b[0];
out_data_hu[1] = hup.b[1];
out_data_hu[2] = hup.b[2];
out_data_hu[3] = hup.b[3];


//printf(" %4.4f : %x : %x : %x : %x \r\n",val2.f,\ 
//    val2.b[0],val2.b[1],val2.b[2],val2.b[3]); 
//    //The new float value, and the bytes that make it. 

 //////////////////// 
 float temp,press,hu, thermo,thermo1;
   printf("\r\n\r\nCCS CAN TRANSFER BME280 DATA\r\n");

   setup_timer_2(T2_DIV_BY_4,79,16);   //setup up timer2 to interrupt every 1ms if using 20Mhz clock

   can_init();
   init_BME280();
   BME280Begin();
   delay_ms(200);
  
   
  // can_set_mode(CAN_OP_LOOPBACK);
  
   enable_interrupts(INT_TIMER2);   //enable timer2 interrupt
   enable_interrupts(GLOBAL);       //enable all interrupts (else timer2 wont happen)

   printf("\r\nRunning...");
   
   
   while(TRUE)
   {   
        BurstRead();
        delay_ms(200);
        //out_data[0] = BME280ReadByte(0xD0);
        //delay_ms(50);
        //printf("\r\ID=%X", out_data[0]);
        //delay_ms(10);
        temp = BME280ReadTemperature();
        tempp.f = temp;
        out_data[0] = tempp.b[0];
        out_data[1] = tempp.b[1];
        out_data[2] = tempp.b[2];
        out_data[3] = tempp.b[3];
        delay_ms(10);
        printf("\r\nTemp_BME280 = %f3",temp);
        hu = BME280ReadHumidity();
        delay_ms(20);
        hup.f = hu;
         out_data_hu[0] = hup.b[0];
         out_data_hu[1] = hup.b[1];
         out_data_hu[2] = hup.b[2];
         out_data_hu[3] = hup.b[3];
        printf("\r\nHumidityBME280 = %f3",hu);
        press = Calculate_Pess();
        pressp.f = press;
         out_data[4] = pressp.b[0];
         out_data[5] = pressp.b[1];
         out_data[6] = pressp.b[2];
         out_data[7] = pressp.b[3];
        delay_ms(20);
        printf("\r\nTPressBME280 = %f3",press);
       

          delay_ms(10);
       // printf("\r\nThermocouple 1 = %f", thermo1);
        BME280WriteByte(0xF4,0x6E);
      if ( can_kbhit() )   //if data is waiting in buffer...
      {
         if(can_getd(rx_id, &in_data[0], rx_len, rxstat)) { //...then get data from buffer
            printf("\r\nGOT: BUFF=%U ID=%LU LEN=%U OVF=%U ", rxstat.buffer, rx_id, rx_len, rxstat.err_ovfl);
            printf("FILT=%U RTR=%U EXT=%U INV=%U", rxstat.filthit, rxstat.rtr, rxstat.ext, rxstat.inv);
            printf("\r\n    DATA = ");
            for (i=0;i<rx_len;i++) {
               printf("%X ",in_data[i]);
            }
            printf("\r\n");
         }
         else {
            printf("\r\nFAIL on GETD\r\n");
         }

      }
      //every two seconds, send new data if transmit buffer is empty
      if ( can_tbe() && (ms > 2000))
      {
         ms=0;
         i=can_putd(tx_id, out_data, tx_len,tx_pri,tx_ext,tx_rtr); //put data on transmit buffer
         if (i != 0xFF) { //success, a transmit buffer was open
            printf("\r\nPUT %U: ID=%LU LEN=%U ", i, tx_id, tx_len);  //i return 1 if transmit success
            printf("PRI=%U EXT=%U RTR=%U\r\n   DATA = ", tx_pri, tx_ext, tx_rtr);
            //  printf("\r\nID=%X", out_data[0]);
            for (i=0;i<tx_len;i++) {
               printf("\r\n%X ",out_data[i]);
            }
            printf("\r\n");
         }
         else { //fail, no transmit buffer was open
            printf("\r\nFAIL on PUTD\r\n");
        
         }
         j=can_putd(tx_idh, out_data_hu, tx_len,tx_pri,tx_ext,tx_rtr); //put data on transmit buffer
         if(j != 0xFF)
         {
            printf("\r\nPUT %U: ID=%LU LEN=%U ", i, tx_id, tx_len);  //i return 1 if transmit success
            printf("PRI=%U EXT=%U RTR=%U\r\n   DATA = ", tx_pri, tx_ext, tx_rtr);
             for (j=0;j<tx_len;i++) {
               printf("\r\n%X ",out_data_hu[i]);
            }
            printf("\r\n");
         }
         
   
      
     delay_ms(500); 
   }
}
}
