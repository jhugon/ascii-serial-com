# Arduino Uno Integration Tests

These test involve both C-code firmware and the python host interface

You have to run each test suite (python class) seperately after uploading a specific firmware.

See the class docstring for how to upload firmware.

As an example, make the firmware and install the python package in a virtualenv.

```bash
avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -Uflash:w:build/avr5_gcc_debug/arduino_uno_write_pattern_to_serial
cd arduino_integration_tests/
python -m unittest test_rx.TestRxCounterFromDevice
```
