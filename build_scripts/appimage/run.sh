#!/bin/bash

CUR_DIR="$(dirname "$(readlink -f "${0}")")"

case "$1" in
    pragma)
        shift
        exec "$CUR_DIR/usr/bin/pragma" -user_data_dir "~/.local/share/pfm" "$@"
        ;;
    pragma_server)
        shift
        exec "$CUR_DIR/usr/bin/pragma_server" -user_data_dir "~/.local/share/pfm" "$@"
        ;;
    prad)
        shift
        exec "$CUR_DIR/usr/bin/lib/prad" "$@"
        ;;
    render_raytracing)
        shift
        exec "$CUR_DIR/usr/bin/lib/render_raytracing" "$@"
        ;;
    udm_convert)
        shift
        exec "$CUR_DIR/usr/bin/lib/udm_convert" "$@"
        ;;
    updater)
        shift
        exec "$CUR_DIR/usr/bin/lib/updater" "$@"
        ;;
    *)
        exec "$CUR_DIR/usr/bin/pfm" -user_data_dir "~/.local/share/pfm" "$@"
        ;;
esac
