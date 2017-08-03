#!/bin/sh

#
# You need ffmpeg. installing with these options gets you the heic support you need... and some stuff you don't need
#
# brew install ffmpeg --with-vpx --with-vorbis --with-libvorbis --with-vpx --with-vorbis --with-theora --with-libogg --with-libvorbis --with-gpl --with-version3 --with-nonfree --with-postproc --with-libaacplus --with-libass --with-libcelt --with-libfaac --with-libfdk-aac --with-libfreetype --with-libmp3lame --with-libopencore-amrnb --with-libopencore-amrwb --with-libopenjpeg --with-openssl --with-libopus --with-libschroedinger --with-libspeex --with-libtheora --with-libvo-aacenc --with-libvorbis --with-libvpx --with-libx264 --with-libxvid
#

# reset git repo
if [[ -d heif ]]; then
	rm -rf heif
fi
git clone https://github.com/nokiatech/heif.git

# build heif
cd heif
HEIF_ROOT=$(pwd)
cmake .
make
cd ..
cp heif/Srcs/*/*.a .
cp heif/Srcs/*/*/*.a .

# reset any previous output
if [[ -d build ]]; then
	rm -rf build
fi
mkdir build
if [[ -f a.out ]]; then
	rm a.out
fi

#compile tile extractor
c++ -I$HEIF_ROOT/Srcs/common \
 -I$HEIF_ROOT/Srcs/parser \
 -I$HEIF_ROOT/Srcs/parser/avcparser \
 -I$HEIF_ROOT/Srcs/parser/h265parser \
 -I$HEIF_ROOT/Srcs/reader \
 -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -L. -lcommon -lheifreader -ljson -lavcparser -lh265parser example_app.cpp 

 # processs tiles
 if [[ -f a.out ]]; then
 	echo ""
 	echo "Running example app"
 	echo ""
 	# TODO: the process_tiles.go script
	# needs to be updated use the actual metadata from the file.
	# atm it assumes an 8x6 grid and no rotation
  	./a.out $1 && mv *.tile build/. && go run process_tiles.go montage.sh && ./montage.sh
 else
 	echo "Compile fail!"
 	exit 1
 fi

open .