// Copyright (c) 2014-2015 by Wayne C. Gramlich.  All rights reserved.
//
// This code drives the bus_beagle_bone_black board.

#include <Bus_Slave.h>
#include <Frame_Buffer.h>
#include <bus_server.h>

#define TEST TEST_RAB_FREYA

class Freya_Motor_Encoder : Bus_Motor_Encoder {
 public:
  Freya_Motor_Encoder(Bus_Slave *bus_slave, UShort address, 
   UByte encoder_get_command, UByte encoder_set_command,
   UByte pwm_set_command);
  Integer encoder_get();
  void encoder_set(Integer encoder);
  void pwm_set(Byte pwm);
 private:
  UShort _address;
  Bus_Slave *_bus_slave;
  UByte _encoder_get_command;
  UByte _encoder_set_command;
  UByte _pwm_set_command;
};

Freya_Motor_Encoder::Freya_Motor_Encoder(Bus_Slave *bus_slave, UShort address,
 UByte encoder_get_command, UByte encoder_set_command, UByte pwm_set_command) {
  _address = address;
  _bus_slave = bus_slave;
  _encoder_get_command = encoder_get_command;
  _encoder_set_command = encoder_set_command;
  _pwm_set_command = pwm_set_command;
}

void Freya_Motor_Encoder::pwm_set(Byte pwm) {
  _bus_slave->command_byte_put(_address, _pwm_set_command, pwm);
  _bus_slave->flush();
}

Integer Freya_Motor_Encoder::encoder_get() {
  return _bus_slave->command_integer_get(_address, _encoder_get_command);
}

void Freya_Motor_Encoder::encoder_set(Integer encoder) {
  _bus_slave->command_integer_put(_address, _encoder_set_command, encoder);
  _bus_slave->flush();
}

// The *setup* routine runs on power up and when you press reset:

// Define the UART's:
NULL_UART null_uart;
AVR_UART *bus_uart = &avr_uart1;
AVR_UART *debug_uart = &avr_uart0;
AVR_UART *host_uart = &avr_uart0;

Bus_Slave bus_slave((UART *)bus_uart, (UART *)host_uart);
static const UShort address = 33;
Freya_Motor_Encoder left_motor_encoder(&bus_slave, address, 2, 3, 9);
Freya_Motor_Encoder right_motor_encoder(&bus_slave, address, 4, 5, 11);

Bridge bridge(&avr_uart0, &avr_uart1, &avr_uart0, &bus_slave,
 (Bus_Motor_Encoder *)&left_motor_encoder,
 (Bus_Motor_Encoder *)&right_motor_encoder);

void setup() {
  bridge.setup(TEST);
}

void loop () {
  bridge.loop(TEST);
}
