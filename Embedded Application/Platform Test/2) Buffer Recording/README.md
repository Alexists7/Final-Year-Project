## Buffer Recording

### Laptop Program:
1. Added a buffer system to store video in frames.
1. When 2 seconds of continuous motion is detected, recording starts, capturing frames a bit before motion, and frames 2 seconds after motion, and encodes that into an mp4 file.
1. A sub-directory called data is created, within it, another sub-directory named today's date, and within that directory are saved the .mp4 and the .txt metadata file generated.

### RB5 Program:
1. Program converted over for RB5 using gstreamer pipeline. 
1. Data now stored in the actual /data directory available on the RB5 platform. Within it, directories containing date's, similar to laptop program, and .mp4's and .txt files within these "dated" directories.

### Next Steps:
1. Implement face recognition and add into current program.
1. Look to send over metadata and .mp4 to a database to store.
1. Create alarm system, to de-activate, if person's face not recognised etc.
1. Potentially develop app, to sort through date, time, and view motion recordings.



