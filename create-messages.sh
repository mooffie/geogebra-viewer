#!/bin/bash

FNT="-pointsize 20"
#SIZE="-size 250x50"
SIZE="-size 300x50"

# New image can also be generated with "xc:white" instead of "null:" if we don't want a transparent image.

FL=message-no-access.png
LINE1="Error: I cannot access"
LINE2="this worksheet."

convert $SIZE null: "$FL"
mogrify $FNT -annotate 0x0+8+20 "$LINE1" "$FL"
mogrify $FNT -annotate 0x0+8+44 "$LINE2" "$FL"

FL=message-no-thumbnail.png
LINE1="This worksheet doesn't"
LINE2="contain a thumbnail."

convert $SIZE null: "$FL"
mogrify $FNT -annotate 0x0+8+20 "$LINE1" "$FL"
mogrify $FNT -annotate 0x0+8+44 "$LINE2" "$FL"
