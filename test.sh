#!/bin/bash
make -j
gnome-terminal -- ./serv &
gnome-terminal -- ./dev 4847 &
gnome-terminal -- ./dev 4848 &
gnome-terminal -- ./dev 4849 &