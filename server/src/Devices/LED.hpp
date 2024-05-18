#ifndef LED_HPP
#define LED_HPP

#include <gpiod.h>

class LED {
public:
    explicit LED(int pin);
    ~LED();

    void on();
    void off();

private:
    void setup_pin();

private:
    int pin_;
    gpiod_chip *chip_;
    gpiod_line *line_;
};

#endif // LED_HPP
