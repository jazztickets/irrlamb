#!/bin/sh
for f in $(ls *.irr 2>/dev/null); do
	d=${f%.*}
	mkdir -p levels/$d
	mv $f levels/$d
done

for f in $(ls *.obj 2>/dev/null); do
	../bin/Release/colmesh $f
	d=${f%.*}
	# used in caves_4 to move caves_4{a,b,static}.col into caves_4
	#l=`echo $d | grep -P '^.*_[0-9]+' -o`
	l="$d"
	mkdir -p levels/$l
	mv $d.col levels/$l
	rm $f
done

for f in $(ls meshes/*.irrmesh 2>/dev/null); do
	nf=${f%.*}.irrbmesh
	$IMESHCVT -i $f -o $nf
	rm $f
done

rm -f irrb.log

# test level
if [ -n "$1" ]; then
	../bin/Release/irrlamb -level "$1"
fi
