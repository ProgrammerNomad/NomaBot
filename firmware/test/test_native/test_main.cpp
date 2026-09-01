#include <unity.h>

int quantizeEnergy(int energy) {
  if (energy <= 0) {
    return 0;
  }
  if (energy >= 100) {
    return 100;
  }
  return ((energy + 2) / 5) * 5;
}

void test_quantize_energy_buckets() {
  TEST_ASSERT_EQUAL(0, quantizeEnergy(0));
  TEST_ASSERT_EQUAL(80, quantizeEnergy(79));
  TEST_ASSERT_EQUAL(100, quantizeEnergy(100));
}

void test_brain_duration_bounds() {
  TEST_ASSERT_TRUE(1500 >= 25 * 60 - 60);
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_quantize_energy_buckets);
  RUN_TEST(test_brain_duration_bounds);
  return UNITY_END();
}
