#define DT_DRV_COMPAT zmk_easy_charge_indicator

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(zmk_easy_charge_indicator, LOG_LEVEL_DBG /*ONFIG_ZMK_EASY_CHARGE_INDICATOR_LOG_LEVEL*/);

struct zmk_easy_charge_indicator_config {
    struct gpio_dt_spec charge;
    struct gpio_dt_spec led;
};

struct zmk_easy_charge_indicator_data {
    struct gpio_callback charge_cb;
    struct k_work work;
    const struct device *dev;
};

static int led_set(const struct gpio_dt_spec *led, bool on)
{
    int value = on ? 1 : 0;

    if ((led->dt_flags & GPIO_ACTIVE_LOW) != 0U) {
        value = !value;
    }

    return gpio_pin_set(led->port, led->pin, value);
}

static int update_led(const struct device *dev)
{
    const struct zmk_easy_charge_indicator_config *cfg = dev->config;
    int raw = gpio_pin_get(cfg->charge.port, cfg->charge.pin);
    bool charging;

    if (raw < 0) {
        LOG_ERR("charge read failed: %d", raw);
        return raw;
    }

    charging = (raw != 0);
    if ((cfg->charge.dt_flags & GPIO_ACTIVE_LOW) != 0U) {
        charging = !charging;
    }

    LOG_INF("charge=%d -> charging=%d", raw, charging);
    return led_set(&cfg->led, charging);
}

static void work_handler(struct k_work *work)
{
    struct zmk_easy_charge_indicator_data *data =
        CONTAINER_OF(work, struct zmk_easy_charge_indicator_data, work);

    LOG_DBG("work: update led");
    (void)update_led(data->dev);
}

static void charge_isr(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
    struct zmk_easy_charge_indicator_data *data =
        CONTAINER_OF(cb, struct zmk_easy_charge_indicator_data, charge_cb);

    ARG_UNUSED(port);
    ARG_UNUSED(pins);

    LOG_DBG("irq: charge change");
    k_work_submit(&data->work);
}

static int zmk_easy_charge_indicator_init(const struct device *dev)
{
    const struct zmk_easy_charge_indicator_config *cfg = dev->config;
    struct zmk_easy_charge_indicator_data *data = dev->data;
    int ret;

    LOG_INF("init: start");
    if (!device_is_ready(cfg->charge.port) || !device_is_ready(cfg->led.port)) {
        LOG_ERR("init: gpio not ready");
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&cfg->charge, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("init: charge gpio config failed: %d", ret);
        return ret;
    }

    ret = gpio_pin_configure_dt(&cfg->led, GPIO_OUTPUT);
    if (ret != 0) {
        LOG_ERR("init: led gpio config failed: %d", ret);
        return ret;
    }

    data->dev = dev;
    k_work_init(&data->work, work_handler);

    gpio_init_callback(&data->charge_cb, charge_isr, BIT(cfg->charge.pin));
    ret = gpio_add_callback(cfg->charge.port, &data->charge_cb);
    if (ret != 0) {
        LOG_ERR("init: gpio_add_callback failed: %d", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&cfg->charge, GPIO_INT_EDGE_BOTH);
    if (ret != 0) {
        LOG_ERR("init: gpio irq config failed: %d", ret);
        return ret;
    }

    ret = update_led(dev);
    if (ret != 0) {
        LOG_ERR("init: initial update failed: %d", ret);
        return ret;
    }

    LOG_INF("init: done");
    return 0;
}

#define ZMK_EASY_CHARGE_INDICATOR_INST(inst)                                        \
    static const struct zmk_easy_charge_indicator_config                           \
        zmk_easy_charge_indicator_config_##inst = {                                \
            .charge = GPIO_DT_SPEC_INST_GET(inst, charge_gpios),                    \
            .led = GPIO_DT_SPEC_INST_GET(inst, led_gpios),                          \
    };                                                                               \
    static struct zmk_easy_charge_indicator_data zmk_easy_charge_indicator_data_##inst; \
    DEVICE_DT_INST_DEFINE(inst,                                                     \
                          zmk_easy_charge_indicator_init,                           \
                          NULL,                                                     \
                          &zmk_easy_charge_indicator_data_##inst,                   \
                          &zmk_easy_charge_indicator_config_##inst,                 \
                          POST_KERNEL,                                              \
                          CONFIG_ZMK_EASY_CHARGE_INDICATOR_INIT_PRIORITY,           \
                          NULL);

DT_INST_FOREACH_STATUS_OKAY(ZMK_EASY_CHARGE_INDICATOR_INST)
