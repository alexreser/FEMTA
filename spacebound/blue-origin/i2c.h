#ifndef HEADER_GAURD_I2C
#define HEADER_GAURD_I2C

#include <pthread.h>
#include <stdbool.h>

#include "list.h"
#include "types.h"

#include "sensor.h"

typedef struct Sensor Sensor;
typedef struct i2c_device i2c_device;

typedef pthread_t pthread;

typedef bool (* i2c_reader)(i2c_device * i2c);


typedef struct i2c_pattern {
  
  void * todo;
  
} i2c_pattern;


typedef struct i2c_device {

  Sensor * sensor;
  
  uint8 address;          // address on bus
  
  uchar interval;         // time span between reads in ms
  uchar count;            // counts since last read
  
  i2c_pattern pattern;    // 
  
  i2c_reader read;
  
} i2c_device;



typedef struct i2c_schedule {
  
  
  pthread * thread;
  bool term_signal;    // when set to true, schedule terminates
  
} i2c_schedule;


i2c_schedule * schedule;

void init_i2c();

i2c_device * create_i2c_device(Sensor * sensor, uint8 address, i2c_reader reader, uint8 interval);



#endif
