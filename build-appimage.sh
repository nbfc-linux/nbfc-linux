#!/bin/bash
make
mkdir AppDir
DESTDIR=AppDir make install

if [ ! -e "appimagetool-x86_64.AppImage" ] ; then
    wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" "appimagetool-x86_64.AppImage"
fi

wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-x86_64" "Appdir/AppRun"

cp "data/nbfc-linux.svg" "Appdir/nbfc-linux.svg"
cp "data/nbfc-linux.desktop" "Appdir/nbfc-linux.desktop"

./appimagetool-x86_64.AppImage AppDIr