# Car Black Box System using PIC16F877A

## About the Project

The Car Black Box is an embedded systems project developed using the PIC16F877A microcontroller and Embedded C.

The system records vehicle-related events such as gear changes, collision events, and power ON events along with timestamps and speed information. It provides a dashboard, password-protected menu, event log management, and UART-based log downloading.

## Hardware and Technologies Used

- PIC16F877A Microcontroller
- Embedded C
- MPLAB X IDE
- MPLAB XC8 Compiler
- CLCD (Character LCD)
- Digital Keypad
- ADC (Analog-to-Digital Converter)
- DS1307 RTC
- External EEPROM
- UART Communication
- I2C Communication

## Features

- **Dashboard:** Displays real-time clock, vehicle event, and speed.
- **Event Logging:** Records power ON, gear changes, reverse gear, and collision events.
- **Speed Monitoring:** Reads analog input through ADC and displays speed.
- **RTC Integration:** Uses DS1307 to maintain time information.
- **External EEPROM:** Stores event logs and password information.
- **Password Protection:** Provides a password-protected menu.
- **Login Security:** Locks access for 60 seconds after three incorrect password attempts.
- **View Logs:** Displays previously recorded vehicle events.
- **Clear Logs:** Allows stored event logs to be cleared.
- **Download Logs:** Transfers event logs through UART.
- **Time Configuration:** Allows the RTC time to be adjusted.
- **Change Password:** Allows users to update their password.

## Communication Protocols

| Protocol | Application |
|---|---|
| I2C | Communication with DS1307 RTC and external EEPROM |
| UART | Downloading event logs |
| ADC | Reading analog speed input |

## Project File Structure

| File | Description |
|---|---|
| `main.c` | Main program and initialization |
| `car_black_box.c` | Core application logic |
| `car_black_box.h` | Application definitions |
| `adc.c / adc.h` | ADC configuration and reading |
| `clcd.c / clcd.h` | LCD operations |
| `dkp.c / dkp.h` | Digital keypad handling |
| `ds1307.c / ds1307.h` | RTC operations |
| `external_eeprom.c / external_eeprom.h` | EEPROM read/write operations |
| `i2c.c / i2c.h` | I2C communication |
| `uart.c / uart.h` | UART communication |
| `Makefile` | Build configuration |

## Working Principle

1. The microcontroller initializes the peripherals and application modules.
2. The dashboard displays time, event information, and speed.
3. Vehicle events are recorded with timestamp and speed information.
4. Event records are stored in external EEPROM.
5. Users can access the menu using password authentication.
6. Stored logs can be viewed, cleared, or downloaded through UART.

## Compilation and Execution

1. Open the project in MPLAB X IDE.
2. Select PIC16F877A as the target microcontroller.
3. Configure the MPLAB XC8 compiler.
4. Build the project.
5. Generate the HEX file and load it onto the microcontroller.

## Author

**Alisha Shelke**
