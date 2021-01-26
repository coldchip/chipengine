#!/bin/sh
rep='.o'

for d in ./*.c; do
	echo -c -o bin/$(echo $(basename "$d") | sed "s/\.c/$rep/") $d
    gcc -c -o bin/$(echo $(basename "$d") | sed "s/\.c/$rep/") $d
done

echo 'Linking'

gcc -o bin/chipengine bin/*.o