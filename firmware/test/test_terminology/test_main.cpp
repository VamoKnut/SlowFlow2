#include <Arduino.h>
#include <unity.h>

#include <slowflow/Terminology.h>

namespace {

void board_type_uses_agreed_name() {
  TEST_ASSERT_EQUAL_STRING(
      "ArduinoPortentaH7",
      slowflow::terminology::BoardType);
}

void shield_type_uses_agreed_name() {
  TEST_ASSERT_EQUAL_STRING(
      "SlowFlowShieldRev1",
      slowflow::terminology::ShieldType);
}

}  // namespace

void setup() {
  UNITY_BEGIN();
  RUN_TEST(board_type_uses_agreed_name);
  RUN_TEST(shield_type_uses_agreed_name);
  UNITY_END();
}

void loop() {}
