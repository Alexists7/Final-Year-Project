## Motion Detection

### Laptop Program:
1. First carried out on laptop, able to simply open camera using opencv.
1. Converts video to grayframe and analyses motion between one frame and the next.
1. If there is definite motion, i.e, animal or human, incremenents counter.

### RB5 Program:
1. For RB5, needed to build opencv from source with gstreamer support using this [link](https://medium.com/@arfanmahmud47/build-opencv-4-from-source-with-gstreamer-ubuntu-zorin-peppermint-c2cff5393ef), as you need the qmmfsrc plugin to open the camera on the platform. The link only seems to build opencv for cpp and causing issues with python.
1. Same logic, converting to grayframe and analysing motion. Need to convert here to BGR from NV12 as cvtColor needs a 3-channel format.

### Problems encountered and possible solutions:
1. Unable to display live feed to wayland display, cable connected through HDMI, but able to view over ssh with -X forwarding. Issue with wayland compositor.
1. Just reading output now and not displaying feed to wayland. Can possibly stream instead to view video, instead of displaying on wayland going forward.


