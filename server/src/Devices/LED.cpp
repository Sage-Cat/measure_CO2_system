#include "LED.hpp"

#include "SpdlogConfig.hpp"

LED::LED(int pin) : pin_(pin)
{
    SPDLOG_INFO("Created LED for GPIO pin: {}", pin_);

    chip_ = gpiod_chip_open_by_number(0);
    if (!chip_) {
        throw std::runtime_error("Failed to open GPIO chip");
    }

    line_ = gpiod_chip_get_line(chip_, pin_);
    if (!line_) {
        gpiod_chip_close(chip_);
        throw std::runtime_error("Failed to get GPIO line");
    }

    setup_pin();
}

LED::~LED()
{
    SPDLOG_TRACE("LED::~LED");
    gpiod_line_release(line_);
    gpiod_chip_close(chip_);
}

void LED::on()
{
    SPDLOG_INFO("LED is ON");
    gpiod_line_set_value(line_, 1);
}

void LED::off()
{
    SPDLOG_INFO("LED is OFF");
    gpiod_line_set_value(line_, 0);
}

void LED::setup_pin()
{
    SPDLOG_TRACE("LED::setup_pin");
    int ret = gpiod_line_request_output(line_, "LED", 0);
    if (ret < 0) {
        throw std::runtime_error("Failed to request line as output");
    }
}
