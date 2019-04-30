filepath=$1
if [ -e $filepath ]; then
	path=$(dirname $filepath)/
	filename=$(basename $filepath)
	soname=$( objdump -p $path$filename | grep SONAME )
	soname=lib${soname#*lib}
	if [ $filename != $soname ]; then
		echo "Renaming library $filename to match its SONAME $soname"
		mv $path$filename $path$soname
	fi;
fi;
