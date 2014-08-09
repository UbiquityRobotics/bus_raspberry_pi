# bus_raspberry_pi

Raspberry Pi that can be connected to a bus.

The bus is a half-duplex multi-drop bus that runs at 500 Kbps.
It uses a Microchip MCP2562 CAN bus transceivers to connect to
the bus.  To be rigorous, the CAN bus transceiver is techincally
an ISO-11898 transceiver.

## Revision A

The schematic can be found in the `rev_a` folder as
`bus_raspberry_pi.pdf`.

### Power Supply

The power comes in on either N4 or N5.  These two connectors
are "daisy-chained" so that power can be routed to other boards.
The power goes through Q1 -- a P-channel MOSFET with a very low
Rds.  Q1 is used to prevent damage just in case the power
comes in with polarity reversed.  The power continues to N6
which is a DC-to-DC power converter up to 5V@1.5A output.
The 5V converter takes up 24V input with an absolute maximum
of 36V.

### Bus Connection

The bus connection is N1, which is a 2x5 shrouded header.
LGND1 and LGND2 are connected to ground.  CANH and CANL
are routed over to U2 which is the CAN bus (strictly
speaking ISO-11898) transceiver -- a Microchip MCP2562.
The TXD and RXD outputs are routed to the RXD1 and TXD1
pins on U4, the ATmega324P.

### ISP Connection

The ISP connector is the 2x3 N7 the is connected to the
MOSI, MISO, #RESET, and SCK pins on U4.

### JTAG Connection

The JTAG connector is N8 which is also a 2x5 shrouded header.
The TDI, TDO, DMS, TCK, and #RESET lines are connected to the
appropriate JTAG pins on U4 (the ATmega324P.)

### 3.3 Volt Issues

The processor (U4), open collector inverters (U1), AND gates (U3),
and CAN bus transceiver all run at 5V.  The USB to serial connectors
(N2 and N3) and the Raspberry Pi connector (N9) runs at 3.3V signal
levels.  Thus, all through out the schematic you will see 22K/33K
voltage dividers that will take 5V in at the top and produce 3.0V
in the middle.  U1 and U3 are both HCT parts which will treat any
voltage above 2.0V as a logic "high."  Thus feeding in 3.0V will
be treated as a logic "high".  They produce 5.0V outputs that ca
be divided down to 3.0 volts which will be safe to feed to the 3.3V
connectors (N2, N3, and N9.)

### Reset Circuitry

The reset signal can be sourced from four locations:

* The manual push button -- SW1,

* #NRST on the JTAG connector (N8),

* The GPIO18 pin on N9, and

* The #RTS signal (3.3V) on the programming FTDI connector (N2),

The push button is the simplest.  It connects the #RESET line
to ground when pushed.  Done.

Similarly, the #NRST line on the JTAG connector (N9) can be
driven low by a JTAG programmer/debugger.  Done.

The GPIO18 pin is pulled down to ground through the 100K
resistor R15.  This causes the output of U1C to go "high".
U1C is an open collector output so this leaves the #RESET
line pulled up to "high" through the 10K resistor R10.
The Raspberry Pi can assert #RESET by turning GPIO18 into
an output pin *and* driving it high.  The 3.3V high signal
is processed by U1C and inverted to drive #RESET low.
To de-assert #RESET, GPIO18 can be driven low again.

The reset path from #RTS on the programmer connector (N2)
adheres to the Arduino defacto standard for reset.  Inverters
U1A and U1B are used level shift #RTS from
3.3V to 5V.  R4 and R5 are pull up resistors for the open
collector outputs of U1A and U1B.

When the Arduino programmer (usually AVRDUDE) opens the connection
to the serial cable, #RTS (Request to Send) is driven low
for the duration of the programming session.  Capacitor C3, R10,
and D1 are used to convert the falling edge of #RTS into a
pulse onto the #RESET line.  Prior to asserting #RTS low,
R9 and R10 pull both ends of C3 to 5V, thereby completely
discharging C3.  When #RTS asserts low at the start of
programming, inverter U1B output is driven low.  This pulls
both ends of C3 0V and assert the  #RESET line low.  Now
C3 starts getting charged via R10 and start to develop 5V
across C3.  Eventually the voltage across C3 will get high
enough to de-assert #RESET and the processor (U4)
will start to run the boot-loader.  When the programming
session is over, #RTS is driven high.  This will cause
C3 to start discharging through R9 and R10.  Alas, the
connection to #RESET will attempt to go from 5V to 10V
when #RTS is driven high.  This is prevented by Schottky
diode D1 which will clip off any voltage above 5.2V.
Thus, C3 discharges extremely rapidly.  The 5.2V voltage
cap on the #RESET line will prevent the voltage from exceeding
the absolute maximum input voltage for the #RESET line of
the microprocessor (U4) of 13V and the maximum output voltage
of U1C of 5.5V.  If U1C were not connected to the #RESET line,
a regular silicon diode could be used instead of the Schottky
diode (D1).

Talk about R5/R6

### RXD and TXD Considerations

In general, the primary purpose of this board is to connect
TXD0 and RXD1 to GPIO15_RXD and GPIO14_TXD respectively.
In addition, it is desirable to allow the programming
connector (N2) to be used to program the microprocessor (U4)
*and* to use the console connector (N3) to view the console
traffic from the Raspberry Pi on boot up.  To accomplish this
task some additional circuitry is used to OR the serial
cable TXD signals into the correct locations.

A key thing to understand about serial connections is that
the line idles "high".  U3A and U3B are AND gates.  Using
De Morgan's law, A.B = ~(~A + ~B).  Thus, using 2-input AND
gates, the signals a correctly OR'ed together so that when
both inputs are idling "high", the output is also "high".

There are three paths to follow:

* Processor (U4) to Raspberry Pi (N9),

* Processor (U4) to programmer connector (N2), and

* Processor (U4) to console connector (N3).

#### Processor to Raspberry Pi

In this situation, the 5V TXD0 output  on the processor (U4)
is routed through the R11/R12 voltage divider to bring the
voltage down to 3.0V.  This signal is routed to U3B.  U3B
converts the 3.0V input to a 5V output which is again divided
back down to 3.0V via R13/R14.  This 3.0V signal goes into
GPIO15_RXD on the Raspberry Pi.

In the reverse direction, the 3.3V output of GPIO15_TXD is
routed through to U3A with converts it to a 5V output and
directs it to RXD0.

If for any reason, the Raspberry Pi is not connected to
the board, the R16/R17 voltage divider will produce a 3.0V
signal that idle "high" into U3A.  When GPIO14_TXD is
driving the line it will overpower the voltage divider
when is sends 3.3V "high" of 0V "low" signals.

#### Processor to Programmer Connector

In this situation, the 5V TXD0 output on the processor (U4)
is voltage divided by R11/R12 down to 3.0V and goes to RXD
on the programmer connector (N2).

The 3.3V TXD output on the programmer connector (N2) is
routed to U3A which converts it up to 5V and routes the
signal into RXD0 input on the processor (U4).

As expected, the R7/R8 voltage divider holds the input
to U3A at 3.0V if there is no USB to serial connector
present on N2.  If there is a connection, the 3.3V or
0V output from TXD will override the voltage divider
resistors R7/R8.

#### Processor to Console Connector

In this situation, the 5V TXD0 output on processor (U4)
is voltage divided R11/R12 down to 3.0V and and goes U3B.
U3B level shift the 3.0V up to 5V and then voltage divides
back down to 3.0V using the R13/14 voltage divider.
The 3.0V signal if fed back into GPIO15_RXD on the
Raspberry Pi connector (N9).

The 3.3V signal from GPIO14_TXD is goes from the Raspberry
Pi connector (N9) and heads directly over to RXD on the
console connector (N3).  No further level shifting is needed.

If for some reason, the Raspberry Pi is not plugged into N9,
the R16/R17 voltage divider will hold GPIO14_TXD at 3.0V
thereby idling the line "high".  When the Raspberry Pi is
connected, the 3.3V signal from the Raspberry Pi on GPIO14_TXD
will override the voltage divider R16/R17.

The same think occurs on the 3.3V TXD signal from N3.
If there is no USB to serial cable plugged into N3, the
R1/R2 voltage divider will hold the line at 3.0V and
there feed an idle signal into U3B.  When the cable is
plugged into N3, the 3.3V signal will override the R1/R2
voltage divider.

### Closing Comments on Revision A

The purpose of N2, N3, U1, U3, R1/R2, R5/R6, R7/R8,
R16/R17, and R15 is to provide a flexible platform for
developing firmware on the processor (U4).  In future
revisions, some of this circuitry will become rather
redundant and can be removed.

