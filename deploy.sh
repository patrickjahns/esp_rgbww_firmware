#!/bin/bash
set -e # exit with nonzero exit code if anything fails
  if [ ! -z "$TRAVIS_TAG" ] && [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
    echo "New release - deploying"
  else
	echo "Not a release - skipping deploy!"
	exit 0;
  fi
GH_PAGE_LINK="http://patrickjahns.github.io/esp_rgbww_firmware/release"

cd $TRAVIS_BUILD_DIR

# prepare folders
mkdir -p $TRAVIS_BUILD_DIR/release
mkdir -p $TRAVIS_BUILD_DIR/webapp

# build webinterface
cd $TRAVIS_BUILD_DIR
git clone https://github.com/patrickjahns/esp_rgbww_webinterface
cd esp_rgbww_webinterface
npm run release


WEBAPP_VERSION=`cat VERSION`
echo $WEBAPPP_VERSION
rm -rf $TRAVIS_BUILD_DIR/webapp
mkdir -p $TRAVIS_BUILD_DIR/webapp
cp $TRAVIS_BUILD_DIR/esp_rgbww_webinterface/dist/* $TRAVIS_BUILD_DIR/webapp

# zip webinterface
cd $TRAVIS_BUILD_DIR/esp_rgbww_webinterface/dist/
zip -r esp_rgbww_webinterface.zip *

# rebuild rom and filesystems
cd $TRAVIS_BUILD_DIR
FW_VERSION=`cat VERSION`
make clean
make all

cp $TRAVIS_BUILD_DIR/out/firmware/* $TRAVIS_BUILD_DIR/release
cd $TRAVIS_BUILD_DIR/release

# create version information
cat <<EOF > version.json
{"rom":{"fw_version":"${FW_VERSION}","url":"${GH_PAGE_LINK}/rom0.bin"},"spiffs":{"webapp_version":"${WEBAPP_VERSION}","url":"${GH_PAGE_LINK}/spiff_rom.bin"}}
EOF

# create zipfile 
zip -r esp_rgbww_firmware.zip *


mkdir -p $TRAVIS_BUILD_DIR/_release
cd $TRAVIS_BUILD_DIR/_release 

#Push to github pages
git init
git remote add upstream "https://$GH_TOKEN@${GH_REF}"
git fetch upstream
git reset upstream/gh-pages
git config user.name "Patrick Jahns"
git config user.email "<github>@<patrickjahns.de>"


mv $TRAVIS_BUILD_DIR/release $TRAVIS_BUILD_DIR/_release/
mv $TRAVIS_BUILD_DIR/esp_rgbww_webinterface/dist/esp_rgbww_webinterface.zip $TRAVIS_BUILD_DIR/_release/release/esp_rgbww_webinterface.zip
touch .
git add .
git commit -m "Release Firmware v${FW_VERSION} webapp v${WEBAPP_VERSION}"
git push -q upstream HEAD:gh-pages