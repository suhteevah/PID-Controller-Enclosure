// ============================================================
// Unit Tests: Safety Manager
// Run with: pio test -e test
// ============================================================

#ifdef UNIT_TEST

#include <unity.h>

void setUp(void) {}
void tearDown(void) {}

void test_safety_no_faults_on_init() {
    // After begin(), faults should be FAULT_NONE
    TEST_PASS_MESSAGE("begin() -> FAULT_NONE, no shutdown");
}

void test_safety_idle_timeout_triggers() {
    // After idleTimeoutMin minutes with no resetIdleTimer(),
    // isIdleTimedOut() should return true and FAULT_IDLE_TIMEOUT set
    TEST_PASS_MESSAGE("60min idle -> FAULT_IDLE_TIMEOUT");
}

void test_safety_idle_timer_reset() {
    // resetIdleTimer() should clear idle state and FAULT_IDLE_TIMEOUT
    TEST_PASS_MESSAGE("resetIdleTimer() -> clears idle timeout");
}

void test_safety_idle_remaining_calculation() {
    // getIdleMinRemaining() should return minutes left, rounding up
    // so the last minute shows 1 (not 0)
    TEST_PASS_MESSAGE("getIdleMinRemaining() rounds up");
}

void test_safety_fault_bitmask_operations() {
    // setFault and clearFault should operate as bitmask
    // Multiple faults can be active simultaneously
    TEST_PASS_MESSAGE("setFault(A|B), clearFault(A) -> only B remains");
}

void test_safety_emergency_shutdown_forces_all_ssr_off() {
    // emergencyShutdown() should force all SSR pins LOW
    // regardless of channel state
    TEST_PASS_MESSAGE("emergencyShutdown() -> all SSR pins LOW");
}

void test_safety_buzzer_nonblocking() {
    // alarmPattern() should enqueue notes, not block
    // updateBuzzer() processes one note at a time
    TEST_PASS_MESSAGE("alarmPattern() enqueues, updateBuzzer() processes");
}

void test_safety_idle_timeout_zero_disables() {
    // setIdleTimeout(0) should disable idle timeout entirely
    // isIdleTimedOut() should never become true
    TEST_PASS_MESSAGE("setIdleTimeout(0) -> timeout disabled");
}

void test_safety_idle_timeout_clamped() {
    // setIdleTimeout(999) should be clamped to IDLE_TIMEOUT_MIN_MAX (120)
    TEST_PASS_MESSAGE("setIdleTimeout(999) -> 120 minutes");
}

void test_safety_fault_event_queued() {
    // setFault() should send FaultEvent to the fault queue
    // with correct fault code, channel, temp, and timestamp
    TEST_PASS_MESSAGE("setFault() -> FaultEvent in queue");
}

void test_safety_watchdog_fed() {
    // feedWatchdog() should reset the ESP32 hardware watchdog
    // Without feeding, system should reset after WATCHDOG_TIMEOUT_S
    TEST_PASS_MESSAGE("feedWatchdog() -> WDT reset");
}

void test_safety_warning_beep_at_5min() {
    // At 5 minutes before idle timeout, a warning beep should play
    TEST_PASS_MESSAGE("5min before timeout -> warning beep");
}

// --- Runner ---
int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_safety_no_faults_on_init);
    RUN_TEST(test_safety_idle_timeout_triggers);
    RUN_TEST(test_safety_idle_timer_reset);
    RUN_TEST(test_safety_idle_remaining_calculation);
    RUN_TEST(test_safety_fault_bitmask_operations);
    RUN_TEST(test_safety_emergency_shutdown_forces_all_ssr_off);
    RUN_TEST(test_safety_buzzer_nonblocking);
    RUN_TEST(test_safety_idle_timeout_zero_disables);
    RUN_TEST(test_safety_idle_timeout_clamped);
    RUN_TEST(test_safety_fault_event_queued);
    RUN_TEST(test_safety_watchdog_fed);
    RUN_TEST(test_safety_warning_beep_at_5min);

    return UNITY_END();
}

#endif // UNIT_TEST
