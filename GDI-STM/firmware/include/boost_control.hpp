#ifndef BOOST_CONTROL_HPP
#define BOOST_CONTROL_HPP

#include <cstdint>

// Software PWM on PD0 (BOOST_PWM) using TIM7 for the IRS21867S gate driver.
namespace boost {

void init();
void setDutyPercent(uint8_t percent);
void enable();
void disable();

// Call from the main task loop. Forces disable if system is unhealthy.
void update(bool systemHealthy);

} // namespace boost

#endif // BOOST_CONTROL_HPP
