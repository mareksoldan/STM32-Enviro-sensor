
# LAN/SNMP Environmental Sensor

This project is a LAN/SNMP environmental sensor based on the STM32F103C8T6 microcontroller. It utilizes the W5500 Ethernet module to communicate over a network and features temperature and humidity sensing capabilities with the AM2301 (DHT21) sensor.

## Table of Contents

- [Components](#components)
- [Wiring Diagram](#wiring-diagram)
- [Setup](#setup)
- [Usage](#usage)
- [Future Improvements](#future-improvements)
- [Contributing](#contributing)

## Components

- **Microcontroller**: STM32F103C8T6
- **Ethernet Module**: W5500 TCP/IP Ethernet Module
- **Temperature and Humidity Sensors**: AM2301 (DHT21)
- **Voltage Regulator**: D4012SA 3.3V
- **Programmer**: ST-Link V2

## Wiring Diagram

```plaintext
STM32F103C8T6 <-> W5500 Module
PA4 (CS) <-> CS
PA5 (SCK) <-> SCK
PA6 (MISO) <-> MISO
PA7 (MOSI) <-> MOSI

AM2301 (DHT21) <-> STM32F103C8T6
Sensor 1 <-> PA1
Sensor 2 <-> PA2
```

## Setup

1. Clone the repository:
    ```bash
    git clone <repository-url>
    cd <repository-directory>
    ```
2. Open the project in your preferred IDE.
3. Upload the code to your STM32 microcontroller using the ST-Link V2 programmer.

## Usage

Once the code is uploaded and the sensors are connected, the sensor will start reporting temperature and humidity over the network using SNMP. You can access the data by querying the SNMP agent.

## Future Improvements

- Static IP configuration support
- External status LED (for box indication)
- Issue with saving special characters - to be fixed
- MAC address display on the web server

## Contributing

Contributions are welcome! To contribute:

1. Fork the repository.
2. Create a new branch: `git checkout -b feature-branch-name`
3. Commit your changes: `git commit -m 'Add some feature'`
4. Push to the branch: `git push origin feature-branch-name`
5. Submit a pull request.
