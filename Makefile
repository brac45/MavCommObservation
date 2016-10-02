#
# Top parent directory Makefile for managing build process of the entire project
# GNU MAKE
#

# =========== Variables ================
# Directories
SERIAL_CLIENT_DIR := mavlink-serial-client
SERIAL_SERVER_DIR := mavlink-serial-server
BLUETOOTH_CLIENT_DIR := mavlink-bluetooth-client
BLUETOOTH_SERVER_DIR := mavlink-bluetooth-server
WIFI_CLIENT_DIR := mavlink-wifi-client
WIFI_SERVER_DIR := mavlink-wifi-server

all:
	@echo "Compiling all sub-projects"
	$(MAKE) -C $(SERIAL_CLIENT_DIR)
	$(MAKE) -C $(SERIAL_SERVER_DIR)
	$(MAKE) -C $(BLUETOOTH_CLIENT_DIR)
	$(MAKE) -C $(BLUETOOTH_SERVER_DIR)
	$(MAKE) -C $(WIFI_CLIENT_DIR)
	$(MAKE) -C $(WIFI_SERVER_DIR)

serial_client:
	@echo "Compiling serial client project.."
	$(MAKE) -C $(SERIAL_CLIENT_DIR)

serial_server:
	@echo "Compiling serial server project.."
	$(MAKE) -C $(SERIAL_SERVER_DIR)

clean:
	@echo "Cleaning all sub-projects"
	$(MAKE) -C $(SERIAL_CLIENT_DIR) clean
	$(MAKE) -C $(SERIAL_SERVER_DIR) clean
	$(MAKE) -C $(BLUETOOTH_CLIENT_DIR) clean
	$(MAKE) -C $(BLUETOOTH_SERVER_DIR) clean
	$(MAKE) -C $(WIFI_CLIENT_DIR) clean
	$(MAKE) -C $(WIFI_SERVER_DIR) clean
