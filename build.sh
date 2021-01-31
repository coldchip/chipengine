#!/bin/sh

cp /home/ryan/compiler/data/out.bin bin/out.bin
ld -r -b binary -o bin/chipcode_program.o bin/out.bin

rep='.o'

for d in ./*.c; do
	echo  gcc -c -Ofast -s -o bin/$(echo $(basename "$d") | sed "s/\.c/$rep/") $d
    gcc -c -Ofast -s -o bin/$(echo $(basename "$d") | sed "s/\.c/$rep/") $d
done

echo gcc -o bin/chipengine bin/*.o

gcc -o bin/chipengine bin/*.o -s -Ofast