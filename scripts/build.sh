premake5 gmake
cd proj
config=debug_x86-64 make
config=release_x86-64 make
config=debug_x86 make
config=release_x86 make
cd ..