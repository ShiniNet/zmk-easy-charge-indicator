# ZMK Easy Charge Indicator

Minimal GPIO-based charge indicator driver for ZMK.

## Purpose
This module was created to read Xiao BLE `CHG_GPIO` (`&gpio0 17`) and light an external LED connected outside the MCU.
It provides a simple "charging only" indicator: the LED turns on while charging and turns off otherwise.

## Usage
1) Add the module to your ZMK build (west manifest or ZMK_EXTRA_MODULES):

```yaml west.yml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: ShiniNet                           # <--- add this
      url-base: https://github.com/ShiniNet    # <--- add this
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
    - name: zmk-easy-charge-indicator         # <--- add this
      remote: ShiniNet                        # <--- add this
      revision: main                          # <--- add this
  self:
    path: config
```

2) Enable the driver to your keyboard conf file:

```ini
   CONFIG_ZMK_EASY_CHARGE_INDICATOR=y
```

3) Add a devicetree node in your shield/board overlay:

```dts
   easy_charge_indicator: easy_charge_indicator {
       compatible = "zmk,easy-charge-indicator";
       charge-gpios = <&gpio0 17 GPIO_ACTIVE_LOW>;
       led-gpios = <&gpio0 15 (GPIO_ACTIVE_LOW | GPIO_OPEN_DRAIN)>;
   };
```

Notes:
- CHG is expected to be open-drain, active-low (low = charging).
- LED GPIO is driven open-drain, active-low (low = LED on).
- If you need pull-ups, add GPIO_PULL_UP in the GPIO flags.
- Add appropriate current limiting for the external LED (e.g., a series resistor) and verify LED polarity.
- The indicator only reflects charging state; it does not show charge-complete or battery level.
