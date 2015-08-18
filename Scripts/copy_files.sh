#!/bin/bash

mkdir assets
mkdir assets/PackedContent

for f in ../../../Dist/PackedContent/*.*
do
	outfile=`echo $f | grep -o "PackedContent/.*"`

	cp $f assets/$outfile.mp3
done

