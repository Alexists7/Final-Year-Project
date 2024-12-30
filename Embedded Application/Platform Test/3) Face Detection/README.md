## Face Detection

### Laptop Program:
1. Timestamps the video now, on the side of the overview that appears.
1. Stores it in same manner as the Buffer Recording program, within a data directory.
1. A facial recognition method is now implemented using the python face_recognition package. Images are first taken by user, added to dataset, and encodings file is generated from this.
1. An alarm sound will be played for a certain amount of time, if person detected is not recognised.

### RB5 Program:
1. Program converted over for RB5 using gstreamer pipeline. 
1. Data is now stored within same directory for simplicity instead of ~/data directory of the embedded device.

### Next Steps:
1. Look to send over metadata and .mp4 to a database to store and to be easily retrievable by the desktop application.
1. Develop C# app, to be able to view these videos, and motions of your choosing, implement blurring algorithm within said Desktop app.



