# Visual-Based Voice Alert System for the Visually Impaired

## Introduction
This project presents a visual-based voice alert system. The system detects different objects, including humans, bicycles, cars…etc. with convolutional neural networks, and makes voice alerts upon detection. It is expected to provide those with visually impaired with more awareness of the surroundings and make them safer on the road.

## HW/SW Setup
### Hardware
* Himax WE-I Plus EVB
* PCM5102 DAC module
* Speaker

The WE-I Plus EVB is connected to the DAC module through I2C protocol, and the DAC module is connected to the speaker with a 3.5mm audio connector.

### Software
No additional software required

## User Manual
### Prerequisites
* Tera Term
* USB Serial Port driver

### Flashing the image
1. Clone the repository

    `$ git clone https://github.com/vncntlin/vision_studio.git`
2. Connect the WE-I EVB to the PC with an USB cable
3. Open Tera Term and select "COMx: USB Serial Port (COMx)"
4. Change baud to 115200
5. Keep pressing "1" on the keyboard and press the reset button on the WE-I Plus EVB
6. Transfer `output_gun.img` to the WE-I Plus EVB
7. Press the reset button again and run the image, and the system is ready to go

### Portable system
To bring the system outdoor, connect the power supply to the WE-I Plus EVB and the speaker. The system is now protable.

## Team Name
Vision Studio
