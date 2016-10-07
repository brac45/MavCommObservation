#!/bin/bash

##### Constants
CONFIG_SH="config.sh"
FUNCTIONS_S="shfuncts.sh"

##### Main
# Source config script and functions
. $CONFIG_SH
. $FUNCTIONS_S
# Check for configuration mode
if [ $1 = "--configure" ]; then
	echo "Entering config mode"
	while :
	do
		echo "======"
		echo "OPTIONS"
		echo "	clear_records: Clears data from records table"
		echo "	clear_sessions: Clears data from session table"
		echo " 	clear_db: Clears all data from database"
		echo "	exit: Exit script"
		echo "======"
		echo -n "What should I do? > "
		read user_input
		case $user_input in
			"clear_records" )
				clear_records
				;;
			"clear_sessions" )
				clear_sessions
				;;
			"clear_db" )
				clear_db
				;;
			"exit" )
				echo "Exiting.."
				exit 0
				;;
			* )
				echo "What??"
				;;
		esac
	done
else
	# Argument check
	if [ $# -ne 3 ]; then
		usage
		exit 1
	fi
	if [ $2 != "-m" ]; then
		usage
		exit 1
	fi
	# Initialise database file and prompt for session description
	if [ $1 == "-c" ]; then
		init_db
		IFS= read -r -p "Enter description: " description_var
		date_var=`date +%Y-%m-%d:%H:%M:%S`
		id_var=`sqlite3 $DB_FILE "SELECT MAX(id) AS id_var FROM session"`
		let "id_var++"
		db_insert_session "$id_var" "$date_var" "$description_var"
	fi

	# Main procedure 
	case $3 in
		"433" )
			echo "433MHz sik radio selected!"
			start_433 $1 $id_var
			;;
		"915" )
			echo "915MHz sik radio selected!"
			start_915 $1 $id_var
			;;
		"wifi" )
			echo "wifi selected!"
			start_wifi $1 $id_var
			;;
		"bluetooth" )
			echo "bluetooth selected!"
			start_bluetooth $1 $id_var
			;;
		* )
			usage
			exit 1
			;;
	esac
fi

exit 0
