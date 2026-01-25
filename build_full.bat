@echo off
python build_scripts/build.py --with-pfm --with-all-pfm-modules --with-vr --with-networking --with-lua-debugger --with-swiftshader %*
pause