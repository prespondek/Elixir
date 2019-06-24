# Elixir

The repositry contains code from the cross platform game "Food of the Gods" for iOS and Android. This repository contains none of the frontend code and is purely for demonstration purposes only 

<img src=https://i.imgur.com/XbTg2rG.png width=200 align=left>
<img src=https://i.imgur.com/rIkMalf.png width=200 align=left>
<img src=https://i.imgur.com/Uj0x8sj.png width=200 align=left>
<img src=https://i.imgur.com/BObcQiR.png width=200>

## Core Game Data
[Class/data](https://github.com/prespondek/Elixir/tree/master/Classes/data)
This folder contains the core game logic for Food of the God's. The "Match 3" game logic is in here. This is all my own code.

#### TableGraph.h/TableGraph.cpp
This controls all of the whole table of blocks

#### TableNode.h/TableNode.cpp
This represents the containers on the board which the blocks fall into.

#### TableBlock.h/TableBlock.cpp
Code for the falling blocks. Mainly contains data. 

## C++ Bindings
[Classes/external](https://github.com/prespondek/Elixir/tree/master/Classes/external)

## Objective C Wrappers
[proj.ios_mac/ios](https://github.com/prespondek/Elixir/tree/master/proj.ios_mac/ios)

## JNI Bindings
[proj.android/app/jni](https://github.com/prespondek/Elixir/tree/master/proj.android/app/jni)

## Java Wrappers
[proj.android/app/src/com/bridge](https://github.com/prespondek/Elixir/tree/master/proj.android/app/src/com/bridge)
