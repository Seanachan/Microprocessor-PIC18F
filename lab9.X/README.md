# Basic
## Description
Design a system using four LEDs to represent decimal numbers (0–9) in
binary format. A variable resistor serves as the control input. When the
resistor is rotated at a constant speed, the LEDs should illuminate
sequentially to display today’s date in binary form. For example, if today’s
date is 2025/01/01, the LEDs should display the binary representations of
the sequence: 2, 0, 2, 5, 0, 1, 0, 1. Each digit should be represented clearly
using the four-LED binary display.

# Advance
## Description
Implement a system that uses four LEDs to indicate whether a measured
value is odd or even, based on the voltage level read from a variable resistor.
As the resistor is rotated, the ADC module continuously samples the voltage.
The LEDs should display a binary number corresponding to the measured
value, following these rules:
- When the voltage increases, the LEDs display even numbers: `0, 2, 4, 6, 8, 10, 12, 14`
- When the voltage decreases, the LEDs display odd numbers: `1, 3, 5, 7, 9, 11, 13, 15`

# Hard
## Description
Implement a system using a variable resistor to control the brightness of an
LED by adjusting its PWM duty cycle. Rotating the resistor should
dynamically vary the LED’s intensity according to the following rule:
- Clockwise rotation: decreases brightness (dimming)
- Counterclockwise rotation: increases brightness
## Hint
1. You can use the PWM implementation and setup you learned in Lab 8. 
2. Ensure compliance with hardware timing constraints: $TAD$(>= 0.7 $\mu $s) and acquisition time (>= 2.4 $\mu s$).

