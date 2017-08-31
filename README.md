# Touch2Tuio
Forwarding Windows touch events over Tuio.
I'm using this to develop a tuio based application on OSX where the dev touchscreen has no OSX TUIO support.
Connected the touchscreen usb to a windows box where I capture touches and forwared them using the TUIO lib from tuio.org.

Added a binary in the bin folder.
Commandline usage for extra args.

Touch2Tuio.exe host port startFullscreen
example:
Touch2Tuio.exe 10.200.10.179 3333 f



[![Demo video](https://image.ibb.co/dKeBxQ/Screen_Shot_2017_08_31_at_13_26_20.png)](https://vimeo.com/231852693)
