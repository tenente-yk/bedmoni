#!/bin/sh

rm ./*.cc -f
rm ./*~ -f
./bmp2c `find . -name '*.bmp' -print` > /dev/null


