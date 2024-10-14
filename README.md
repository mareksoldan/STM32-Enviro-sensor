# STM32 Enviro Sensor (LAN/SNMP)

This project implements a DIY environmental sensor for measuring temperature and humidity, based on the STM32 microcontroller with LAN connectivity and SNMP support. It allows you to monitor environmental conditions remotely via web or SNMP.

## Features

- **LAN Connectivity**: Uses the W5500 Ethernet module (currently only DHCP).
- **Supports Two Sensors**: Can connect two DHT21 (AM2301) temperature and humidity sensors.
- **SNMP Support**: Provides environmental data via SNMP, including standard SNMP values.
- **SNMP Values Implemented**:
  - Uptime, Device Name, Location, Description, Contact
  - Temperature 1, Humidity 1, Temperature 2, Humidity 2
- **Web Server**: Displays sensor data on a basic web interface.
- **EEPROM Storage**: Saves device name, location, description, and contact information in EEPROM.

## Components

- **Microcontroller**: STM32F103C8T6 (Blue Pill)
- **Ethernet Module**: W5500 Module TCP/IP Ethernet
- **Sensors**: AM2301 (DHT21) for temperature and humidity
- **Power**: D4012SA 3.3V for powering components
- **Programmer**: ST-Link V2 for flashing firmware

## Wiring Diagram

### Ethernet (W5500)
- PA4 <-> CS
- PA5 <-> SCK
- PA6 <-> MISO
- PA7 <-> MOSI

### Sensors (AM2301/DHT21)
- Sensor 1 <-> PA1
- Sensor 2 <-> PA2

## How to Use

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/STM32-Enviro-sensor.git

Set up the Environment:

Install the Arduino IDE or PlatformIO.
Install the necessary libraries:
Ethernet
SNMP
DHT
FlashStorage_STM32
Configure the Sensor:

The system will use DHCP for LAN configuration.
Modify the SNMP OID values in the code to match your specific requirements if needed.
Upload the Code:

Connect the STM32 board to your computer via the ST-Link V2 programmer.
Upload the .ino file using the Arduino IDE or PlatformIO.
Access the Web Interface:

Once the board is connected to your network, you can access the web interface using the assigned IP address. The web interface will display the sensor data and allow for device information updates.
SNMP Configuration:

Use an SNMP tool (such as Net-SNMP) to retrieve sensor data via the provided SNMP OIDs.
Default SNMP community: read
Default SNMP OIDs
1.3.6.1.2.1.1.2.0: sysObjectID
1.3.6.1.2.1.1.5.0: sysName (Device Name)
1.3.6.1.2.1.1.6.0: sysLocation
1.3.6.1.2.1.1.1.0: sysDescription
1.3.6.1.2.1.1.4.0: sysContact
1.3.6.1.2.1.1.3.0: sysUptime
1.3.6.1.4.1.148.1.1.1: Sensor 1 Temperature
1.3.6.1.4.1.148.1.1.2: Sensor 1 Humidity
1.3.6.1.4.1.148.1.2.1: Sensor 2 Temperature
1.3.6.1.4.1.148.1.2.2: Sensor 2 Humidity
Web Interface
The web interface shows the current temperature and humidity values from both sensors and allows updating device details such as name, location, description, and contact.
