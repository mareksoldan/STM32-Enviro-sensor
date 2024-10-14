# LAN/SNMP Environmental Sensor

This project is a LAN/SNMP environmental sensor using STM32F103C8T6, W5500 Ethernet Module, and AM2301 sensors for monitoring temperature and humidity.

## Components

- **STM32F103C8T6** - Microcontroller
- **W5500 Module TCP/IP Ethernet Module** - Ethernet communication
- **AM2301 (DHT21)** - Temperature and humidity sensors
- **D4012SA 3.3V** - Power regulator
- **ST-LINK V2** - Programmer

## Wiring

### STM32 to W5500 Ethernet Module
- `PA4` <-> `CS`
- `PA5` <-> `SCK`
- `PA6` <-> `MISO`
- `PA7` <-> `MOSI`

### STM32 to AM2301 (DHT21)
- `PA1` <-> Sensor 1 (temperature and humidity)
- `PA2` <-> Sensor 2 (temperature and humidity)

### Power Supply
The W5500 Ethernet Module requires an external power supply.

## Features

- Monitor temperature and humidity using two DHT21 sensors.
- SNMP communication over LAN using W5500 Ethernet module.
- Web-based configuration interface for device settings (name, location, description, contact).
- LED status indicator for network connection status.
- EEPROM-based storage for user settings (device name, location, etc.).

## Set up the Environment

### Prerequisites
Ensure you have the following tools and libraries installed:

- STM32 Core for Arduino IDE
- FlashStorage_STM32
- Ethernet library
- SNMP library
- DHT library

### Cloning the Repository

To clone this repository, run the following command:

```bash
git clone <repository_url>
```

### Programming the STM32

1. Connect the STM32 board to your computer using the ST-LINK V2 programmer.
2. Open the `.ino` project in Arduino IDE.
3. Select the appropriate board (`STM32F103C8T6`) and upload the code.

### Configuring the Device

After flashing the code, configure the device using a web browser:

1. Connect the device to your local network.
2. Open the device's IP address in a browser.
3. Configure the device name, location, description, and contact details via the web interface.

## SNMP Configuration

The device supports SNMP (Simple Network Management Protocol) and responds to the following OIDs:

- `1.3.6.1.2.1.1.5.0` - System Name
- `1.3.6.1.2.1.1.1.0` - System Description
- `1.3.6.1.2.1.1.6.0` - System Location
- `1.3.6.1.2.1.1.4.0` - System Contact
- `1.3.6.1.2.1.1.3.0` - System Uptime
- `1.3.6.1.4.1.148.1.1.1` - Sensor 1 Temperature
- `1.3.6.1.4.1.148.1.1.2` - Sensor 1 Humidity
- `1.3.6.1.4.1.148.1.2.1` - Sensor 2 Temperature
- `1.3.6.1.4.1.148.1.2.2` - Sensor 2 Humidity

### Default SNMP Settings

- **Community**: `read`
- **Version**: `v1` or `v2c`

## Status LED

The status LED on pin `PC13` indicates the network connection status:

- **DHCP Waiting**: LED blinks every 500ms.
- **Running**: LED blinks every 3 seconds.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
