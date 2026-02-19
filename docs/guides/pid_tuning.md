# PID Tuning Guide

## Default Values

The factory PID values work well for most standard 100W barrel coils:

| Parameter | Default | Description |
|-----------|---------|-------------|
| Kp | 8.0 | Proportional gain |
| Ki | 0.2 | Integral gain |
| Kd | 2.0 | Derivative gain |

## When to Tune

Tune your PID if:
- Temperature oscillates continuously (overshooting and undershooting)
- Temperature takes too long to reach setpoint
- Temperature never quite reaches setpoint (steady-state error)
- You're using a non-standard coil (different wattage or size)

## Auto-Tune (Recommended)

The easiest way to tune is the built-in auto-tuner:

### Via OLED Menu:
1. Set your desired target temperature
2. Long press → Settings → PID Tuning → Auto-Tune
3. Press to start
4. Wait 3-5 minutes (progress bar shows completion)
5. New values are automatically applied and saved

### Via Web Dashboard:
1. Go to Settings → PID Tuning
2. Click "Auto-Tune" for the desired channel
3. Monitor progress in real-time
4. Review and accept the computed values

### How Auto-Tune Works
The auto-tuner uses the relay feedback method:
1. Heats at full power until the setpoint is reached
2. Switches between 0% and 100% power at setpoint crossings
3. Measures the oscillation period and amplitude
4. Computes optimal Kp/Ki/Kd using Ziegler-Nichols rules

## Manual Tuning

If you prefer to tune manually, follow this procedure:

### Step 1: Start with P-Only
1. Set Ki = 0, Kd = 0
2. Start with Kp = 2.0
3. Enable the channel and observe behavior
4. Increase Kp until the temperature starts oscillating around the setpoint
5. Note this value as **Ku** (ultimate gain)
6. Set Kp to 60% of Ku

### Step 2: Add Integral
1. Start with Ki = 0.05
2. Slowly increase Ki
3. Watch for the steady-state error to disappear
4. If oscillations appear, reduce Ki
5. Target: temperature settles at setpoint within 2-3 minutes

### Step 3: Add Derivative
1. Start with Kd = 0.5
2. Increase Kd to reduce overshoot on initial heat-up
3. Too much Kd causes jittery output - reduce if this happens
4. Target: minimal overshoot, smooth approach to setpoint

### Quick Reference (Z-N Rules)

If you found the ultimate gain Ku and oscillation period Tu:

| Controller | Kp | Ki | Kd |
|------------|----|----|-----|
| P only | 0.5 * Ku | - | - |
| PI | 0.45 * Ku | 0.54 * Ku / Tu | - |
| PID | 0.6 * Ku | 1.2 * Ku / Tu | 0.075 * Ku * Tu |

## Per-Profile Tuning

Different coil sizes may need different PID values:
- **16mm coil** (lower thermal mass): Lower Kp, faster response
- **25mm coil**: Default values usually work
- **30mm coil** (higher thermal mass): Higher Kp, more integral needed

Save tuned values to profiles so you don't need to retune when switching coils.

## Troubleshooting PID Issues

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| Constant oscillation | Kp too high | Reduce Kp by 20-30% |
| Never reaches setpoint | Ki too low | Increase Ki slowly |
| Slow response | Kp too low | Increase Kp |
| Large overshoot | Kd too low or Ki too high | Increase Kd, decrease Ki |
| Jittery output | Kd too high or noisy TC | Reduce Kd, check TC wiring |
| Temperature hunts +-20°F | Bad tuning overall | Run auto-tune |
