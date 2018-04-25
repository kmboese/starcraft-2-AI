#!/bin/bash
echo "Creating maps directory..."
#Create maps directory
mkdir sc2-maps
cd sc2-maps
#Download SC2 Ladder maps
echo "Downloading SC2 ladder maps..."
wget http://blzdistsc2-a.akamaihd.net/MapPacks/Ladder2017Season1.zip
wget http://blzdistsc2-a.akamaihd.net/MapPacks/Ladder2017Season2.zip
wget http://blzdistsc2-a.akamaihd.net/MapPacks/Ladder2017Season3_Updated.zip
wget http://blzdistsc2-a.akamaihd.net/MapPacks/Ladder2017Season4.zip
wget http://blzdistsc2-a.akamaihd.net/MapPacks/Ladder2018Season1.zip

#Download SC2 Melee maps
echo "Downloading melee maps..."
wget http://blzdistsc2-a.akamaihd.net/MapPacks/Melee.zip

#Download PySC2 mini-game maps
echo "Downloading PySC2 mini-game maps"
wget https://github.com/deepmind/pysc2/releases/download/v1.2/mini_games.zip
