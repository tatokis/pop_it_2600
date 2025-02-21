#!/usr/bin/env bash
# Thin wrapper around make because I didn't have time to make a proper flash target in the Makefile
set -xe

make

# Due to how I built my flashcart, the binary needs to be in the last bank.
srec_cat pop_it -Binary -offset 0x0003f000 > pop_it.srec

read -p 'Press Return to write to the cart'
minipro -p w29c020 -w pop_it.srec && printf \\a

