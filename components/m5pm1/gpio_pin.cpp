#include "gpio_pin.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5pm1 {

static const char *const TAG = "m5pm1.gpio_pin";

void M5PM1GPIOPin::setup() { this->pin_mode(this->flags_); }

void M5PM1GPIOPin::pin_mode(gpio::Flags flags) {
  this->flags_ = flags;
  // Route the pin to the plain GPIO function (not IRQ/WAKE/special).
  this->parent_->gpio_set_func(this->pin_, GPIO_FUNC_GPIO);

  if (flags & gpio::FLAG_OUTPUT) {
    this->parent_->gpio_set_drive(this->pin_,
                                  (flags & gpio::FLAG_OPEN_DRAIN) ? GPIO_DRIVE_OPENDRAIN : GPIO_DRIVE_PUSHPULL);
    this->parent_->gpio_set_mode(this->pin_, true);
  } else {
    if (flags & gpio::FLAG_PULLUP) {
      this->parent_->gpio_set_pull(this->pin_, GPIO_PULL_UP);
    } else if (flags & gpio::FLAG_PULLDOWN) {
      this->parent_->gpio_set_pull(this->pin_, GPIO_PULL_DOWN);
    } else {
      this->parent_->gpio_set_pull(this->pin_, GPIO_PULL_NONE);
    }
    this->parent_->gpio_set_mode(this->pin_, false);
  }
}

bool M5PM1GPIOPin::digital_read() {
  bool level = false;
  this->parent_->gpio_get_input(this->pin_, &level);
  return level != this->inverted_;
}

void M5PM1GPIOPin::digital_write(bool value) {
  this->parent_->gpio_set_output(this->pin_, value != this->inverted_);
}

std::string M5PM1GPIOPin::dump_summary() const {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%u via M5PM1", this->pin_);
  return buffer;
}

}  // namespace m5pm1
}  // namespace esphome
