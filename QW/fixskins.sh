#!/bin/sh
for x in *; do
	y=`echo $x | tr '[A-Z]' '[a-z]'`
	if [ $x != $y ]; then
		mv $x $y
	fi
done
	
