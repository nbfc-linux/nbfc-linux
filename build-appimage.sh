#!/bin/bash
make
mkdir AppDir
DESTDIR=AppDir make install

if [ ! -e "appimagetool-x86_64.AppImage" ] ; then
    wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" "appimagetool-x86_64.AppImage"
    chmod +x "appimagetool-x86_64.AppImage"
fi

#wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-x86_64"
#mv "AppRun-x86_64" "AppDir/AppRun"
#chmod +x "AppDir/AppRun"

cp "data/nbfc-linux.svg" "AppDir/nbfc-linux.svg"
cp "data/nbfc-linux.desktop" "AppDir/nbfc-linux.desktop"
#cp "data/run" "AppDir/usr/bin/run"
#chmod +x "AppDir/usr/bin/run"

cp "data/run" "AppDir/AppRun"
chmod +x "AppDir/AppRun"

./appimagetool-x86_64.AppImage AppDir
chmod +x nbfc-linux-*.AppImage