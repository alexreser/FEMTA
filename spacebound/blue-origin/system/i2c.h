#ifndef HEADER_GAURD_I2C
#define HEADER_GAURD_I2C

#include <pthread.h>
#include <stdbool.h>

#include "../structures/list.h"
#include "../types/types.h"
#include "../types/thread-types.h"
#include "../sensors/sensor.h"

typedef struct Sensor Sensor;
typedef struct i2c_device i2c_device;

typedef bool (* i2c_reader)(i2c_device * i2c);

typedef struct i2c_device {
  
  Sensor * sensor;
  
  FILE * file;             // log file
  char * buffer;           // buffer for file I/O
  
  uint8 address;           // address on bus
  
  ushort interval;         // time span between reads in ms
  ushort count;            // counts since last read
  
  int handle;              //

  int total_reads;         // total times this sensor has been read
  
  i2c_reader read;
  
} i2c_device;


typedef struct i2c_schedule {
  
  List * devices;      // list of all i2c device pointers
  
  long interval;
  
  Thread * thread;     // i2c thread 
  bool term_signal;    // when set to true, schedule terminates
  
} i2c_schedule;

i2c_schedule * schedule;

void init_i2c();
void start_i2c();

void terminate_i2c();

bool i2c_write_byte (i2c_device * dev, uint8 reg, uint8 value);
bool i2c_write_bytes(i2c_device * dev, uint8 reg, uint8 * buf, char n);
bool i2c_read_bytes (i2c_device * dev, uint8 reg, uint8 * buf, char n);
bool i2c_raw_write  (i2c_device * dev,            uint8 * buf, char n);
bool i2c_raw_read   (i2c_device * dev,            uint8 * buf, char n);

uint8 i2c_read_byte(i2c_device * dev, uint8 reg);

i2c_device * create_i2c_device(Sensor * sensor, uint8 address, i2c_reader reader, uint16 interval);

#endif