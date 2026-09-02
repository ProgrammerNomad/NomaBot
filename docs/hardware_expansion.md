# NomaBot Hardware Expansion Guide

This document outlines the detailed implementation plan for adding **Capacitive Touch Sensing** and an **IMU/Accelerometer (MPU6050)** to NomaBot, turning it into a truly interactive desktop pet.

---

## 1. Touch Sensing (Capacitive)

Adding touch sensing allows the user to "pet" the device, triggering happy or affectionate animations.

### Hardware Requirements
- **Sensors**: None! The ESP32-S3 has built-in capacitive touch pins.
- **Materials**: Copper foil tape (or aluminum foil) and a single jumper wire.
- **Wiring**: Solder one end of the jumper wire to the copper foil tape. Solder the other end to an available touch-capable GPIO pin on the LilyGo T-Display S3 (e.g., `GPIO 4`). 
- **Enclosure Design Changes**:
  - The 3D printed case needs a flat or slightly curved area at the top ("head" of the pet) where the copper tape can be affixed *inside* the case. 
  - Capacitive sensing works through thin plastic (up to ~2-3mm). Ensure the plastic shell is thin enough above the copper tape.

### Code Implementation
1. **Initialize Touch Pin**: In `main.cpp` or a new `touch_service.cpp`, configure the touch pin using `touchAttachInterrupt()`.
2. **Read Values**: Use `touchRead(pin)` in the main loop to read the raw capacitance. When the value drops below a certain threshold (calibrated for your specific 3D case), a touch is registered.
3. **Debouncing**: Implement a simple cooldown timer (e.g., 500ms) to avoid registering a single touch as multiple pets.
4. **Trigger Animation**: When touched, send a command to the `CharacterRuntime` to override the current animation clip to a "happy" or "heart eyes" animation.

---

## 2. IMU / Accelerometer (MPU6050)

Adding an accelerometer allows NomaBot to react to physical movement—like being picked up, shaken, or laid down to sleep.

### Hardware Requirements
- **Module**: MPU6050 I2C 6-DOF Accelerometer/Gyroscope module (GY-521).
- **Wiring**:
  - **VCC** ➔ 3.3V (LilyGo)
  - **GND** ➔ GND (LilyGo)
  - **SDA** ➔ GPIO 43 (LilyGo JST connector or available pin)
  - **SCL** ➔ GPIO 44 (LilyGo JST connector or available pin)
- **Enclosure Design Changes**:
  - Add mounting standoffs inside the 3D printed case to securely screw in the MPU6050 board. It must be rigidly mounted to the case to accurately detect motion.

### Code Implementation
1. **Library**: Add `Adafruit MPU6050` or `ElectronicCats/MPU6050` to your `platformio.ini` dependencies.
2. **Service Integration**: Implement the empty methods inside `src/ambient/motion_service.cpp`.
3. **Behavior Logic**:
  - **Shake Detection**: Calculate the total acceleration magnitude: `sqrt(ax^2 + ay^2 + az^2)`. If it exceeds a high threshold (e.g., > 2.5g) repeatedly, trigger a "dizzy" animation.
  - **Sleep Detection**: Check if the Z-axis acceleration is inverted (meaning the device is laid flat on its face or back). If this state is maintained for > 3 seconds, change the ambient mode to `NightMode` (dimming the screen and entering a sleeping animation).
  - **Tap Detection**: The MPU6050 has built-in tap detection interrupts that can be routed to an ESP32 pin to detect when the desk is knocked.

---

## 3. Screen Design & Animation Creation

To support these new physical interactions, new screen animations need to be created.

### Required New Animations (Sprite Sequences)
1. **Heart / Happy Eyes**: For the touch sensor (petting). Eyes turning into hearts, curving upward in a smile, or a pulsing "blush" effect.
2. **Dizzy Eyes**: For the shake detection. Spirals in the eyes or eyes rapidly bouncing around the screen out of sync.
3. **Sleeping / Yawning**: For the lay-down detection. Eyelids drooping, a "Zzz" bubble, and a slow, dark, breathing animation.

### Easy Animation Creation Pipeline
Instead of manually drawing every frame in C++ code, you should create assets in a visual editor and pack them using the existing NomaBot tools.

1. **Pixel Art Software (Aseprite / Piskel)**:
   - Create your animations in a tool like **Aseprite** (paid) or **Piskel** (free, web-based).
   - Set the canvas size to the bounding box of your eyes (e.g., `100x60` pixels).
   - Use a clear color for the background (e.g., pure magenta `#FF00FF` or pure green `#00FF00`) which will be used as the `colorKey` in the firmware.
2. **Export**: Export the animation as a sequence of PNG files (e.g., `happy_001.png`, `happy_002.png`).
3. **Asset Generation**:
   - Use the Python scripts in the `scripts/` or `sdk/` directory (e.g., `generate_eyes_art.py`) to pack these PNG sequences into the optimized binary `pack.bin` format used by NomaBot.
   - The script will automatically convert the PNGs to RGB565 and generate the metadata needed by `CharacterRuntime` to play them back smoothly.

By using this sprite-based workflow, you can rapidly iterate on the visual personality of the pet without writing complex C++ drawing routines!
