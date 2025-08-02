# Hanoi-Tower-Claw-Composition
Tower Of Hanoi :

The Tower of Hanoi is a mathematical game or puzzle. It consists of three rods and a number of disks of different sizes, which can slide onto any rod. The puzzle starts with the disks in a neat stack in ascending order of size on one rod, the smallest at the top, thus making a conical shape.

The objective of the puzzle is to move the entire stack to another rod, obeying the following simple rules:

    1) Only one disk can be moved at a time.
    2) Each move consists of taking the upper disk from one of the stacks and placing it on top of another stack or on an empty rod.
    3) No larger disk may be placed on top of a smaller disk.
With 5 disks, the puzzle can be solved in 31 moves. The minimal number of moves required to solve a Tower of Hanoi puzzle is 2^n − 1, where n is the number of disks. In my case there are 5 disks therefore the minimum number of moves required are 31.


Hardware Introduction

This project uses the following hardware components to implement the automated Tower of Hanoi device:
# Disclaimer: All parameters have to be adjusted based on your stituation!
# 1. Arduino Uno R3

Description: The Arduino Uno R3 is a microcontroller development board based on the ATmega328P, responsible for controlling the stepper motor's movement and                     performing logic operations.

Purpose: Executes the Tower of Hanoi algorithm and sends commands to the CNC Shield to control the motors.

Specifications:

Operating Voltage: 5V

Digital I/O Pins: 14 (6 of which can be used as PWM outputs)

Analog Input Pins: 6

Flash Memory: 32KB

# 2. Arduino CNC Shield

Description: The CNC Shield is an expansion board designed for the Arduino Uno, used to control stepper motors.

Purpose: Connects to stepper motor drivers to precisely control the motor's movement to move the Tower of Hanoi disks.

Specifications:

Supports 4-axis stepper motor control (this project uses 2 axes)

Compatible with A4988 or DRV8825 stepper motor drivers

Provides power management and pin expansion

# 3.Usongshine 17H4401 Stepper Motor

Description: The Usongshine 17H4401 is a high-performance NEMA 17 stepper motor that provides high torque and precise control.

Purpose: This project uses two 17H4401 stepper motors, each responsible for:

Controlling the horizontal movement of the robotic arm between three posts.

Controlling the vertical movement of the robotic arm to pick up and place a disk.

Specifications:

Step Angle: 1.8° (200 steps per revolution)

Rated Current: 1.7A

Holding Torque: 40 N.cm

Number of Phases: 2
