// Copyright (c) 2014-2015 by Wayne C. Gramlich.  All rights reserved.
//
// This code drives the bus_beagle_bone_black board.

#include <Bus_Slave.h>
#include <Frame_Buffer.h>
#include <bus_server.h>

#define TEST TEST_RAB_FREYA

// The *setup* routine runs on power up and when you press reset:

Bridge bridge(&avr_uart0, &avr_uart1, &avr_uart0);

void setup() {
  bridge.setup(TEST);
}

void loop () {
  bridge.loop(TEST);
}
