#!/bin/bash

./waf clean
./waf configure --build-profile=debug --disable-python
./waf
