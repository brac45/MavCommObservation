##### Functions
db_insert_session () {
	echo "INSERT INTO session(id, date_time, description) VALUES($1, '$2', '$3')"
	sqlite3 $DB_FILE "INSERT INTO session(id, date_time, description) VALUES($1, '$2', '$3')"
	return
}

init_db () {
	if [ ! -f $DB_FILE ]; then
		echo "database.db not found!"
		echo "Creating new database file with predefined schema.."
		echo $DB_SESSION_TABLE > $TEMP_FILE
		echo $DB_RECORDS_TABLE >> $TEMP_FILE
		cat $TEMP_FILE
		sqlite3 $DB_FILE < $TEMP_FILE
		rm -f $TEMP_FILE
	else
		echo "Initialising DB.."
		echo $DB_SESSION_TABLE > $TEMP_FILE
		echo $DB_RECORDS_TABLE >> $TEMP_FILE
		cat $TEMP_FILE
		sqlite3 $DB_FILE < $TEMP_FILE
		rm -f $TEMP_FILE
	fi
}

clear_db () {
	echo "Clearing tables.."
	echo $DB_SESSION_CLEAR > $TEMP_FILE
	echo $DB_RECORDS_CLEAR >> $TEMP_FILE
	cat $TEMP_FILE
	sqlite3 $DB_FILE < $TEMP_FILE
	rm -f $TEMP_FILE
}

clear_records () {
	echo "Clearing records.."
	echo $DB_RECORDS_CLEAR > $TEMP_FILE
	cat $TEMP_FILE
	sqlite3 $DB_FILE < $TEMP_FILE
	rm -f $TEMP_FILE
}

clear_sessions () {
	echo "Clearing session.."
	echo $DB_SESSION_CLEAR > $TEMP_FILE
	cat $TEMP_FILE
	sqlite3 $DB_FILE < $TEMP_FILE
	rm -f $TEMP_FILE
}

usage () {
	echo "usage.."
	echo "./tester.sh [-c | -s | --configure] [-m communication type]"
	echo "	-c or -s: client-side or server-side, only one option may be used"
	echo "	--configure: go into config mode"
	echo "	-m: communication type(type in 433, 915, blue or bluetooth)"
	echo ""
}

start_serial () {
	case $1 in 
		"-c" )
			echo "Serial client starting.."
			if [ -f "$SERIAL_CLIENT/bin/$SERIAL_EXE_C" ]; then
				echo "Entering $SERIAL_CLIENT"
				cd $SERIAL_CLIENT
				./bin/$SERIAL_EXE_C $SERIAL_DEVICE_C ../$DB_FILE $2 
			else
				echo "Compile $SERIAL_EXE_C in $SERIAL_CLIENT"
			fi
			;;
		"-s" )
			echo "Serial server starting.."
			if [ -f "$SERIAL_SERVER/bin/$SERIAL_EXE_S" ]; then
				echo "Entering $SERIAL_SERVER"
				cd $SERIAL_SERVER
				./bin/$SERIAL_EXE_S $SERIAL_DEVICE_S
			else
				echo "Compile $SERIAL_EXE_S in $SERIAL_SERVER"
			fi
			;;
		* )
			usage
			exit 1
			;;
	esac
}

start_wifi () {
	case $1 in
		"-c" )
			echo "Wifi client side starting.."
			if [ -f "$WIFI_CLIENT/bin/$WIFI_EXE_C" ]; then
				echo "Entering $WIFI_CLIENT"
				cd $WIFI_CLIENT
				./bin/$WIFI_EXE_C ../$DB_FILE $2 -u $IP_ADDR $PORT
			else
				echo "Please compile $WIFI_EXE_C in $WIFI_CLIENT"
			fi
			;;
		"-s" )
			echo "Wifi server side starting.."
			if [ -f "$WIFI_SERVER/bin/$WIFI_EXE_S" ]; then
				echo "Entering $WIFI_SERVER"
				cd $WIFI_SERVER
				./bin/$WIFI_EXE_S -u $PORT
			else
				echo "Please compile $WIFI_EXE_S in $WIFI_SERVER"
			fi
			;;
		* )
			usage
			exit 1
			;;
	esac
}

start_bluetooth () {
	case $1 in
		"-c" )
			echo "Bluetooth client starting.."
			if [ -f "$BLUE_CLIENT/bin/$BLUE_EXE_C" ]; then
				echo "Entering $BLUE_CLIENT"
				cd $BLUE_CLIENT
				./bin/$BLUE_EXE_C ../$DB_FILE
			else
			fi
	esac
}

