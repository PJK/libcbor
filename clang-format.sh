#!/usr/bin/env bash

# TravisCI workaround to use new clang-format while avoiding painful aliasing
# into the subshell
if which clang-format-8; then
    clang-format-8 -style=file -i **/*.c **/*.h **/*.cpp
else
    clang-format -style=file -i **/*.c **/*.h **/*.cpp
fi

