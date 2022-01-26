# Nucleo-F091RC Development Board Integration Tests

These test involve both C-code firmware and the python host interface

You have to run each test suite (python class) seperately after uploading a specific firmware.

See the class docstring for how to upload firmware.

As an example, make the firmware and install the python package in a virtualenv.

```bash
openocd -f /usr/share/openocd/scripts/board/st_nucleo_f0.cfg -c "program build/cortex-m0_gcc_debug/stm32f091nucleo64_write_pattern_to_serial.elf verify reset
cd nucleo-f091rc_integration_tests//
python -m unittest test_rx.TestRxCounterFromDevice
```
