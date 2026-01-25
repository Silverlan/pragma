#!/usr/bin/env bash
python3 build_scripts/build.py --with-pfm --with-all-pfm-modules --with-vr --with-networking --with-lua-debugger --with-swiftshader "$@"
read -p "Press [Enter] key to continue..."