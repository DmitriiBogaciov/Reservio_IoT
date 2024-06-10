#include <application.h>

// LED instance
twr_led_t led;

// Button instance
twr_button_t button;

// Thermometer instance
twr_tmp112_t tmp112;
uint16_t button_click_count = 0;

// Variables for LED blinking
bool led_blinking = false;
int blink_count = 0;
int max_blink_count = 10; 
int blink_type = 0;
twr_tick_t last_blink_time = 0;
const twr_tick_t blink_interval = 500; // 500 ms interval for blinking

void twr_set_led(uint64_t *id, const char *topic, void *value, void *param)
{
    int led_value = *((int *)value);
    twr_log_debug("Received value: %d", led_value);

    switch (led_value)
    {
    case 0:
        set_led_color(255, 0, 0);
        led_blinking = false;
        break;
    case 1:
        set_led_color(0, 0, 255);
        led_blinking = false;
        break;
    case 2:
        set_led_color(0, 255, 0);
        led_blinking = false;
        break;
    case 3:
        set_led_color(0, 0, 0);
        led_blinking = false;
        break;
    case 4:
        blink_count = 0;
        max_blink_count = 10; // Number of blinks
        blink_type = 1;
        led_blinking = true;
        last_blink_time = twr_tick_get();
        twr_scheduler_plan_now(0); // Start blinking
        break;
    case 5:
        blink_count = 0;
        max_blink_count = 10; // Number of blinks
        blink_type = 2;
        led_blinking = true;
        last_blink_time = twr_tick_get();
        twr_scheduler_plan_now(0); // Start blinking
        break;
    default:
        break;
    }
    twr_log_info("State: %d", led_value);
}

static const twr_radio_sub_t subs[] = {
    {"led/-/state/set", TWR_RADIO_SUB_PT_INT, twr_set_led, NULL}};

// Button event callback
void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    const char *subtopic = "node/demo:0/button/-/state";
    // Log button event
    twr_log_info("APP: Button event: %i", event);

    int call = 1;
    int extend = 2;

    // Check event source
    if (event == TWR_BUTTON_EVENT_CLICK)
    {
        // Publish message on radio
        twr_radio_pub_int(subtopic, &call);
    }
    else if (event == TWR_BUTTON_EVENT_HOLD)
    {
        twr_radio_pub_int(subtopic, &extend);
    }
}

void set_led_color(uint8_t red, uint8_t green, uint8_t blue)
{
    twr_pwm_set(TWR_PWM_P6, blue);
    twr_pwm_set(TWR_PWM_P7, green);
    twr_pwm_set(TWR_PWM_P8, red);
}

void led_blink_task(void *param)
{
    static bool led_on = false;
    twr_tick_t current_time = twr_tick_get();

    if (led_blinking && (current_time - last_blink_time) >= blink_interval)
    {

        if (led_on)
        {
            // Turn off the LED
            set_led_color(0, 0, 0);
        }
        else
        {
            if (blink_type == 1)
            {
                set_led_color(0, 0, 255); // Blue
            }
            else if (blink_type == 2)
            {
                set_led_color(255, 0, 0); // Red
            }
        }

        led_on = !led_on;
        blink_count++;
        last_blink_time = current_time;

        if (blink_count >= max_blink_count * 2) 
        {
            led_blinking = false;
            set_led_color(0, 0, 255); 
        }
    }

        twr_scheduler_plan_current_from_now(blink_interval); // Plan next execution

}

void tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param)
{
    if (event == TWR_TMP112_EVENT_UPDATE)
    {
        float celsius;
        // Read temperature
        twr_tmp112_get_temperature_celsius(self, &celsius);

        twr_log_debug("APP: temperature: %.2f °C", celsius);

        twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE, &celsius);
    }
}

// Application initialization function which is called once after boot
void application_init(void)
{
    // Initialize logging
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, 0);
    twr_led_pulse(&led, 2000);

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, 0);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize thermometer on core module
    twr_tmp112_init(&tmp112, TWR_I2C_I2C0, 0x49);
    twr_tmp112_set_event_handler(&tmp112, tmp112_event_handler, NULL);
    twr_tmp112_set_update_interval(&tmp112, 10000);

    twr_radio_init(TWR_RADIO_MODE_NODE_LISTENING);
    twr_radio_set_subs((twr_radio_sub_t *)subs, sizeof(subs) / sizeof(twr_radio_sub_t));
    twr_radio_pairing_request("demo", FW_VERSION);

    // Initialize PWM for LED control
    // blue
    twr_pwm_init(TWR_PWM_P6);
    twr_pwm_enable(TWR_PWM_P6);
    // red
    twr_pwm_init(TWR_PWM_P7);
    twr_pwm_enable(TWR_PWM_P7);
    // green
    twr_pwm_init(TWR_PWM_P8);
    twr_pwm_enable(TWR_PWM_P8);

    twr_scheduler_register(led_blink_task, NULL, blink_interval);
}

