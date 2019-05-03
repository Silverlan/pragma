filepath=$1
origin=$2
if [ -e $filepath ]; then
	patchelf --set-rpath $origin $filepath
fi;
