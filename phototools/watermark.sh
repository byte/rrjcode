#!/bin/sh

echo "Enter watermarked message"
read response

# font supports a filename (path to font)
for img in `ls *.jpg`
do
	convert -font arial -fill white -pointsize 36 -draw 'text 10,50 "$response" ' $img watermarked-$img
done
