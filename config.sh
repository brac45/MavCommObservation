#!/bin/bash

# Interface for the MAVlink messaging experiment

##### Configuration file
DB_FILE="database.db"
TEMP_FILE="/tmp/sqltemp"
# Directories
WIFI_CLIENT="mavlink-wifi-client"
WIFI_SERVER="mavlink-wifi-server"
SERIAL_CLIENT="mavlink-serial-client"
SERIAL_SERVER="mavlink-serial-server"
BLUE_CLIENT="mavlink-bluetooth-client"
BLUE_SERVER="mavlink-bluetooth-server"
# Executables
WIFI_EXE_C="wifi-client.out"
WIFI_EXE_S="wifi-server.out"
SERIAL_EXE_C="serial-client.out"
SERIAL_EXE_S="serial-server.out"
BLUE_EXE_C="bluetooth-client.out"
BLUE_EXE_S="bluetooth-server.out"

# Wifi related constants
IP_ADDR="192.168.42.1"
PORT=9090

# Serial related constants
SERIAL_DEVICE_C="/dev/ttyUSB0"
SERIAL_DEVICE_S="/dev/ttyUSB0"

# Bluetooth related constants
DEST_MAC_ADDR="00:1A:7D:DA:71:05"

# SQL QUERIES
DB_SESSION_TABLE="CREATE TABLE IF NOT EXISTS session (\
	id INTEGER NOT NULL, \
	date_time TEXT NOT NULL, \
	description TEXT NOT NULL, \
	PRIMARY KEY (id));"
DB_RECORDS_TABLE="CREATE TABLE IF NOT EXISTS records (\
	session_id INTEGER NOT NULL, \
	frame_seq INTEGER NOT NULL, \
	frame_contents TEXT NOT NULL, \
	time_sent TEXT NOT NULL, \
	msg_size INTEGER NOT NULL, \
	rtt REAL NOT NULL, \
	uplink_time REAL NOT NULL, \
	downlink_time REAL NOT NULL, \
	PRIMARY KEY (session_id, frame_seq), \
	FOREIGN KEY (session_id) REFERENCES session(id));"
DB_SESSION_CLEAR="DELETE FROM session;"
DB_RECORDS_CLEAR="DELETE FROM records;"
