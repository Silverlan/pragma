patch_library() {
    if [ -e $1 ]; then
	    patchelf --set-rpath '$ORIGIN' $1
	    patchelf --add-rpath "./lib" $1
    fi;
}

for f in $(find "$1/lib" -name '*.so*'); do
    echo "Patching $f ..."
    patch_library "$f"
done

for f in $(find "$1/modules" -name '*.so*'); do
    echo "Patching $f ..."
    patch_library "$f"
done
