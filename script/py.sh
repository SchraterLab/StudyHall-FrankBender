#!/bin/bash
make clean
make -f Makefile
python3 py/router.py