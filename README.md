# esphome-m5pm1

![Build](https://github.com/GitNik1/esphome-m5pm1/actions/workflows/build.yml/badge.svg)

ESPHome external component for the **M5Stack PM1** power-management IC (PMIC).

The PM1 is an I²C-controlled multifunction PMIC (default address `0x6E`) that
provides battery charging & monitoring, several power rails (5 V DCDC, 3.3 V LDO,
5 V boost / 5VINOUT), 5 multifunction GPIOs, 2 PWM channels, 2 ADC inputs + an
internal temperature sensor, a NeoPixel controller (up to 32 LEDs), a watchdog,
a wake/power timer, button handling and power-hold logic.

This component is a native ESPHome port of the vendor driver
([m5stack/M5PM1](https://github.com/m5stack/M5PM1)) — it talks to the chip over
ESPHome's own `i2c` bus, no Arduino library needed.

## Installation

```yaml
external_components:
  - source: github://GitNik1/esphome-m5pm1@main
    components: [m5pm1]
```

**All platforms are optional.** Each one lives in its own sub-package
(`components/m5pm1/<platform>/`), so ESPHome only copies and compiles the C++ for
the platforms you actually use — a config with just the `m5pm1:` hub does not pull
in `light`, `select`, `number`, … . This is verified in CI by
`test/test-esp32-minimal.yaml`.

```
components/m5pm1/
├── __init__.py, m5pm1.{h,cpp}   # hub (needs only i2c) + GPIO expander pin + actions
├── sensor/  binary_sensor/  text_sensor/
├── switch/  output/  number/  button/  select/  light/
```

## Hub

```yaml
i2c:
  sda: 21
  scl: 22

m5pm1:
  id: pmic
  address: 0x6E            # default
  # optional boot-time defaults (all optional):
  charge: true             # enable battery charging
  dcdc: true               # 5 V DCDC rail on
  ldo: true                # 3.3 V LDO rail on
  boost: false             # 5 V boost / 5VINOUT rail
  battery_low_voltage_protection: 3.0V   # 2.0V .. 4.0V
  i2c_sleep_timeout: 0     # 0 = never sleep, 1..15 s idle timeout
```

## Platforms

### `sensor`

| `type`               | Unit | Notes                                   |
|----------------------|------|-----------------------------------------|
| `battery_voltage`    | V    | battery voltage                         |
| `vin_voltage`        | V    | VIN (USB/DC) voltage                    |
| `vinout_voltage`     | V    | 5VINOUT bidirectional port voltage      |
| `reference_voltage`  | V    | internal reference                      |
| `temperature`        | °C   | internal chip temperature (1 °C resolution) |
| `adc1`               | —    | ADC on GPIO1 (12-bit raw, routes GPIO1) |
| `adc2`               | —    | ADC on GPIO2 (12-bit raw, routes GPIO2) |
| `watchdog_count`     | s    | current watchdog countdown              |
| `pwm_frequency`      | Hz   | configured PWM frequency                |

```yaml
sensor:
  - platform: m5pm1
    type: battery_voltage
    name: "Battery"
  - platform: m5pm1
    type: temperature
    name: "PMIC Temp"
```

### `binary_sensor`

`type: button` (power-button state) or `type: external_power` (true when powered
from 5VIN or 5VINOUT rather than battery).

### `text_sensor`

`type: power_source` (`5VIN` / `5VINOUT` / `Battery` / `Unknown`) or
`type: wake_source` (comma-separated list of the last wake reasons).

### `switch`

`type:` one of `charge`, `dcdc`, `ldo`, `boost`, `led_en_level`, `ldo_hold`,
`boost_hold`, or `gpio_hold` (requires `pin: 0..4`). Holds keep a rail/GPIO
powered across a power-off request.

### `number`

`type:` `battery_lvp` (2000–4000 mV), `watchdog_timeout` (0–255 s) or
`pwm_frequency` (1–65535 Hz).

### `output` (PWM)

```yaml
output:
  - platform: m5pm1
    id: pm1_pwm0
    channel: 0          # 0 = GPIO3 (PWM0), 1 = GPIO4 (PWM1)
    frequency: 2000Hz
```

### `button`

`type:` `shutdown`, `reboot`, `feed_watchdog`, `refresh_leds`,
`enter_download_mode`, `timer_clear`.

### `select`

I²C-speed select (`100kHz` / `400kHz`) for the PMIC-side config bit.

```yaml
select:
  - platform: m5pm1
    name: "PM1 I2C speed"
```

### `light` (NeoPixel)

Addressable RGB light driven by the PM1's built-in NeoPixel controller
(max 32 LEDs):

```yaml
light:
  - platform: m5pm1
    name: "PM1 LEDs"
    m5pm1_id: pmic
    num_leds: 8
```

### GPIO expander (`pin`)

GPIO0–GPIO4 can be used like any ESPHome pin:

```yaml
switch:
  - platform: gpio
    name: "Relay"
    pin:
      m5pm1: pmic
      number: 0
      mode:
        output: true
```

## Actions

```yaml
on_...:
  - m5pm1.shutdown: pmic
  - m5pm1.reboot: pmic
  - m5pm1.enter_download_mode: pmic
  - m5pm1.feed_watchdog: pmic
  - m5pm1.set_timer:
      id: pmic
      seconds: 3600
      action: power_on      # stop / flag / reboot / power_on / power_off
  - m5pm1.set_battery_lvp:
      id: pmic
      millivolts: 3200
  - m5pm1.aw8737a_pulse:     # AW8737A audio amp gain pulse
      id: pmic
      pin: 0
      pulses: 2             # 0..3
```

## Notes & limitations

- GPIO/PWM/ADC share the same physical pins. Routing a pin to a special function
  (ADC/PWM/NeoPixel) is done automatically by the relevant platform.
- Switching the PMIC I²C speed to 400 kHz only sets the chip-side bit; set the
  actual bus speed in the `i2c:` block (`frequency: 400kHz`).
- The internal temperature sensor reports **whole degrees Celsius**, despite the
  vendor header documenting 0.1 °C units. Confirmed on hardware against a
  reference sensor (PM1 reported 31 °C at 30.7 °C ambient).
- Hardware validation is **in progress**: battery voltage and chip temperature
  are verified on a real PM1; the remaining functions are CI-compile-tested only.
  Feedback from hardware is welcome.

## License

MIT. Portions derived from the M5Stack M5PM1 driver (MIT). See [LICENSE](LICENSE).
