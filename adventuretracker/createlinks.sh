export PEBBLE_SDK_ROOT=/Applications/PebbleKit-v1.1/Pebble
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