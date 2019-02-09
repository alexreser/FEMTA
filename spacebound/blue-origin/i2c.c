

#include <stdlib.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>

#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

#include "i2c.h"
#include "sensor.h"
#include "adxl.h"
#include "color.h"
#include "clock.h"

void * i2c_main();

i2c_device * create_i2c_device(Sensor * sensor, uint8 address, i2c_reader reader, uint8 interval) {
  
  i2c_device * i2c = malloc(sizeof(i2c_device));
  
  i2c -> sensor   = sensor;
  i2c -> read     = reader;
  i2c -> interval = interval;

  i2c -> count = 0;
  
  return i2c;
}

void init_i2c() {
  
  // add all the sensors present on the system
  
  schedule = malloc(sizeof(i2c_schedule));
  
  schedule -> thread = malloc(sizeof(pthread));
  
  // create i2c thread
  if (pthread_create(schedule -> thread, NULL, i2c_main, NULL)) {
    printf(CONSOLE_RED "Could not start i2c!\n" CONSOLE_RESET);
    return;
  }
}


void * i2c_main() {
  
  while (!schedule -> term_signal) {
    
    for (Node * node = sensors -> head; node; node = node -> next) {
      
      Sensor * sensor = (Sensor *) node -> value;
      
      i2c_device * i2c = sensor -> i2c;

      i2c -> count += 10;
      
      if (i2c -> count == i2c -> interval) {
        
        (i2c -> read)(i2c);
        
        i2c -> count = 0;
      }
    }
    
    real_nano_sleep(1E5);    // 10ms
  }
  
}


/*int main() {
  
  int fd = open("/dev/i2c-1", O_RDWR);
  
  if (fd < 0) {
    printf("Error opening file: %s\n", strerror(errno));
    return 1;
  }
  
  if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
    printf("ioctl error: %s\n", strerror(errno));
    return 1;
  }
  
  /*buffer[0]=0xFF;
    write(fd, buffer, 1);*/

/*char buffer[1] = {
    0x00,
  };
  write(fd, buffer, 1);
  
  read(fd, buffer, 1);
  
  printf("0x%02X\n", buffer[0]);
  return 0;
}
*/
