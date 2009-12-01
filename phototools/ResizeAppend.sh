#!/bin/sh

# resize the JPEGs or PNGs or whatever
# Colin Charles <byte@aeon.com.my>
# Thu Mar 10 15:43:49 EST 2005


echo "Enter re-sized width" 
read response

if [ $1 = '-J' ]; then
ls -1 *.JPG | sed "s/\(.*\)\.JPG/\1.JPG \1_thumb.JPG/" | xargs -n 2 convert -resize $response -rotate 90
fi

if [ $1 = '-j' ]; then
ls -1 *.jpg | sed "s/\(.*\)\.jpg/\1.jpg \1_thumb.jpg/" | xargs -n 2 convert -resize $response -rotate 90
fi


