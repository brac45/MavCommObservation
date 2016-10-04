#!/bin/bash

# Interface for the MAVlink messaging experiment

##### Configuration file

DB_FILE="database.db"
TEMP_FILE="/tmp/sqltemp"

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
