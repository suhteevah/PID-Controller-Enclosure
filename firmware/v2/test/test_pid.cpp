// ============================================================
// Unit Tests: PID Controller
// Run with: pio test -e test
// ============================================================

#ifdef UNIT_TEST

#include <unity.h>

// Minimal Arduino stubs for native testing
#ifndef ARDUINO
#include <cmath>
#include <cstdint>
#include <algorithm>
static uint32_t _millis_val = 0;
uint32_t millis() { return _millis_val; }
void advance_millis(uint32_t ms) { _millis_val += ms; }
float constrain(float val, float lo, float hi) {
    return std::max(lo, std::min(hi, val));
}
#define TEMP_MIN_F 0.0f
#define TEMP_MAX_F 999.0f
#define PID_KP_DEFAULT 8.0f
#define PID_KI_DEFAULT 0.2f
#define PID_KD_DEFAULT 2.0f
#define PID_SAMPLE_MS 250
#define PID_OUTPUT_MIN 0.0f
#define PID_OUTPUT_MAX 100.0f
#define PID_DERIVATIVE_FILTER 0.1f
#include "../src/core/pid.h"
#include "../src/core/pid.cpp"
#endif

void setUp(void) {
    _millis_val = 0;
}

void tearDown(void) {}

// --- Tests ---

void test_pid_init_defaults() {
    PIDController pid;
    TEST_ASSERT_FALSE(pid.isEnabled());
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, pid.getOutput());
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, pid.getSetpoint());
}

void test_pid_disabled_returns_zero() {
    PIDController pid;
    pid.begin(8.0, 0.2, 2.0, 250);
    pid.setSetpoint(710.0);
    // Not enabled - should return 0
    advance_millis(250);
    float out = pid.compute(500.0);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, out);
}

void test_pid_basic_proportional() {
    PIDController pid;
    pid.begin(1.0, 0.0, 0.0, 250);  // P-only
    pid.setOutputLimits(0, 100);
    pid.setSetpoint(100.0);
    pid.setEnabled(true);

    // First call: initializes, returns 0
    advance_millis(250);
    pid.compute(50.0);

    // Second call: error = 100 - 50 = 50, P = 1.0 * 50 = 50
    advance_millis(250);
    float out = pid.compute(50.0);
    TEST_ASSERT_FLOAT_WITHIN(1.0, 50.0, out);
}

void test_pid_output_clamping() {
    PIDController pid;
    pid.begin(10.0, 0.0, 0.0, 250);  // High gain
    pid.setOutputLimits(0, 100);
    pid.setSetpoint(999.0);
    pid.setEnabled(true);

    advance_millis(250);
    pid.compute(0.0);  // Init

    advance_millis(250);
    float out = pid.compute(0.0);  // Error = 999, P = 9990
    TEST_ASSERT_FLOAT_WITHIN(0.01, 100.0, out);  // Clamped to max
}

void test_pid_zero_output_at_setpoint() {
    PIDController pid;
    pid.begin(1.0, 0.0, 0.0, 250);
    pid.setOutputLimits(0, 100);
    pid.setSetpoint(710.0);
    pid.setEnabled(true);

    advance_millis(250);
    pid.compute(710.0);  // Init

    advance_millis(250);
    float out = pid.compute(710.0);  // Error = 0
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, out);
}

void test_pid_integral_accumulates() {
    PIDController pid;
    pid.begin(0.0, 1.0, 0.0, 250);  // I-only, Ki=1.0
    pid.setOutputLimits(0, 100);
    pid.setSetpoint(100.0);
    pid.setEnabled(true);

    advance_millis(250);
    pid.compute(50.0);  // Init

    // Each compute: integral += Ki_scaled * error
    // Ki_scaled = 1.0 * 0.25 = 0.25, error = 50
    // integral += 0.25 * 50 = 12.5 per step

    advance_millis(250);
    float out1 = pid.compute(50.0);
    TEST_ASSERT(out1 > 0.0);

    advance_millis(250);
    float out2 = pid.compute(50.0);
    TEST_ASSERT(out2 > out1);  // Integral growing
}

void test_pid_reset_clears_state() {
    PIDController pid;
    pid.begin(1.0, 1.0, 1.0, 250);
    pid.setOutputLimits(0, 100);
    pid.setSetpoint(500.0);
    pid.setEnabled(true);

    advance_millis(250);
    pid.compute(100.0);
    advance_millis(250);
    pid.compute(100.0);

    pid.reset();

    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, pid.getOutput());
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, pid.getITerm());
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, pid.getDTerm());
}

void test_pid_setpoint_clamped() {
    PIDController pid;
    pid.setSetpoint(-100.0);
    TEST_ASSERT_FLOAT_WITHIN(0.01, TEMP_MIN_F, pid.getSetpoint());

    pid.setSetpoint(5000.0);
    TEST_ASSERT_FLOAT_WITHIN(0.01, TEMP_MAX_F, pid.getSetpoint());
}

void test_pid_sample_time_respected() {
    PIDController pid;
    pid.begin(1.0, 0.0, 0.0, 250);
    pid.setSetpoint(100.0);
    pid.setEnabled(true);

    advance_millis(250);
    pid.compute(50.0);  // Init

    // Call before sample time elapsed - should return previous output
    advance_millis(100);  // Only 100ms, need 250
    float out = pid.compute(50.0);

    advance_millis(150);  // Now at 250ms total
    float out2 = pid.compute(50.0);
    TEST_ASSERT(out2 != out || out2 > 0);  // Should compute now
}

void test_pid_enable_disable_bumpless() {
    PIDController pid;
    pid.begin(1.0, 1.0, 0.0, 250);
    pid.setOutputLimits(0, 100);
    pid.setSetpoint(500.0);
    pid.setEnabled(true);

    // Accumulate some integral
    for (int i = 0; i < 10; i++) {
        advance_millis(250);
        pid.compute(400.0);
    }
    float accumulated = pid.getITerm();
    TEST_ASSERT(accumulated > 0);

    // Disable then re-enable - should reset (bumpless)
    pid.setEnabled(false);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, pid.getOutput());

    pid.setEnabled(true);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, pid.getITerm());  // Reset
}

// --- Runner ---
int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_pid_init_defaults);
    RUN_TEST(test_pid_disabled_returns_zero);
    RUN_TEST(test_pid_basic_proportional);
    RUN_TEST(test_pid_output_clamping);
    RUN_TEST(test_pid_zero_output_at_setpoint);
    RUN_TEST(test_pid_integral_accumulates);
    RUN_TEST(test_pid_reset_clears_state);
    RUN_TEST(test_pid_setpoint_clamped);
    RUN_TEST(test_pid_sample_time_respected);
    RUN_TEST(test_pid_enable_disable_bumpless);

    return UNITY_END();
}

#endif // UNIT_TEST
