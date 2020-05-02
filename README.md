# ![](https://github.com/vasyahuyasa/remod-sauerbraten/blob/master/src/res/remod.png) Remod [![Build Status](https://travis-ci.org/vasyahuyasa/remod-sauerbraten.svg?branch=master)](https://travis-ci.org/vasyahuyasa/remod-sauerbraten)

Lightweight crossplatform sauerbraten server mod.

The goal of project is create lightweight crossplatform (FreeBSD, Linux, Windows and others) mod for replacement of official sauerbraten server.

Nightly builds (Linux and win32 packages) available here: http://remod.butchers.su/.

## Download

  * Latest development version: https://github.com/vasyahuyasa/remod-sauerbraten/releases
  * GIT repository: https://github.com/vasyahuyasa/remod-sauerbraten
 
## Docker

https://hub.docker.com/r/vasyahuyasa/remod

## Installation notes and some guides
```
git clone https://github.com/vasyahuyasa/remod-sauerbraten.git
cd remod-sauerbraten
git submodule update --init --recursive
cd src
make
```
  * Check our [wiki page](https://github.com/vasyahuyasa/remod-sauerbraten/wiki/Installation)

## Update GeoIP database

Remod uses GeoLite2 Country database. Because of new policy of Maxmind 

## Implemented features
  * [x] server side cubescript
  * [x] remote control, access to serverside cubescript (via tcp, udp and netcat)
  * [x] new cubescript functions
  * [x] GeoIP support
  * [x] IRC bot
  * [x] cubescript events
  * [x] user #commands
  * [x] irc bot user commands
  * [x] save and load maps to local file system in coopedit mode
  * [x] database (sqlite3, mysql)
  * [x] docker

## Work in progress
  * [ ] users system
  * [ ] scoreboard
  * [ ] anticheat
  * [ ] docker comose / documentation

## Supported platforms
  * Mac (cmake, Xcode)
  * FreeBSD 
  * Linux
  * Windows

## Contacts
  * [degrave](https://github.com/vasyahuyasa), [^o_o^](https://github.com/rmhmlhr)
  * IRC: irc://irc.gamesurge.net/rb
