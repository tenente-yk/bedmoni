#!/bin/sh

rm ./*.h -f
rm ./*~ -f
./xml2h -f ./csio_typedefs.xml -o ./csio_typedefs.h > /dev/null

mv *.h ../modules


