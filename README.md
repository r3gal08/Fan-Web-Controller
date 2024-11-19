# Fan Web Controller

A FreeRTOS program I created to run on an ESP8266 microcontroller device. The device replaced a broken fan's remote control function and allowed me to mimic its input with a microcontroller. The microcontroller was controlled by local network UDP packets, which would signal the various controls.

This project was an overall success until I accidentally spliced the fan's power inputs due to a bad solder job and fried the control board. I learned a lot about networking and programming with FreeRTOS though :)

## Configuration

### Serial Flasher Options

- Set the `Default serial port`.

### Example Configuration Options

- Set the `WiFi SSID` of the router (Access Point).
- Set the `WiFi Password` of the router (Access Point).
- Set the `IP version` of the example to be IPv4 or IPv6.
- Set the `Port` number that represents the remote port the example will create.

## Build and Flash

Build the project and flash it to the board, then run the monitor tool to view serial output:

```bash
make -j4 flash monitor
