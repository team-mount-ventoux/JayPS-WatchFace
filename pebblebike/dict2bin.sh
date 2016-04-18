#!/bin/bash

python dict2bin.py  locale_english.json
python dict2bin.py  locale_french.json
python dict2bin.py  locale_spanish.json
python dict2bin.py  locale_german.json

mv *.bin resources/

git status local* resources/*