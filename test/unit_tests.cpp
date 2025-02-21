#include <Arduino.h>
#include <unity.h>
#include "motor_control.h"

void test_motor_initialization() {
    MotorController motor(25, 26, 27);
    TEST_ASSERT_EQUAL(LOW, digitalRead(26));
    TEST_ASSERT_EQUAL(LOW, digitalRead(27));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_motor_initialization);
    UNITY_END();
}

void loop() {}