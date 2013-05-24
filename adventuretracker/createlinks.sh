: ${PEBBLE_SDK_ROOT?"Please set the environment variable PEBBLE_SDK_ROOT and point it to the Pebble folder where you extracted the Pebble SDK, i.e export PEBBLE_SDK_ROOT=/Applications/PebbleKit/Pebble"}

rm tools
rm lib
rm waf
rm pebble_app.ld
rm wscript
rm include
rm ./resources/wscript

ln -s $PEBBLE_SDK_ROOT/sdk/tools tools
ln -s $PEBBLE_SDK_ROOT/sdk/lib lib
ln -s $PEBBLE_SDK_ROOT/sdk/waf waf
ln -s $PEBBLE_SDK_ROOT/sdk/pebble_app.ld pebble_app.ld
ln -s $PEBBLE_SDK_ROOT/sdk/wscript wscript
ln -s $PEBBLE_SDK_ROOT/sdk/include include
ln -s $PEBBLE_SDK_ROOT/sdk/resources/wscript resources/wscript