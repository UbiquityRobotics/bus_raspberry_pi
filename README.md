# bus_raspberry_pi

Raspberry Pi that can be connected to a bus.

The bus is a half-duplex multi-drop bus that runs at 500 Kbps.
It uses a Microchip MCP2562 CAN bus transceivers to connect to
the bus.

## Firmware

The software environment for the bus_raspberry_pi is more complicated
than we would like.  The bus_raspberry_pi deliberately uses
an ATmega324P microcontroller to be as compatible as possible
with the Arduino community.  Unfortunately, about the only
board out there that used the '324P is the Sanguino and it
rapidly fell out of use.  Thus, '324P support is a bit more
complicated than we would like.  The key thing is that we have
forked a version of the Arudino repository to add '324P support.
Please note, that our fork of the Arduino system only adds '324P
support and nothing else.  Thus, we should have no problems tracking
the main portions of the Arduino code base.

In addition, the Arduino-IDE does not play nice with version
control systems.  So, we use a fairly popular alternative called
`Arduino-Makefile` which uses fairly standard Makefiles and
standalone editors (e.g. `vim` or `emacs`) to develop Arduino
code that is complatible with the Arduino IDE.

### Installing the Bootloader

(To be written up)

### Installing the Arduino-Makefile Development Environment

To set things up:

        cd .../catkin_ws/src
        git clone https://github.com/UbiquityRobotics/Arduino-Makefile.git
        git clone https://github.com/UbiquityRobotics/bus_common.git
        git clone https://github.com/UbiquityRobotics/bus_raspberry_pi.git
        git clone https://github.com/UbiquityRobotics/bus_server.git
        git clone https://github.com/UbiquityRobotics/bus_slave.git
        # The following command downloads ~2GB of version control stuff.
	# Even with a fast link it takes a while...
        git clone https://github.com/UbiquityRobotics/Arduino.git

Next, we need to bring over all the compilers and stuff to support
the AVR compilation environment:

        sudo apt-get avr-libc avrdude binutils-avr gcc-avr
	# We also need GNU make:
        sudo apt-get install build-essential

When all of the dust settles, you should be able to do the following:

        cd .../catkin_ws/src/bus_raspberry_pi
        make
        make upload       # Read below for more about uploading

The upload target expects you to have USB-serial cable
plugged into your laptop/desktop.  We use the DFRobot
FTDI Basic Breakout board.  This board has two features
that we like:

* One you can select between 3.3V and 5V.  We always use 3.3V.
  Ground is pin 1 and plugs into pin 1 of N4 on the
  bus_raspberry_pi.

* It puts DTR on pin 6 instead of RTS.  The DTR signal is
  what the Arduinos use for their reset signal.

This board requires a USB-A to mini-USB-B cable.  We purchased
ours from Jameco (part number: 2152243.)

### The Firmware

The actual code that implements the firmware starts in
`bus_raspberry_pi.ino`.  The key line is:

        #define TEST TEST_...

The values for `TEST_...` are found in `../bus_server/bus_server.h`:

* `TEST_BUS_OUTPUT` outputs a stream of chacters on the serial port.
* `TEST_BUS_ECHO` is a strange bus debugging mode (too hard to explain.)
* `TEST_BUS_COMMAND` blinks an LED on the motor controller board.
* `TEST_BUS_BRIDGE` runs the software as a bus bridge.
* `TEST_BUS_LINE` runs the ROS Arduino Bridge protocol.

## Revision A

The schematic can be found in the `rev_a` folder as
`bus_raspberry_pi.pdf`.

### Power Supply

The power comes in on either N1 or N2.  These two connectors
are "daisy-chained" so that power can be routed to other boards.
The power goes through Q1 -- a P-channel MOSFET with a very low
R<Sub>ds</Sub>.  Q1 is used to prevent reverse polarity damage
just in case the power comes in reversed.  The power continues
to N6 which is a 5V DC-to-DC power converter that provides up
to 1.5 Amperes of output.  The 5V converter takes a nominal 24V
input with an absolute maximum of 36V.

### Bus Connection

The bus connection is on N3, which is a 2x5 .1 inch pitch
shrouded header.  LGND1 and LGND2 are connected to ground.
CANH and CANL are routed over to U2 which is the CAN bus
(more correctly -- a ISO-11898) transceiver -- a Microchip
MCP2562.  The TXD and RXD outputs are routed to the RXD1 and
TXD1 pins on the ATmega324P processor (U3).

### ISP Connection

The ISP connector is the 2x3 .1 inch pitch connector (N7)
that is connected to the MOSI, MISO, #RESET, and SCK pins
on the processor (U3).

### JTAG Connection

The JTAG connector is N9 which is also a .1 inch 2x5 shrouded
header.  The TDI, TDO, DMS, TCK, and #RESET lines are connected
to the appropriate JTAG pins on the processor (U3).

### 3.3 vs 5V Volt Issues

The processor (U3), AND gates (U1), and CAN bus transceiver (U2)
all run at 5V.  The USB to serial connectors (N4, N5) and the
Raspberry Pi connector (N10) use 3.3V signal levels.  Thus,
there are numerous places where 3.3V vs. 5V signal levels have
to be dealt with.

The signal level conversion is done with a 74HCT08 quad 2-input
AND gate (U1).  The HCT logic family is capable of converting TTL
voltage levels into "high" and "low" signals.  In particular,
the HCT logic family treats any voltage above 2.0V as a logic
"high."

There are 7 separate 22K/33K voltage dividers used for voltage
level issues.  The seven 22K resistors are contained in an 8-pin
SIP (Serial In-line Package.)  These seven resistors are named
R1A through R1F.  Likewise, the seven 33K resistors are contained
in another 8-pin SIP and are named R2A through R2F.  The voltage
dividers are deliberately arranged so that they have the letter
suffix -- R1A/R2A, ... , R1F/R2F.

The nominal center tap voltage of a 22K/33K voltage divider is:

    Vin * 33K/(22K+33K) = Vin * 3/5.

With Vin equal to 5 volts, the center tap voltage is 5V * 3/5 = 3V.

These 8-pin serial in-line packages have a resistance tolerance
of 2%.  This results in a center tap voltage range from 2.88V
to 3.12V.  2.88V is greater than the 2.0 Vih<Sub>min</Sub> for
the HCT logic family.  2.88V is also greater than .7*Vcc
(=.7*3.3V=2.31V) which is the standard Vin<Sub>min</Sub> for
standard CMOS.  3.12V is less than VCC (=3.3V).  Hence, the
voltage divider does not produce illegal 3.3V signaling levels.

Whenever a 5V output needs to be converted from 5 volts to 3V,
it directly fed into a voltage divider.  This occurs with R1D/R2D
connected to TXD0 of the processor (U4) and R1E/R2E and the
output of U1C.

The remaining five voltage dividers are all inputs of the 
74HCT08 quad AND gate package.  There are three cases to
be considered:

* When there is no connection, the voltage divider will 
  pull the input pin to a nominal 3V which is treated as
  a logic high.

* a 3.3V signal is connected, it will be at either 0V or 3.3V.
  In either case, the provided 3.3V voltage signal "overrides"
  the voltage divider.

* For the USB to serial connectors (N4, N5), it is possible
  to accidentally connect in a 5V USB to serial cable.  Again,
  in this case, the provided 5V voltage signal "overrides"
  the voltage divider.  The 74HCT05 is perfectly happy to
  tolerate a 5V input signal.

### Reset Circuitry

The reset signal can be sourced from four locations:

* The manual reset push button -- SW1,

* The #NRST on the JTAG connector (N9),

* The GPIO18 pin on N10, and

* The #RTS signal (3.3V) on the programming FTDI connector (N4),

The push button is the simplest.  It connects the #RESET line
to ground when pushed.  Done.

Similarly, the #NRST line on the JTAG connector (N9) can be
driven low by a JTAG programmer/debugger.  Done.

Both the #RTS signal from N4 and the GPIO18 pin from N10
are used to trigger temporary reset pulse.  The basic
circuit is C6, D1 and R4 for #RTS/N4 and C8, D1 and R4.
Since the circuit is basically the same for both branches,
only the #RTS/N4 branch is explained.

The reset circuit from #RTS/N4 adheres to the Arduino defacto
standard for reset.  The signal comes in on pin 2 of U1A.
A voltage divider R1B/R2B holds the line at a nominal 3V
if no serial cable is plugged in.  Pin 1 of U1A is tied
to 5V so that U1A operates as a simple 3.3V to 5V voltage
level converter.

When the Arduino programmer (usually AVRDUDE) opens the connection
to the serial cable, #RTS (Request to Send) is driven low
for the duration of the programming session.  Capacitor C6, D1,
and R4 are used to convert the falling edge of #RTS into a
pulse onto the #RESET line.

* Prior to asserting #RTS low, the U1A 5V output and the
  R4 10K pull-up resistor hold both ends of capacitor C6
  at 5V.  This completely discharges C6.

* When #RTS goes low at the start of programming, the U1A
  output also goes low.  Since there is no voltage across
  C6, the other end of C6 is driven low as well.  This
  causes the #RESET line to be asserted low.

* Now C6 starts getting charged via R4 and starts to develop
  5V across C6.  Eventually the voltage across C6 will get
  high enough to de-assert #RESET and the processor (U3)
  will start to run the boot-loader.o

* When the programming session is over, #RTS is driven high.
  The causes the output of C6 to attempt to jump from 5V
  up to 10V.

<Pre>
        10V |                           *
            |                           **
            |                           * ***
            |                           *    ****
            |                           *        *****
        5V  ********              *******             *******
            |      *        ******      ^
            |      *    ****            |
            |      * ***                |
            |      **                   |
        0V  +------*--------------------|----------------------->
                                        |
                   ^                    |
                   |                    |
               #RTS goes low       #RTS goes high
</Pre>

* To prevent the voltage from going up to 10V, the Schottky
  diode D1 clamps the voltage to no more than 5.2V by very
  rapidly discharging C6.

The reason why a Schottky diode is used instead of a silicon
diode is because it unclear what the voltage tolerance of #NRST
of the JTAG programmer connector (N9) and the #RESET connector
of the ISP connector (N7) are.  By using a Schottky diode,
the voltage is clamped to 5.2V rather than 5.7.

### RXD and TXD Considerations

In general, the primary purpose of this board is to connect
TXD0 and RXD1 to GPIO15_RXD and GPIO14_TXD respectively.
In addition, it is desirable to allow the programming
connector (N5) to be used to program the processor (U4)
*and* to use the console connector (N5) to view the console
traffic from the Raspberry Pi on boot up.  To accomplish this
task some additional circuitry is used to OR the serial
cable TXD signals into the correct locations.

A key thing to understand about serial connections is that
the line idles "high".  U1B and U1D are AND gates.  Using
De Morgan's law, A.B = ~(~A + ~B).  Thus, using 2-input AND
gates, the signals a correctly OR'ed together so that when
both inputs are idling "high", the output is also "high".

There are three paths to follow:

* Processor (U4) <==> Raspberry Pi (N9),

* Processor (U4) <==> programmer connector (N2), and

* Processor (U4) <==> console connector (N3).

#### Processor to Raspberry Pi

In this situation, the 5V TXD0 output on the processor (U3)
is routed through the R1D/2D voltage divider to bring the
voltage down to a nominal 3.0V.  This signal is routed to U1C.
U1C converts the 3.0V signal to a 5V output which is again
divided back down to 3.0V via R1E/R2E.  This 3.0V signal goes
into GPIO15_RXD on the Raspberry Pi.

In the reverse direction, the 3.3V output of GPIO15_TXD is
sent through the R1G/R2G voltage divider U1B which converts
it to a 5V output and directs it to RXD0 on the processor (U3).

#### Processor to Programmer Connector

In this situation, the 5V TXD0 output on the processor (U3)
is voltage divided by R1D/R2D down to 3.0V and goes to RXD
on the programmer connector (N2).

The 3.3V TXD output on the programmer connector (N2) is
routed to U1B which converts it up to 5V and routes the
signal into RXD0 input on the processor (U4).

#### Processor to Console Connector

In this situation, the 5V TXD0 output on processor (U4)
is voltage divided R1D/R2D down to 3.0V and to both U1C
and the RXD signal of N4.

The GPIO14_TXD is routed through the R1G/R2G voltage
divider directly to the RXD pin of N5.

### LED Circuit

The LED circuit is provides a single LED (D2) that can be

* connected to the power supply to provide a power on LED, or

* connected to PB7/SCK (aka the Arduino D13 pin) of the
   processor (U3), or

* can be left unconnected.

This is accomplished using J1, R5, R6 and D2.

When pins 1 and 3 of J2 are shorted using a jumper, power is
routed from +5V through R6 through the LED (D2) to ground.

When pins 2 and 3 of J2 are shorted together, the output
of PB7/SCK of the processor (U3) is routed through R5 
through the LED to ground.

When pins 3 and 4 of J2 are shorted together, there is
no circuit path through LED (D2).

When pins 1 and 2 of J2 are accidentally shorted together,
resistors R5 and R6 form a voltage divider that simply
wastes power.  At least it does not accidentally short
the power supply to ground.

### Revision B and C Issues

The following issues need to be worked on:

* The two mounting holes on the right are too close to the
  SIP9 resistor.

* Realistically, the processor should be run at 5V in order
  run the crystal at 16MHz.

