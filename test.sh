#!/bin/bash
make clean
make -j SPECIFICATION=STRICT MODE=RELEASE
gnome-terminal --title="Server"     -- ./serv &
gnome-terminal --title="Device 1"   -- ./dev 4847 &
gnome-terminal --title="Device 2"   -- ./dev 4848 &
gnome-terminal --title="Device 3"   -- ./dev 4849 &