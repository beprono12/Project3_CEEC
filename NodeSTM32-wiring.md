***\[Sơ đồ nối dây NodeSTM32]***



***<0> Nút Bấm***

***PB0 - X***

*GND - GND*



***<1> TFT \[SP1]***

**PA2 - RST**

**PA3 - DC**

**PA4 - CS**

**PA5 - SCK**

**PA7 - MOSI**

VCC, GND = 3.3V, GND



***<2> NRF24 \[SPI2]***

**PB10 - SCK**

**PB12 - CSN**

**PB13 - CE**

**PB14 - MISO**

**PB15 - MOSI**

**PA1 - IRQ**

VCC, GND = 3.3V, GND



**<3> GPS \[USART1]**

**PA9 - TX**

**PA10 - RX**

VCC, GND = 3.3V, GND



**<4> BNO \[I2C1]**

**PB6 - SCL**

**PB7 - SDA**

**PB8 - INT**

**PB9 - RST**

**PS0, PS1 - GND (\*)**

VCC, GND = 3.3V, GND



<5> Buzzer

PB5 - I/O

GND, VCC - GND, 3.3V

