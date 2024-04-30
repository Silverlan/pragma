root_dir=$1
lib_dir="$root_dir/lib"


patch_library() {
    if [ -e $1 ]; then
	    patchelf --set-rpath '$ORIGIN:$ORIGIN/lib' $1;
    fi;
}


patch_module() {

    if [ -e $1 ]; then
        #https://stackoverflow.com/a/28523143
        relative_path=$(realpath --relative-to="$(dirname "$1")" "$lib_dir")
	    patchelf --set-rpath '$ORIGIN' $1;
	    patchelf --add-rpath "\$ORIGIN/$relative_path" $1
    fi;
}

patch_module_unirender() {

    if [ -e $1 ]; then
        #https://stackoverflow.com/a/28523143
        relative_path=$(realpath --relative-to="$(dirname "$1")" "$lib_dir")
	    patchelf --set-rpath '$ORIGIN' $1;
	    patchelf --add-rpath "\$ORIGIN/$relative_path" $1
	    patchelf --add-rpath "\$ORIGIN/cycles" $1
    fi;
}


#This will iterate over all encountered SO and executables of ELF. See https://unix.stackexchange.com/a/699959.

# Some ELFs are not marked as executables. That is fine. We now account for that too.
for f in $(find "$root_dir/lib" -type f -exec bash -c "[[ \"\$(head -c 4 -- \"\${1}\")\" == \$'\\x7FELF' ]]" -- \{\} \; -print 2> /dev/null); do
    echo "Patching $f ..."
    patch_library "$f"
done


for f in $(find "$root_dir/modules" -type f -exec bash -c "[[ \"\$(head -c 4 -- \"\${1}\")\" == \$'\\x7FELF' ]]" -- \{\} \; -print 2> /dev/null ); do
    if [[ "$f" == *"libtbb"* ]]; then
        # skip libtbb otherwise we will corrupt it.
        echo "Skipped $f..."
        continue
    fi;
    if [[ "$f" == *".cubin" ]]; then
        # Cubins are compiled via nvcc, skip those too.
        echo "Skipped $f..."
        continue
    fi;
    if [[ "$f" == *"modules/unirender"* && "$f" != *"modules/unirender/cycles"* ]]; then
        #special handling. To avoid duplication of dependencies we add cycles directory to rpath.
        echo "Patching $f ..."
        patch_module_unirender "$f"
        continue
    fi;
    echo "Patching $f ..."
    patch_module "$f"
done


for f in $(find "$root_dir/bin" -type f -exec bash -c "[[ \"\$(head -c 4 -- \"\${1}\")\" == \$'\\x7FELF' ]]" -- \{\} \; -print 2> /dev/null ); do
    echo "Patching $f ..."
    patch_module "$f"
done


for f in $(find $root_dir -maxdepth 1 -type f -exec bash -c "[[ \"\$(head -c 4 -- \"\${1}\")\" == \$'\\x7FELF' ]]" -- \{\} \; -print 2> /dev/null); do
    echo "Patching $f ..."
    patch_library "$f"
done


