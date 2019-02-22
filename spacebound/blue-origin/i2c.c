

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <pigpio.h>
#include <time.h>

#include "clock.h"
#include "color.h"
#include "i2c.h"
#include "list.h"
#include "sensor.h"
#include "types.h"

void * i2c_main();

i2c_device * create_i2c_device(Sensor * sensor, uint8 address, i2c_reader reader, uint16 interval) {
  // creates an i2c device, adding it to the device list
  
  i2c_device * i2c = malloc(sizeof(i2c_device));
  
  i2c -> sensor   = sensor;
  i2c -> read     = reader;
  i2c -> interval = interval;
  
  i2c -> count = 0;

  i2c -> file   = NULL;
  i2c -> buffer = NULL;
  
  i2c -> handle = i2cOpen(1, address, 0);
  
  printf("Added i2c device %d\n", i2c -> handle);
  
  list_insert(schedule -> devices, i2c);
  
  return i2c;
}

void i2c_freer(void * device_ptr) {
  // closes and frees the i2c device
  
  i2c_device * i2c = (i2c_device *) device_ptr;
  
  fclose(i2c -> file); 
  
  i2cClose(i2c -> handle);
  free(i2c);
}

void init_i2c() {
  //
  
  schedule = malloc(sizeof(i2c_schedule));
  
  schedule -> devices = create_list(SLL, i2c_freer);
  schedule -> thread  = malloc(sizeof(pthread));
  
  // open communication with the i2c bus
  
  
}

void start_i2c() {
  
  // create i2c thread
  if (pthread_create(schedule -> thread, NULL, i2c_main, NULL)) {
    printf(RED "Could not start i2c thread\n" RESET);
    return;  
  }
}

uint8 i2c_read_byte(i2c_device * dev, uint8 reg) {
  // reads a single byte from an i2c device
  
  return i2cReadByteData(dev -> handle, reg);
}

void i2c_read_bytes(i2c_device * dev, uint8 reg, uint8 * buf, char n) {
  // reads up to 32 bytes from an i2c device
  
  if (i2cReadI2CBlockData(dev -> handle, reg, buf, n) < 0) {
    printf(RED "Could not read bytes from " YELLOW "%s\n" RESET, dev -> sensor -> name);
    exit(3);
  }
}

void i2c_raw_read(i2c_device * dev, uint8 * buf, char n) {
  // reads up to 32 bytes from an i2c device, without asking for a particular register
  
  if (i2cReadDevice(dev -> handle, buf, n)) {
    printf(RED "Could not read raw bytes from " YELLOW "%s\n" RESET, dev -> sensor -> name);
    exit(3);
  }
}

void i2c_raw_write(i2c_device * dev, uint8 * buf, char n){
  // writes up to 32 bytes from an i2c device, without specifying a particular register
  
  if (i2cWriteDevice(dev -> handle, buf, n)) {
    printf(RED "Could not write raw bytes from " YELLOW "%s\n" RESET, dev -> sensor -> name);
    exit(3);
  }
}

void i2c_write_byte(i2c_device * dev, uint8 reg, uint8 value) {
  // writes a byte to the i2c device
  
  if (i2cWriteByteData(dev -> handle, reg, value) < 0) {
    printf(RED "Could not write byte to " YELLOW "%s\n" RESET, dev -> sensor -> name);
    exit(3);
  }
}

void i2c_write_bytes(i2c_device * dev, uint8 reg, uint8 * buf, char n) {
  // writes up to 32 bytes to the i2c device
  
  if (i2cWriteI2CBlockData(dev -> handle, reg, buf, n)) {
    printf(RED "Could not write bytes to " YELLOW "%s\n" RESET, dev -> sensor -> name);
    exit(3);
  }
}

void * i2c_main() {
  
  FILE * i2c_log = fopen("logs/i2c.log", "a");
  fprintf(i2c_log, GRAY "Read duration [ns]\n" RESET);
  
  long last_read_duration = 0;    // tracks time taken to read i2c bus
  
  while (!schedule -> term_signal) {
    
    // get time before we perform the read
    struct timespec pre_read_time;
    clock_gettime(CLOCK_REALTIME, &pre_read_time);
    
    fprintf(i2c_log, "%ld\n", last_read_duration);
    
    // read the sensors
    for (Node * node = schedule -> devices -> head; node; node = node -> next) {
      
      i2c_device * i2c = (i2c_device *) node -> value;
      
      i2c -> count += 10;
      
      if (i2c -> count == i2c -> interval) {
        
        (i2c -> read)(i2c);
        
        i2c -> count = 0;
      }
    }
    
    // figure out how long to sleep
    long read_duration = real_time_diff(&pre_read_time);
    
    long time_remaining = 1E7 - read_duration;
    
    if (time_remaining < 0)
      time_remaining = 0;               // taking too long to read!
    
    last_read_duration = read_duration;
    
    real_nano_sleep(time_remaining);   // 10ms minus time it took to read sensors
  }
  
  fclose(i2c_log);
}


void terminate_i2c() {
  // frees everything associated with the i2c system
  
  list_destroy(schedule -> devices);      // note that this kills
  
  free(schedule -> thread);
  free(schedule);
}