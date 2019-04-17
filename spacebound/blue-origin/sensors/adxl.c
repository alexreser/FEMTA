

#include <stdlib.h>
#include <stdio.h>

#include "adxl.h"
#include "ds32.h"

#include "../system/color.h"
#include "../system/i2c.h"

const float adxl_bias_x =  0.0371;
const float adxl_bias_y = -0.0010;
const float adxl_bias_z = -0.0861;


void free_adxl(Sensor * adxl);
bool read_adxl(i2c_device * adxl_i2c);

Sensor * init_adxl(ProtoSensor * proto) {
  
  Sensor * adxl = malloc(sizeof(Sensor));
  
  adxl -> name = "ADXL345";
  adxl -> free = free_adxl;
  
  adxl -> i2c = create_i2c_device(adxl, ADXL_ADDRESS, read_adxl, 1000 / proto -> hertz);
  
  adxl -> i2c -> file = fopen("logs/adxl.log", "a");
  
  fprintf(adxl -> i2c -> file, RED "\n\nADXL345\nStart time %s\nAccel x\tAccel y\tAccel z\n" RESET, formatted_time);
  
  // set up data format for acceleration measurements (page 28)
  // tell adxl to use full resolution when measuring acceleration (bit    3)
  // tell adxl to use full range      when measuring acceleration (bits 1,2)
  i2c_write_byte(adxl -> i2c, 0x31, 0b00001011);    
  
  // tell adxl to bypass it's FIFO queue (page 28)
  i2c_write_byte(adxl -> i2c, 0x38, 0b00000000);
  
  // tell adxl to enter measurement mode (page 26)
  i2c_write_byte(adxl -> i2c, 0x2D, 0b00001000);    // bit 3 indicates measure mode
  
  
  printf("Started " GREEN "%s " RESET "at " YELLOW "%dHz " RESET "on " BLUE "0x%x " RESET,
	 adxl -> name, proto -> hertz, ADXL_ADDRESS);

  if (proto -> print) printf("with " MAGENTA "printing\n" RESET);
  else                printf("\n");
  
  printf("logged in logs/adxl.log\n");
  printf("An accelerometer\n\n");
  
  return adxl;
}

bool read_adxl(i2c_device * adxl_i2c) {
  
  uint8 accel_raws[6];
  
  if (!i2c_read_bytes(adxl_i2c, 0x32, accel_raws, 6)) return false;
  
  int16 xAccel = (accel_raws[1] << 8) | accel_raws[0];
  int16 yAccel = (accel_raws[3] << 8) | accel_raws[2];
  int16 zAccel = (accel_raws[5] << 8) | accel_raws[4];

  /*int xAccel = (accel_raws[0] << 8) | accel_raws[1];
  int yAccel = (accel_raws[2] << 8) | accel_raws[3];
  int zAccel = (accel_raws[4] << 8) | accel_raws[5];*/

  
  //printf("%d, %d, %d\n", xAccel, yAccel, zAccel);
  //printf("Zenith %f g\n", zAccel * 3.9 / 1000.0);
  /*printf("d:%f %f %f\n",
	 xAccel * 3.9 / 1000.0,
	 yAccel * 3.9 / 1000.0,
	 zAccel * 3.9 / 1000.0);*/

  //printf("d:%f %f %f\n",
  
  fprintf(adxl_i2c -> file, "%.3f\t%.3f\t%.3f\n",
	  xAccel * 0.004 - adxl_bias_x,
	  yAccel * 0.004 - adxl_bias_y,
	  zAccel * 0.004 - adxl_bias_z);
  
  return true;
}

void free_adxl(Sensor * adxl) {
  
  
  
}