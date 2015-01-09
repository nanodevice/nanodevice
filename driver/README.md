# Device driver

## Origins

This is based off the [Teensy raw HID driver](https://www.pjrc.com/teensy/rawhid.html).

The makefiles there were out of date, so I used xCode to get it to build and added a few bits to get it going. 

## Purpose

The purpose of this driver is to abstract as much of the protocol complexity as possible - ascii-binary conversion, creating json and so on. 

## TODO

1. Fix makefile, based on [Teensy raw HID driver](https://www.pjrc.com/teensy/rawhid.html), so this can be built on all platforms again. 
2. Maybe the sending protocol can be a bit more binary, so that we don't have to spend as much time parsing on the teensy. If it's necessary.
3. That's pretty much it for now. 

## Function

The `hidDriver` command line tool takes strings and sends them to the client when the `;` character arrives on stdin.

It parses the response and provides JSON that represents the devices response. 

Design drawbacks are that the protocol is a bit clumsy and non-standard. On the other hand, it's easy to debug and makes decent use of the USB speeds without creating lots of complexity. 