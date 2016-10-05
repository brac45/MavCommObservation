// MESSAGE TEST_FRAME PACKING

#define MAVLINK_MSG_ID_TEST_FRAME 150

MAVPACKED(
typedef struct __mavlink_test_frame_t {
 uint64_t timestamp_sender; /*< Timestamp of the frame when it was created. Counted in milliseconds since Epoch.*/
 uint64_t timestamp_echo; /*< Timestamp of the frame when it was received and echoed back. Counted in milliseconds since Epoch.*/
 uint32_t sequence; /*< Frame sequence number.*/
}) mavlink_test_frame_t;

#define MAVLINK_MSG_ID_TEST_FRAME_LEN 20
#define MAVLINK_MSG_ID_TEST_FRAME_MIN_LEN 20
#define MAVLINK_MSG_ID_150_LEN 20
#define MAVLINK_MSG_ID_150_MIN_LEN 20

#define MAVLINK_MSG_ID_TEST_FRAME_CRC 248
#define MAVLINK_MSG_ID_150_CRC 248



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_TEST_FRAME { \
	150, \
	"TEST_FRAME", \
	3, \
	{  { "timestamp_sender", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_test_frame_t, timestamp_sender) }, \
         { "timestamp_echo", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_test_frame_t, timestamp_echo) }, \
         { "sequence", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_test_frame_t, sequence) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_TEST_FRAME { \
	"TEST_FRAME", \
	3, \
	{  { "timestamp_sender", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_test_frame_t, timestamp_sender) }, \
         { "timestamp_echo", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_test_frame_t, timestamp_echo) }, \
         { "sequence", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_test_frame_t, sequence) }, \
         } \
}
#endif

/**
 * @brief Pack a test_frame message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param sequence Frame sequence number.
 * @param timestamp_sender Timestamp of the frame when it was created. Counted in milliseconds since Epoch.
 * @param timestamp_echo Timestamp of the frame when it was received and echoed back. Counted in milliseconds since Epoch.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_test_frame_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint32_t sequence, uint64_t timestamp_sender, uint64_t timestamp_echo)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_TEST_FRAME_LEN];
	_mav_put_uint64_t(buf, 0, timestamp_sender);
	_mav_put_uint64_t(buf, 8, timestamp_echo);
	_mav_put_uint32_t(buf, 16, sequence);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_TEST_FRAME_LEN);
#else
	mavlink_test_frame_t packet;
	packet.timestamp_sender = timestamp_sender;
	packet.timestamp_echo = timestamp_echo;
	packet.sequence = sequence;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_TEST_FRAME_LEN);
#endif

	msg->msgid = MAVLINK_MSG_ID_TEST_FRAME;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_TEST_FRAME_MIN_LEN, MAVLINK_MSG_ID_TEST_FRAME_LEN, MAVLINK_MSG_ID_TEST_FRAME_CRC);
}

/**
 * @brief Pack a test_frame message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param sequence Frame sequence number.
 * @param timestamp_sender Timestamp of the frame when it was created. Counted in milliseconds since Epoch.
 * @param timestamp_echo Timestamp of the frame when it was received and echoed back. Counted in milliseconds since Epoch.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_test_frame_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint32_t sequence,uint64_t timestamp_sender,uint64_t timestamp_echo)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_TEST_FRAME_LEN];
	_mav_put_uint64_t(buf, 0, timestamp_sender);
	_mav_put_uint64_t(buf, 8, timestamp_echo);
	_mav_put_uint32_t(buf, 16, sequence);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_TEST_FRAME_LEN);
#else
	mavlink_test_frame_t packet;
	packet.timestamp_sender = timestamp_sender;
	packet.timestamp_echo = timestamp_echo;
	packet.sequence = sequence;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_TEST_FRAME_LEN);
#endif

	msg->msgid = MAVLINK_MSG_ID_TEST_FRAME;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_TEST_FRAME_MIN_LEN, MAVLINK_MSG_ID_TEST_FRAME_LEN, MAVLINK_MSG_ID_TEST_FRAME_CRC);
}

/**
 * @brief Encode a test_frame struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param test_frame C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_test_frame_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_test_frame_t* test_frame)
{
	return mavlink_msg_test_frame_pack(system_id, component_id, msg, test_frame->sequence, test_frame->timestamp_sender, test_frame->timestamp_echo);
}

/**
 * @brief Encode a test_frame struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param test_frame C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_test_frame_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_test_frame_t* test_frame)
{
	return mavlink_msg_test_frame_pack_chan(system_id, component_id, chan, msg, test_frame->sequence, test_frame->timestamp_sender, test_frame->timestamp_echo);
}

/**
 * @brief Send a test_frame message
 * @param chan MAVLink channel to send the message
 *
 * @param sequence Frame sequence number.
 * @param timestamp_sender Timestamp of the frame when it was created. Counted in milliseconds since Epoch.
 * @param timestamp_echo Timestamp of the frame when it was received and echoed back. Counted in milliseconds since Epoch.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_test_frame_send(mavlink_channel_t chan, uint32_t sequence, uint64_t timestamp_sender, uint64_t timestamp_echo)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_TEST_FRAME_LEN];
	_mav_put_uint64_t(buf, 0, timestamp_sender);
	_mav_put_uint64_t(buf, 8, timestamp_echo);
	_mav_put_uint32_t(buf, 16, sequence);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TEST_FRAME, buf, MAVLINK_MSG_ID_TEST_FRAME_MIN_LEN, MAVLINK_MSG_ID_TEST_FRAME_LEN, MAVLINK_MSG_ID_TEST_FRAME_CRC);
#else
	mavlink_test_frame_t packet;
	packet.timestamp_sender = timestamp_sender;
	packet.timestamp_echo = timestamp_echo;
	packet.sequence = sequence;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TEST_FRAME, (const char *)&packet, MAVLINK_MSG_ID_TEST_FRAME_MIN_LEN, MAVLINK_MSG_ID_TEST_FRAME_LEN, MAVLINK_MSG_ID_TEST_FRAME_CRC);
#endif
}

/**
 * @brief Send a test_frame message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_test_frame_send_struct(mavlink_channel_t chan, const mavlink_test_frame_t* test_frame)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_test_frame_send(chan, test_frame->sequence, test_frame->timestamp_sender, test_frame->timestamp_echo);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TEST_FRAME, (const char *)test_frame, MAVLINK_MSG_ID_TEST_FRAME_MIN_LEN, MAVLINK_MSG_ID_TEST_FRAME_LEN, MAVLINK_MSG_ID_TEST_FRAME_CRC);
#endif
}

#if MAVLINK_MSG_ID_TEST_FRAME_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_test_frame_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t sequence, uint64_t timestamp_sender, uint64_t timestamp_echo)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char *buf = (char *)msgbuf;
	_mav_put_uint64_t(buf, 0, timestamp_sender);
	_mav_put_uint64_t(buf, 8, timestamp_echo);
	_mav_put_uint32_t(buf, 16, sequence);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TEST_FRAME, buf, MAVLINK_MSG_ID_TEST_FRAME_MIN_LEN, MAVLINK_MSG_ID_TEST_FRAME_LEN, MAVLINK_MSG_ID_TEST_FRAME_CRC);
#else
	mavlink_test_frame_t *packet = (mavlink_test_frame_t *)msgbuf;
	packet->timestamp_sender = timestamp_sender;
	packet->timestamp_echo = timestamp_echo;
	packet->sequence = sequence;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TEST_FRAME, (const char *)packet, MAVLINK_MSG_ID_TEST_FRAME_MIN_LEN, MAVLINK_MSG_ID_TEST_FRAME_LEN, MAVLINK_MSG_ID_TEST_FRAME_CRC);
#endif
}
#endif

#endif

// MESSAGE TEST_FRAME UNPACKING


/**
 * @brief Get field sequence from test_frame message
 *
 * @return Frame sequence number.
 */
static inline uint32_t mavlink_msg_test_frame_get_sequence(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint32_t(msg,  16);
}

/**
 * @brief Get field timestamp_sender from test_frame message
 *
 * @return Timestamp of the frame when it was created. Counted in milliseconds since Epoch.
 */
static inline uint64_t mavlink_msg_test_frame_get_timestamp_sender(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field timestamp_echo from test_frame message
 *
 * @return Timestamp of the frame when it was received and echoed back. Counted in milliseconds since Epoch.
 */
static inline uint64_t mavlink_msg_test_frame_get_timestamp_echo(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Decode a test_frame message into a struct
 *
 * @param msg The message to decode
 * @param test_frame C-struct to decode the message contents into
 */
static inline void mavlink_msg_test_frame_decode(const mavlink_message_t* msg, mavlink_test_frame_t* test_frame)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	test_frame->timestamp_sender = mavlink_msg_test_frame_get_timestamp_sender(msg);
	test_frame->timestamp_echo = mavlink_msg_test_frame_get_timestamp_echo(msg);
	test_frame->sequence = mavlink_msg_test_frame_get_sequence(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_TEST_FRAME_LEN? msg->len : MAVLINK_MSG_ID_TEST_FRAME_LEN;
        memset(test_frame, 0, MAVLINK_MSG_ID_TEST_FRAME_LEN);
	memcpy(test_frame, _MAV_PAYLOAD(msg), len);
#endif
}
