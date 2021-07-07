premake5 --clang gmake
cd proj
config=debug_x86-64 make -j12
config=release_x86-64 make -j12
cd ..