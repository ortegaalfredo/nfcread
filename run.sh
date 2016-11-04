rec -r 96000 -e signed-integer --endian little --bits 16 --channels 1 -t s16 - 2>/dev/null | ./sdlview | ./passband
