#!/bin/sh

cd "$(dirname "$0")" || exit
echo "cleanup"
rm -rf ./build ./fastfetch fastfetch.deb

echo "building fastfetch"
mkdir build
cd build || exit
cmake ../../..
cmake --build . --target fastfetch "-j$(nproc)"

echo "generating package contents"
# deb versions must start with a number, hence the 1 added before the rXXX
version="0:1$(./fastfetch --version | cut -d ' ' -f2)-0"
echo "Using package version: $version"
cd ..

mkdir -p fastfetch/DEBIAN
sed "s/<VERSION>/$version/g" control-template > fastfetch/DEBIAN/control

mkdir -p fastfetch/usr/bin
cp build/fastfetch fastfetch/usr/bin/
mkdir -p fastfetch/usr/share/bash-completion/completions
cp ../../completions/bash fastfetch/usr/share/bash-completion/completions/fastfetch
mkdir -p fastfetch/usr/share/fastfetch/
cp -r ../../presets fastfetch/usr/share/fastfetch/

echo "building package"
dpkg-deb --build fastfetch
echo "package built."
echo "cleaning up"
rm -rf build fastfetch