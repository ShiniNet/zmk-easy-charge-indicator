# ZMK Easy Charge Indicator

Minimal GPIO-based charge indicator driver for ZMK.

## Usage
1) Add the module to your ZMK build (west manifest or ZMK_EXTRA_MODULES).
2) Enable the driver:
   CONFIG_ZMK_EASY_CHARGE_INDICATOR=y
3) Add a devicetree node in your shield/board overlay:

   easy_charge_indicator: easy_charge_indicator {
       compatible = "zmk,easy-charge-indicator";
       charge-gpios = <&gpio0 29 GPIO_ACTIVE_LOW>;
       led-gpios = <&gpio0 15 (GPIO_ACTIVE_LOW | GPIO_OPEN_DRAIN)>;
   };

Notes:
- CHG is expected to be open-drain, active-low (low = charging).
- LED GPIO is driven open-drain, active-low (low = LED on).
- If you need pull-ups, add GPIO_PULL_UP in the GPIO flags.
