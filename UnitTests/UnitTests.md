# Google test code

These unit tests are designed to test a small portion of the GRBL_ESP32
code, directly from your desktop PC. This is not a complete test of 
GRBL_ESP32, but a starting point from which we can move on. Testing and 
debugging on a desktop machine is obviously much more convenient than 
it is on an ESP32 with a multitude of different configurations, not to
mention the fact that you can use a large variety of tools such as 
code coverage, profiling, and so forth.

Code here is split into two parts:
1. A subset of the GRBL code is compiled. Over time, this will become more.
2. Unit tests are executed on this code.

