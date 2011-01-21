This is a fork of Edd Dawson's sigfwd library [http://mr-edd.co.uk/code/sigfwd]
with a few minor changes (see CHANGES.txt).

sigfwd is header-only that depends on Qt and Boost.  It is developed against Qt
4.7 and Boost 1.45.  See demo/main.cpp for example usage.

To compile the demo:
    Add Boost headers to your PATH
    $ python waf/waf configure build --qtdir=/path/to/qt --qtbin=/path/to/qt/bin --qtlib=/path/to/qt/lib

To run the demo:
    $ build/sigfwd_demo
