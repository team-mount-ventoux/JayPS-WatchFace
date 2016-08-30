#!/bin/bash

python dict2bin.py  locale_english.json
python dict2bin.py  locale_french.json
python dict2bin.py  locale_spanish.json
python dict2bin.py  locale_german.json
python dict2bin.py  locale_italian.json
python dict2bin.py  locale_japanese.json

mv *.bin resources/

git status local* resources/*
