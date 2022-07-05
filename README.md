# jtagstudy
In my journey of embedded systems, I felt like I was lacking embedded software development experience, and so to gain some, I decided to implement some of the [JTAG IEE 1149.1](https://en.wikipedia.org/wiki/JTAG) functionalities as [bit-bang](https://en.wikipedia.org/wiki/Bit_banging) routines.

I choose JTAG as the protocol to bit-bang because it's relatively simple to understand, useful for a lot of things used across different architectures and it's an industry standard.

I used AVR MCU's because that's what I had at my disposal, no need to spend extra money on a fancy STM32.

# Overview

Here is a picture of my inital setup:

![](/img/initial_setup.jpg)

I used an [ATmega 32u4](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7766-8-bit-AVR-ATmega16U4-32U4_Datasheet.pdf) from an [Arduino Micro](https://store.arduino.cc/products/arduino-micro) board as the the main MCU to which I uploaded code via the [USBasp](https://www.fischl.de/usbasp/) ISP programmer and [avrdude](https://www.nongnu.org/avrdude/). I also implemented the USART interface (TX only) to be able to print to the serial monitor the result of the code.
To test the code, I initially connected the four GPIO bit-banging pins to the JTAG interface of the 32u4 itself, once that was working, I used a custom PCB designed in [KiCad](https://www.kicad.org/) to extend the JTAG chain with two more TAPs and test the code again. The custom PCB is a breakout for another ATmega 32u4 and an [AT90USB1286](http://ww1.microchip.com/downloads/en/DeviceDoc/doc7593.pdf) Extending the JTAG chain revealed some bugs which I managed to solve in a couple of days.

The PCB had some mistakes which were solved by bridging some connections and whatnot.

Here is a picture of my final setup:

![](/img/final_setup.jpg)

I learned a lot of things while doing this project, not only software related, but also hardware, because this was my first time using an [oscilloscope](https://eleshop.eu/owon-sds1104.html), which I used to debug the signals I was bit-banging.


# Functionalities

- **JTAG TAP chain length**

    Uses the **BYPASS** instruction to check how many Test Acces Points (TAP's) are connected to the JTAG interface.

- **IDCODE scan**

    Prints to serial in detail the **IDCODE** of all the TAPs found in the JTAG Chain.

- **JTAG discovery**

    Given a set of GPIO pins, the four JTAG signal (TDI,TDO,TMS,TCK) are applied in different combinations to find if there is a JTAG interface on four of those pins.

# Credits
This was project was inspired by the [JTAGulator](https://github.com/grandideastudio/jtagulator) by Joe Grand.