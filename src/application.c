// Tower Kit documentation https://tower.hardwario.com/
// SDK API description https://sdk.hardwario.com/
// Forum https://forum.hardwario.com/

#include <application.h>

// LED instance
twr_led_t led;

// Button instance
twr_button_t button;

// Thermometer instance
twr_tmp112_t tmp112;
uint16_t button_click_count = 0;

void twr_set_led(uint64_t *id, const char *topic, void *value, void *param){

	int led_value = *((int *)value);

	switch (led_value){
		case 0: 
            set_led_color(255, 0, 0);
			// twr_led_set_mode(&led, TWR_LED_MODE_BLINK_FAST);
			break;
		case 1: 
            set_led_color(0, 0, 255);
            // twr_led_set_mode(&led, TWR_LED_MODE_BLINK_SLOW);
			break;
        case 2:
            set_led_color(0, 255, 0);
			// twr_led_set_mode(&led, TWR_LED_MODE_OFF);
            break;
        case 3:
            set_led_color(0, 0, 0);
            break;
		default: 
			break;
	}
	twr_log_info("State: %d", led_value);
}

static const twr_radio_sub_t subs[] = {
    {"led/-/state/set", TWR_RADIO_SUB_PT_INT, twr_set_led, NULL}
};

// Button event callback
void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    const char *subtopic = "node/demo:0/button/-/state";
    // Log button event
    twr_log_info("APP: Button event: %i", event);
    
        int call = 1;
        int expend = 2;

    // Check event source
    if (event == TWR_BUTTON_EVENT_CLICK)
    {   
        // Toggle LED pin state
        // twr_led_set_mode(&led, TWR_LED_MODE_TOGGLE);

         // Publish message on radio
        // button_click_count++;
        // twr_radio_pub_push_button(&button_click_count);
        twr_radio_pub_int(subtopic, &call);
    } else if(event == TWR_BUTTON_EVENT_HOLD) {
        // set_led_color(0, 0, 0);
        // twr_led_set_mode(&led, TWR_LED_MODE_OFF);
        twr_radio_pub_int(subtopic, &expend);
    }
}

void set_led_color(uint8_t red, uint8_t green, uint8_t blue){
    twr_pwm_set(TWR_PWM_P6, blue);
    twr_pwm_set(TWR_PWM_P7, green);
    twr_pwm_set(TWR_PWM_P8, red);
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
	twr_radio_set_subs((twr_radio_sub_t*)subs, sizeof(subs) / sizeof(twr_radio_sub_t));
	twr_radio_pairing_request("demo", FW_VERSION);

    // blue
    twr_pwm_init(TWR_PWM_P6);
    twr_pwm_enable(TWR_PWM_P6);
    // red
    twr_pwm_init(TWR_PWM_P7);
    twr_pwm_enable(TWR_PWM_P7);
    // green
    twr_pwm_init(TWR_PWM_P8);
    twr_pwm_enable(TWR_PWM_P8);
}

// Application task function (optional) which is called peridically if scheduled
// void application_task(void)
// {
//     static int counter = 0;

//     // Log task run and increment counter
//     twr_log_debug("APP: Task run (count: %d)", ++counter);

//     // Plan next run of this task in 1000 ms
//     twr_scheduler_plan_current_from_now(1000);
// }
