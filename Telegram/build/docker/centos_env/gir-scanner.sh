#!/bin/bash
exec /usr/bin/g-ir-scanner --cflags-begin -std=gnu17 --cflags-end "$@"
