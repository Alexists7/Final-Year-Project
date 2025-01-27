## Desktop Application

### What has been done so far:
1. Home page developed changing theme from default to a darker style.
1. Further SignIn, SignUp and Display pages have been created using the help of Community Model-View-ViewModel (Mvvm) package.
1. Authentication has been implemented into these Mvvm pages, using firebase authentication for email.
1. Edge case conditions included, to ensure user enters all fields when signing up, or signing in.
1. A join as guest button has been implemented for those who do not wish to sign up. By default if you navigate to Display tab, it logs you in as guest.

### What could be done better if time permits:
1. The password checker just ensures that the password field is not empty or filled with whitespaces. Further test cases can be written to see that passwords have a minimum of 8 characters, one upperletter character and one special character.

### What I need to do next:
1. Work on the display page, integrating it to connect it to the RB5 device, to be able to pull the videos of motion recorded on device.
1. Create some sort of list/connection to determine which users are part of whitelist (admins), and therefore should be able to see unblurred video.
1. Create a program on laptop, to take in videos pulled from RB5, and blur and pixelate the faces within.
1. Create a display mechanism on app, to display video either blurred or unblurred depending on if user is in whitelist or not.
1. If all this can be done, work further to change the existing RB5 embedded program to include a multi-camera approach, to detect and record motions from different angles, and to choose the video which best shows the face.
