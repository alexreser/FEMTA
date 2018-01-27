
#include <stdlib.h>
#include <pigpio.h>
#include <stdio.h>

#define RED   "\e[0;31m"
#define GREY  "\e[0;30m"
#define GREEN "\e[0;32m"
#define RESET "\e[0m"

#define NUMBER_OF_MODULES 4

typedef struct pin {

  char state;                    // The input output state of the pin
  char logical;                  // The logical broadcom number of the pin
  char physical;                 // The physical pin number on the board
  
  union {
    char voltage;                // The voltage when not under pulse width modulation
    unsigned char duty_cycle;    // The duty cycle for pulse width modulation
  };
  
} pin;

typedef struct module {

  char * identifier;             // The name of the module
  pin  * pins;                   // The pins bound to the module
  char n_pins;                   // The number of pins bound to the module

} module;

module ** modules, * BNO, * MPU, * Valve, * FEMTA;

void initialize_pin(pin * initialent, char logical, char physical, short state) {
  initialent -> state    = state;
  initialent -> logical  = logical;
  initialent -> physical = physical;

  initialent -> voltage = 0;
  gpioSetMode(logical, state);
  if (state == PI_OUTPUT) gpioWrite(logical, 0);
}

void initialize_satellite() {

  // Exit if gpio's are unavailable for some reason
  if (gpioInitialise() < 0) {
    printf(RED "a critical error has occured\n" RESET);
    exit(1);
  }
  
  // Get space for modules
  modules = malloc(NUMBER_OF_MODULES * sizeof(module *));
  for (char m = 0; m < NUMBER_OF_MODULES; m++) modules[m] = malloc(sizeof(module));

  // All modules should be grouped together
  BNO   = modules[0];
  MPU   = modules[1];
  Valve = modules[2];
  FEMTA = modules[3];

  // Set module identifiers for printing
  BNO   -> identifier = "BNO 055";
  MPU   -> identifier = "MPU 9250";
  Valve -> identifier = "Valve";
  FEMTA -> identifier = "FEMTA";

  // Set each module's number of pins
  BNO   -> n_pins = 3;
  MPU   -> n_pins = 2;
  Valve -> n_pins = 1;
  FEMTA -> n_pins = 4;

  // Get space for module pin arrays
  for (char m = 0; m < NUMBER_OF_MODULES; m++)
    modules[m] -> pins = malloc((modules[m] -> n_pins) * sizeof(module));

  // The BNO has the UART interface
  initialize_pin(&(BNO -> pins[0]), 14,  8, PI_INPUT);   // UART TXD
  initialize_pin(&(BNO -> pins[1]), 15, 10, PI_INPUT);   // UART RXD
  initialize_pin(&(BNO -> pins[2]), 23, 16, PI_OUTPUT);

  // The MPU has the I2C interface
  initialize_pin(&(MPU -> pins[0]),  2,  3, PI_INPUT);   // I2C SDA
  initialize_pin(&(MPU -> pins[1]),  3,  5, PI_INPUT);   // I2C SCL

  // The Valve is controlled via digital states
  initialize_pin(&(Valve -> pins[0]), 17, 11, PI_OUTPUT);
  
  // The FEMTA is controlled via pulse width modulation
  initialize_pin(&(FEMTA -> pins[0]), 24, 18, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[1]), 25, 22, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[2]), 27, 13, PI_OUTPUT);
  initialize_pin(&(FEMTA -> pins[3]), 22, 15, PI_OUTPUT);

  printf("\n" GREEN "satellite initialized successfully!" RESET "\n\n");
}

void print_configuration() {

  // prints out a table indicating the allocation map
  printf(GREY "\tlogical   physical   state\n" RESET);
  for (char m = 0; m < NUMBER_OF_MODULES; m++) {
    printf(GREY "%s\n" RESET, modules[m] -> identifier);
    for (char p = 0; p < modules[m] -> n_pins; p++) {
      if (modules[m] -> pins[p].logical < 10) printf(" ");
      printf("        %d",   modules[m] -> pins[p].logical);
      if (modules[m] -> pins[p].physical < 10) printf(" ");
      printf("        %d",   modules[m] -> pins[p].physical);
      if (modules[m] -> pins[p].state < 10) printf(" ");
      printf("        %d\n", modules[m] -> pins[p].state);
    }
    printf("\n");
  }
}

void terminate_satellite() {

  // Set all output pins to 0 before exit
  for (char m = 0; m < NUMBER_OF_MODULES; m++) {
    for (char p = 0; p < modules[m] -> n_pins; p++) {
      if (modules[m] -> pins[p].state == PI_OUTPUT) gpioWrite(modules[m] -> pins[p].logical, 0);
    }
  }
  
  gpioTerminate();
}

void check_if_writeable(pin * p) {

  // scream bloody hell if this pin is read-only
  if (p -> state == PI_INPUT) {
    printf(RED "read-only pin tried to perform pulse width modulation" RESET);
    exit(p -> logical);
  }
}

void check_if_readable(pin * p) {

  // scream bloody hell if this pin is write-only
  if (p -> state == PI_OUTPUT) {
    printf(RED "write-only pin tried to perform pulse width modulation" RESET);
    exit(p -> logical);
  }
}

char read_voltage(pin * p) {

  check_if_readable(p);
  p -> voltage = gpioRead(p -> logical);
  return p -> voltage;
}

void set_voltage(pin * p, char voltage) {

  check_if_writeable(p);
  p -> voltage = voltage;
  gpioWrite(p -> logical, p -> voltage);
}

void set_pwm(pin * p, unsigned char duty_cycle) {

  check_if_writeable(p);
  p -> duty_cycle = duty_cycle;
  gpioPWM(p -> logical, p -> duty_cycle);
}

int main() {

  initialize_satellite();
  print_configuration();

  
  
  terminate_satellite();
  return 0;
}
