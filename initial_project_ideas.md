## Code: MA3, Title: Lightweight Motion Detector

**A motion sensor project that detects motion, summarised those frames, and may sound an alarm
accordingly. This software system will constantly monitors an environment using a Raspi camera and
sounds an alarm and records images of the motion taking place as soon as it takes place. Every video
frame will be numbered and also take time, so that motion timing can also be recorded. Security codes
will also be set up for activating and deactivating alarms.**

### Notes (Very Random):
The above system is just like a cctv but with embedded device.
Using RB5 as platform.
(Try using cpp for image processing side, if too hard, switch to python)

Possible additions:
If possible, train and implement an ML model, to detect a certain list of people, and if people are in list, the alarm is not triggered.
That way, workers, home owners, can just enter the house, without problem.
Maybe find a way to add/remove people from the above list.

Camera constantly running, like a cctv.
If motion detected, take next step: check if person in ML model list, if so, do nothing, if stranger, then sound alarm, and warn on android app.

Make some sort of on image-ui and logic to disable alarm system.

The alarm will still have value, as if the ML model fails to recognise the person on whitelist, it triggers alarm and person will need to manually disable it.

Implement system for storing images from video frame, including time, date and other info.


### Key Components of the System:
1. Motion Detection: Detect motion using a camera feed.
1. Face Recognition: Process the camera feed to recognize faces.
1. Whitelist/Database: Maintain a database (local or cloud-based) of known individuals (whitelist).
1. Alarm Trigger Logic: Sound the alarm if a face is unrecognized or on a blacklist.
1. Android App for Control: Display the alert and allow guests or authorized users to disarm the alarm with a password.
1. Alarm System Redundancy: Even with facial recognition, the alarm system would still have value in case of failures in detection or intrusion by a masked or obscured person.

### Things to use:
1. (OpenCV - cpp/python)
1. (TensorFlow/Pytorch? Implement on RB5, train model, deploy successfully on RB5)
1. (Firebase or some sort of DB, should be modifiable from website/app)
1. (cpp/python logic code, on the RB5 itself, send info to app, rb5 or server side, need to find a way to store this data of frames, times)
1. (Java/Kotlin android app that connects with RB5 through wifi and/or bluetooth)
