## Lightweight Motion Detector for Constrained Devices
*This motion sensor project is designed to detect motion in real-time, capture
and summarise those frames, and trigger an alarm when an unknown person is
detected. The software system will continuously monitor the environment using a
Raspi camera, sounding an alarm and recording images (frames) whenever motion
is detected. Every video frame will be numbered and timestamped, allowing for
precise tracking of motion events. Additionally, security codes will be implemented
to enable or disable the alarm system, ensuring secure control of the system’s
operation.*

### Pre-requisites:
1. RB5 device with [Thundercomm](https://www.thundercomm.com/product/qualcomm-robotics-rb5-development-kit/#sdk-manager) Linux Ubuntu Image flashed on-board
2. OpenCV built from [source](https://galaktyk.medium.com/how-to-build-opencv-with-gstreamer-b11668fa09c) with [gstreamer flag enabled](https://github.com/Alexists7/Final-Year-Project/blob/main/Embedded%20Application/Platform%20Test/1\)%20Motion%20Detection/opencv_compile_cmd.txt) on the RB5
3. Face-recognition and cv2 python packages installed using pip

### Embedded Application:
For development purposes, make sure you run an ssh -X terminal to the RB5 platform, through a linux host system. This will allow you to view the video feed.
1. To test the headshots script, that enables you to capture images used for training, run the following:
    ```bash
    cd Final-Year-Project/Embedded\ Application/Platform\ Test/3\)\ Face\ Detection/
    make
    ./headshots
    ```
    It will first prompt you to enter your name, then ask you to press space, to capture your image, and save it to the appropriate directory.

1. Once this is done, you can exit the file, and run the *./recognise.py* script, that trains on the images and generates a csv file as an output.

1. Once the csv file is generated, you can proceed to run the main application. The initial make command should have also created an executable file for this script. Simply run *./record_detect_faces*, to launch the file.

1. This main script, waits for motion, records based off detect motion, performs facial recognition, and triggers an alarm or continues, based on output received from recognition script. All of the videos are saved within a data folder for easy retrieval.

### Desktop Application:
The motion-based recordings can be then viewed on the Desktop Application build using .NET Maui, Model-View-ViewModel (MVVM) and Windows Presentation Foundation (WPF).

At the moment, the Desktop Application can only be run from VS Code in Debugging mode, as I have provided absolute folder path for development purposes. I hope in the future, to create it into an exectuable app with a self-signed certificate, allowing users on any machine, to access the app, and motion-based recordings within.
