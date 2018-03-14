//************************************************ 
//  BME280 Barometric Pressure Sensor 
// 
//  - Datasheet:  https://cdn-shop.adafruit.com/datasheets/BST-BME280_DS001-10.pdf
// 
//  - Written in CCS PCH C using floating point math 
//  - Several integer math versions of this driver exist but the speed improvement is 
//    not warranted in typical weather station type applications 
//  
//  - Based on a paper posted to thebackshed.com by  
//    https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout/wiring-and-test  
//   scl = B5, sDA = B4
// 
//    Revision - integer algotihm 
//     
//    07/24/2017 
// 
//  Nguyen Trung Truc 
//  08/17/12 
//************************************************ 
// place a #use i2c statement in the main program and comment this out if not applicable 
//#define BME280_SDA  PIN_C4
//#define BME280_SCL  PIN_C3
//#use i2c(stream=number1, sda=PIN_C4, scl=PIN_C3, MASTER, FAST=400000) 
#use i2c(master, sda=PIN_B4, scl=PIN_B5, FAST, FORCE_SW) // sda = PIN_B4, scl = PIN_B5
#define BME280_ADDRESS 0xEE          // I2C address of BME280 VDD 
#define A_SHRT(var2,y) (var2>0)?(var2>>y):(-((-var2)>>y)) 
#define A_SHLT(x,y) (x>0)?(x<<y):(-((-x)<<y)) 

//#include <math.h> 
//#include<stdint.h>
//calibration values of BME280
        
       static unsigned int16 dig_T1;
       static signed int16  dig_T2;
       static signed int16  dig_T3;
       static unsigned int16  dig_P1;// = 37621;
       static signed int16  dig_P2;
       static signed int16  dig_P3;
       static signed int16  dig_P4;
       static signed int16  dig_P5;
       static signed int16  dig_P6;
       static signed int16  dig_P7;
       static signed int16  dig_P8;
       static signed int16  dig_P9;
       static unsigned int8  dig_H1;
       static signed int16  dig_H2;
       static unsigned int8  dig_H3;
       static unsigned int16  dig_H4;
       static signed int16  dig_H5;
       static signed int8   dig_H6;
       static signed int32 t_fine;//var1, var2;
       //static signed int32 var1_p, var2_p, var3_p, var4_p, var5_p, var6_p, var7_p, var8_p, var9_p, var11_p, var33_p, var44_p, var444_p, var4444_p;
       static unsigned int32 adc_P, adc_T;
       static unsigned int16 adc_H;
       static int8 data[8];
int8 BME280ReadByte(int8 address);
int16 BME280ReadInt(int8 address) ;
int32 BME280Read24(int8 data);
void BME280WriteByte(int8 address, int8 data) ;
int1 BME280ReadingCalibration(void);
void BME280Calibration();
void BME280SetSampling();

void init_BME280()
{
   output_float(PIN_B4);
   output_float(PIN_B5);
}
int1 BME280Begin()
{
   
    // check if sensor, i.e. the chip ID is correct
    if(BME280ReadByte(0xD0) != 0x60)
        return 0;
    // reset the device using soft-reset
    // this makes sure the IIR is off, etc.
    BME280WriteByte(0xE0, 0xB6);

    // wait for chip to wake up.
    delay_ms(300);

    // if chip is still reading calibration, delay
    while ( BME280ReadingCalibration())
          delay_ms(100);

    BME280Calibration(); // read trimming parameters, see DS 4.2.2

    BME280SetSampling(); // use defaults

    return 1;
}
void BME280SetSampling()
{
  
     BME280WriteByte(0xF2,0x03); // oversampling x 4, ctr_hum
     BME280WriteByte(0xF5,0x68); //011 (250ms tstandby) 0100 (filter coefficient =4) 0 (i2c)
     BME280WriteByte(0xF4,0x6E);// 011 011 10 temperature oversampling x 4, pressure oversampling x 4, force mode
    // you must make sure to also set REGISTER_CONTROL after setting the
    // CONTROLHUMID register, otherwise the values won't be applied (see DS 5.4.3)
    // write8(BME280_REGISTER_CONTROLHUMID, _humReg.get());
    // write8(BME280_REGISTER_CONFIG, _configReg.get());
    // write8(BME280_REGISTER_CONTROL, _measReg.get());
}

int8 BME280ReadID() 
//---------------------------------------------- 
{ 
int8 data; 

   i2c_start(); 
   i2c_write(0xEE); // 0X77 11101110X IS IC ADDRESS, 0X76 IS 1110 110
   i2c_write(0xD0); 
   i2c_start(); 
   i2c_write(0xEF ); 
   data=i2c_read(0); 
   i2c_stop(); 
   delay_us(20);
   return(data); 
} 
//----------------------------------------------
byte BME280ReadByte(int8 address) 
//---------------------------------------------- 
{ 
byte data; 

   i2c_start(); 
   i2c_write(BME280_ADDRESS); 
   i2c_write(address); 
   i2c_start(); 
   i2c_write(BME280_ADDRESS | 0x01 ); 
   data=i2c_read(0); 
   i2c_stop(); 
   return(data); 
} 



//---------------------------------------------- 
int16 BME280Read16(int8 address) 
//---------------------------------------------- 
{ 
byte msb, lsb; 
unsigned int16 temp; 

   i2c_start(); 
   i2c_write(BME280_ADDRESS); 
   i2c_write(address); 
   i2c_start(); 
   i2c_write(BME280_ADDRESS | 0x01 ); 
   msb = i2c_read(); 
   lsb = i2c_read(0); 
   i2c_stop(); 
   temp = msb*256 + lsb; //(msb<<8)|lsb;
   return ( temp ); 
} 

/**************************************************************************/
/*!
    @brief  Reads a signed 16 bit value over I2C or SPI
*/
/**************************************************************************/
int16 BME280ReadS16(int8 address)
{
    return ((signed int16)BME280Read16(address));
}

unsigned int16 BME280Read16_LE(int8 address)
{
    unsigned int16 temp;
    temp = BME280Read16(address);
    temp = (temp >> 8) | (temp << 8);
    return (unsigned int16)temp;
}
signed int16 BME280ReadS16_LE(int8 address)
{
    return (signed int16)BME280Read16_LE(address);
}

//---------------------------------------------- 
void BME280WriteByte(int8 address, int8 data) 
//---------------------------------------------- 
{ 
   i2c_start(); 
   i2c_write(BME280_ADDRESS); 
   i2c_write(address); 
   i2c_write(data); 
   i2c_stop(); 
} 

signed int16 BME280ReadDig_H4() 
{ 
   unsigned int16 temp; 
   byte msb, lsb; 
   msb = BME280ReadByte(0xE4); 
   lsb = BME280ReadByte(0xE5); 
   temp = ((int16)msb<<4) + (lsb & 0xF); 
   return temp; 
} 

void BME280Calibration() 
//---------------------------------------------- 
{   
  dig_T1 = BME280Read16_LE(0x88);
  dig_T2 = BME280ReadS16_LE(0x8A);
  dig_T3 = BME280ReadS16_LE(0x8C);
  dig_P1 = BME280Read16_LE(0x8E);
  dig_P2 = BME280ReadS16_LE(0x90);
  dig_P3 = BME280ReadS16_LE(0x92);
  dig_P4 = BME280ReadS16_LE(0x94);
  dig_P5 = BME280ReadS16_LE(0x96);
  dig_P6 = BME280ReadS16_LE(0x98);
  dig_P7 = BME280ReadS16_LE(0x9A);
  dig_P8 = BME280ReadS16_LE(0x9C);
  dig_P9 = BME280ReadS16_LE(0x9E);
  
  dig_H1 = BME280ReadByte(0xA1);
  dig_H2 = BME280ReadS16_LE(0xE1);
  dig_H3 = BME280ReadByte(0xE3);
  dig_H4 = BME280ReadDig_H4(); //285
  dig_H5 = (BME280ReadByte(0xE6)<<4)|(BME280ReadByte(0xE5) >>4) ;
  dig_H6 = (signed int8)BME280ReadByte(0xE7);
  
}

//---------------------------------------------- 


// Read the uncompensated temperature value 
//---------------------------------------------- 

void BurstRead()
{
     // int8 data[8];  
      i2c_start(); 
      i2c_write(BME280_ADDRESS); 
      i2c_write(0xF7); 
      
      i2c_start(); 
      i2c_write(BME280_ADDRESS | 0x01); 
      data[0] = i2c_read(); 
      data[1] = i2c_read();
      data[2] = i2c_read();
      data[3] = i2c_read();
      data[4] = i2c_read();
      data[5] = i2c_read();
      data[6] = i2c_read();
      data[7] = i2c_read();    
      i2c_stop();
     //ata[5]=data[5]&0xF0;
      adc_P = make32(0,data[0],data[1],data[2]);
     // adc_T = make32(0,data[3],data[4]);
      adc_T  = make32(0,data[3],data[4],data[5]);
      adc_H = make16(data[6],data[7]);
      
      // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB) 
}

unsigned int32 BME280Read24(int8 data)
{
 unsigned int32 value;
 int8 msb, lsb, xlsb; 
 
      i2c_start(); 
      i2c_write(BME280_ADDRESS); 
      i2c_write(data); 
      
      i2c_start(); 
      i2c_write(BME280_ADDRESS | 0x01); 
      
      // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB) 
      
      msb = i2c_read(); 
      lsb = i2c_read(); 
      xlsb = i2c_read(0); // NACK on last read 
      i2c_stop(); //*/ 
      
      value = make32(0,msb,lsb,xlsb); // (msb<<16)|(lsb<<8)|xlsb ;

    return value;
}


int1 BME280ReadingCalibration(void)
{
 // unsigned int8 const rStatus = BME280ReadByte(0xF3);
  int8 rStatus = BME280ReadByte(0xF3);
  return (rStatus & (1 << 0)) != 0; // return 1 if bus is busy reading 1<<0  is 1 shifted left 0 times
}

void BME280TakeForcedMeasurement()
{   
    
     BME280WriteByte(0xF4,0x6E); //011 011 01 pressure oversampling x 4, temperature oversampling x 4, forced mode
     while (BME280ReadByte(0xF4) & 0x08)  // wait until measurement has been completed, otherwise we would read the values from the last measurement
      delay_us(10);
    }
    

float BME280ReadTemperature(void)
{

 
   signed int32 var1, var2;

    //signed int32 adc_T = BME280Read24(0xFA); //FA THE TEMPERATURE REGISTER
    if (adc_T == 0x800000) // value in case temp measurement was disabled
        return 0;
    adc_T >>= 4;

    var1 = ((((adc_T>>3) - ((signed int32)dig_T1 <<1)))*((signed int32)dig_T2)) >> 11;
             
    var2 = (((((adc_T>>4) - ((signed int32)dig_T1)) * ((adc_T>>4) - ((signed int32)dig_T1))) >> 12) *((signed int32)dig_T3)) >> 14;
   
   t_fine = var1 + var2;

    float T= (t_fine * 5 + 128) >> 8;
    return T/100;
    
}

unsigned int32 BME280_compensate_P_int32()
{

 //signed int32 adc_P = BME280Read24(0xF7); //FA THE TEMPERATURE REGISTER
if (adc_P == 0x800000) // value in case temp measurement was disabled
        return 0;
    adc_P >>= 4;
    signed int32 var1, var2, var3;
    unsigned int32 p;
    //BME280ReadTemperature(); //must be done to get t_fine
  t_fine = t_fine +2000;
var1 = (((signed int32)t_fine)>>1) - (unsigned int32)64000;
//delay_us(100);
 //var11_p = (int32)(168278)>>1;
var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((signed int32)dig_P6);  // dig_p6 is negative
var2 = var2 + ((var1*((signed int32)dig_P5))<<1);   // dig_p5 is negative therefore left shift is OK ? sfhit left is ok
//var33_p = ((signed int32)dig_P5)<<1;
var2 = A_SHRT(var2,2)+(((signed int32)dig_P4)<<16);
var3=(signed int32)dig_P2 * var1;
var3 = A_SHRT(var3,1);
var3 = ((dig_P3 * ((A_SHRT(var1,2) *A_SHRT(var1,2)) >> 13 )) >> 3)+ var3;
var1= A_SHRT(var3,18);

 //var5_p = var1;
 var1 = (32768+var1)*(signed int32)dig_P1;
 var1 = A_SHRT(var1,15);


if (var1 == 0)
{
return 0; // avoid exception caused by division by zero
}
p = (((unsigned int32)(((signed int32)1048576) - adc_P)- A_SHRT(var2,12)))*3125;
//p = (((unsigned int32)(((signed int32)1048576) - adc_P)-(var2>>12)))*3125;

if (p < 0x80000000)
{
p = (p << 1) / ((unsigned int32)var1);
}
else
{
p = (p / (unsigned int32)var1) * 2;
}
var1 = (((signed int32)dig_P9) * ((signed int32)(((p>>3) * (p>>3))>>13)))>>12;

//var2 = (((signed int32)(p>>2)) * ((signed int32)dig_P8))>>13;
var2 = (((signed int32)(p>>2)) * ((signed int32)dig_P8));
var2 = A_SHRT(var2,13);
//var2 = (((signed int32)(p>>2)) * ((signed int32)dig_P8))>>13;

p = (unsigned int32)((signed int32)p + ((signed int32)(var1 + var2 + dig_P7) >> 4));
return p;
}
   
float Calculate_Pess() 
{ 
   float var1, var2, p; 
    if (adc_P == 0x800000) // value in case temp measurement was disabled
        return 0;
    adc_P >>= 4;
   var1 = ((float)t_fine/2.0) - 64000.0; 
   var2 = var1 * var1 * ((float)dig_P6) / 32768.0; 
   var2 = var2 + var1 * ((float)dig_P5) * 2.0; 
   var2 = (var2/4.0)+(((float)dig_P4) * 65536.0); 
   var1 = (((float)dig_P3) * var1 * var1 / 524288.0 + ((float)dig_P2) * var1) / 524288.0; 
   var1 = (1.0 + var1 / 32768.0)*((float)dig_P1); 
   if (var1 == 0.0) 
   { 
   //return 0; // avoid exception caused by division by zero 
   } 
   p = 1048576.0 - (float)adc_P; 
   p = (p - (var2 / 4096.0)) * 6250.0 / var1; 
   var1 = ((float)dig_P9) * p * p / 2147483648.0; 
   var2 = p * ((float)dig_P8) / 32768.0; 
   p = p + (var1 + var2 + ((float)dig_P7)) / 16.0; 
return p/100;
   
} 
 

float BME280ReadHumidity(void) {
    //readTemperature(); // must be done first to get 
    

    int32 adc_H = BME280Read16(0xFD);// BME280_REGISTER_HUMIDDATA
    
    if (adc_H == 0x8000) // value in case humidity measurement was disabled
        return 0;
    signed int32  v_x1_u32r;

    v_x1_u32r = (t_fine - ((signed int32)76800));

    v_x1_u32r = (((((adc_H << 14) - (((int32)dig_H4) << 20) -
                    (((signed int32)dig_H5) * v_x1_u32r)) + ((signed int32)16384)) >> 15) *
                 (((((((v_x1_u32r * ((int32)dig_H6)) >> 10) *
                      (((v_x1_u32r * ((signed int32)dig_H3)) >> 11) + ((signed int32)32768))) >> 10) +
                    ((signed int32)2097152)) * ((signed int32)dig_H2) + 8192) >> 14));

    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7)*((signed int32)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    float h = (v_x1_u32r>>12);
    return  h / 1024.0;
}

// Read the uncompensated pressure value 
//---------------------------------------------- 



