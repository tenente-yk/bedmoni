#!/bin/sh

#find . -name '*.mn' -print | ./mn_parser xargs -0
rm ./*.men -f
rm ./*~ -f
./mn `find . -name '*.mn' -print` > /dev/null

#./mn_parser ./datetime.mn
#./mn_parser ./patient.mn


