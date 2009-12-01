#!/bin/sh

for img in `ls *.jpg`
do
	convert -sample 25%x25% $img thumb-$img
done

