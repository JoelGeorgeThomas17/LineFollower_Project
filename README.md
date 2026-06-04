# Line-Follower Robot with Two Sensors (Threshold + P-Control)

## Overview

This project implements a line-following robot using two IR sensors. It features two different operating modes:

- **Threshold Mode** (simple black/white logic)
- **P-Control Mode** (proportional control for smooth tracking)

The project demonstrates the difference between basic rule-based control and feedback-based control.

---

## Features

- Two IR sensors for line detection
- Mode switching between Threshold and P-Control
- Basic reactive control system
- Proportional steering correction
- Adjustable motor speed and gain (Kp)

---

## Operating Modes

### Threshold Mode (Simple Logic)

This mode uses direct black/white detection from sensors.

**Behavior:**
- Both sensors on white → move forward
- Left sensor on black → turn left
- Right sensor on black → turn right
- Both sensors on black → stop or special case handling

**Pros:**
- Simple and fast
- Easy to implement

**Cons:**
- Jerky movement
- Poor curve handling

---

### P-Control Mode (Proportional Control)

This mode uses continuous sensor values to calculate steering correction.

**Control Law:**
error = left_sensor - right_sensor
correction = Kp * error

**Motor Control:**
- Left motor speed = base_speed + correction
- Right motor speed = base_speed - correction

**Pros:**
- Smooth movement
- Better curve tracking
- More stable behavior

**Cons:**
- Requires tuning (Kp)
- Sensitive to sensor noise

---

## Hardware Requirements

- Arduino or compatible microcontroller
- 2 × IR sensors
- 2 × DC motors
- Motor driver (L298N / L293D)
- Robot chassis
- Power supply (battery pack)

---

## System Description

The robot uses two IR sensors placed on the left and right front side. Based on their readings, the robot determines its position relative to the line and adjusts motor speeds accordingly.

Black typically represents the line, and white represents the background.

---

## Program Flow

### Mode Selection
if mode == THRESHOLD:
run_threshold_mode()
else if mode == P_CONTROL:
run_p_control_mode()

---

### Threshold Mode Logic
if left == WHITE and right == WHITE:
move_forward()

elif left == BLACK and right == WHITE:
turn_left()

elif left == WHITE and right == BLACK:
turn_right()

else:
stop()

---

### P-Control Logic
error = left_sensor - right_sensor
correction = Kp * error

left_motor_speed = base_speed + correction
right_motor_speed = base_speed - correction

set_motor_speeds(left_motor_speed, right_motor_speed)

---

## Tuning Guide

### Threshold Mode
- Adjust sensor threshold values carefully

### P-Control Mode
- Start with low Kp value
- Increase gradually until response is stable
- Reduce Kp if oscillations occur

---

## Limitations

- Sensitive to lighting
- Threshold mode is not smooth
- P-control may oscillate if not tuned properly and not sufficient to generate a smooth motion

---

## Future Improvements

- Implement PID control (P + I + D)
- Add more sensors for better precision
- Adaptive speed control for curves
