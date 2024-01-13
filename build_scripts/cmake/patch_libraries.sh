root_dir=$1
lib_dir="$root_dir/lib"


patch_library() {
    if [ -e $1 ]; then
	    patchelf --set-rpath '$ORIGIN:$ORIGIN/lib' $1;
	    #patchelf --add-rpath '' $1
    fi;
}


patch_module() {

    if [ -e $1 ]; then
        #https://stackoverflow.com/a/28523143
        relative_path=$(realpath --relative-to="$(dirname "$1")" "$lib_dir")
	    patchelf --set-rpath "\$ORIGIN:\$ORIGIN/$relative_path" $1;
	    #patchelf --add-rpath "\$ORIGIN/$relative_path" $1
    fi;
}


#This will iterate over all encountered SO and executables of ELF. The CUDA kernels are thankfully ommitted. See https://unix.stackexchange.com/a/699959.
for f in $(find "$1/lib" -type f -executable -exec sh -c "[[ \"\$(head -c 4 -- \"\${1}\")\" == \$'\\x7FELF' ]]" -- \{\} \; -print); do
    echo "Patching $f ..."
    patch_library "$f"
done

for f in $(find "$1/modules" -type f -executable -exec sh -c "[[ \"\$(head -c 4 -- \"\${1}\")\" == \$'\\x7FELF' ]]" -- \{\} \; -print); do
    if [[ "$f" == *"libtbb"* ]]; then
        # skip libtbb otherwise we will corrupt it.
        echo "Skipped $f..."
        continue
    fi;
    echo "Patching $f ..."
    patch_module "$f"
done


for f in $(find $1 -maxdepth 0 -type f -executable -exec sh -c "[[ \"\$(head -c 4 -- \"\${1}\")\" == \$'\\x7FELF' ]]" -- \{\} \; -print); do
    echo "Patching $f ..."
    patch_library "$f"
done


