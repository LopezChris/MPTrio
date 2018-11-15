/// DBC file: ../_can_dbc/243.dbc    Self node: 'DRIVER'
/// This file can be included by a source file, for example: #include "generated.h"
#ifndef __GENEARTED_DBC_PARSER
#define __GENERATED_DBC_PARSER
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>



/// Extern function needed for dbc_encode_and_send()
extern bool dbc_app_send_can_msg(uint32_t mid, uint8_t dlc, uint8_t bytes[8]);

/// Missing in Action structure
typedef struct {
    uint32_t is_mia : 1;          ///< Missing in action flag
    uint32_t mia_counter_ms : 31; ///< Missing in action counter
} dbc_mia_info_t;

/// CAN message header structure
typedef struct { 
    uint32_t mid; ///< Message ID of the message
    uint8_t  dlc; ///< Data length of the message
} dbc_msg_hdr_t; 

static const dbc_msg_hdr_t DRIVER_HEARTBEAT_HDR =                 {  100, 1 };
static const dbc_msg_hdr_t IO_DEBUG_HDR =                         {  500, 4 };
static const dbc_msg_hdr_t MOTOR_CMD_HDR =                        {  101, 1 };
static const dbc_msg_hdr_t MOTOR_STATUS_HDR =                     {  400, 3 };
static const dbc_msg_hdr_t SENSOR_SONARS_HDR =                    {  200, 8 };

/// Enumeration(s) for Message: 'DRIVER_HEARTBEAT' from 'DRIVER'
typedef enum {
    DRIVER_HEARTBEAT_cmd_REBOOT = 2,
    DRIVER_HEARTBEAT_cmd_SYNC = 1,
    DRIVER_HEARTBEAT_cmd_NOOP = 0,
} DRIVER_HEARTBEAT_cmd_E ;

/// Enumeration(s) for Message: 'IO_DEBUG' from 'IO'
typedef enum {
    IO_DEBUG_test2_enum_two = 2,
    IO_DEBUG_test2_enum_one = 1,
} IO_DEBUG_test_enum_E ;




/// Message: DRIVER_HEARTBEAT from 'DRIVER', DLC: 1 byte(s), MID: 100
typedef struct {
    DRIVER_HEARTBEAT_cmd_E DRIVER_HEARTBEAT_cmd; ///< B7:0   Destination: SENSOR,MOTOR

    dbc_mia_info_t mia_info;
} DRIVER_HEARTBEAT_t;


/// Message: IO_DEBUG from 'IO', DLC: 4 byte(s), MID: 500
typedef struct {
    uint8_t IO_DEBUG_test_unsigned1;          ///< B7:0   Destination: DBG
    uint8_t IO_DEBUG_test_unsigned_minmax;    ///< B15:8  Min: 0 Max: 100   Destination: DBG
    int8_t IO_DEBUG_test_signed;              ///< B23:16   Destination: DBG
    int8_t IO_DEBUG_test_signed_minmax;       ///< B31:24   Destination: DBG
    IO_DEBUG_test_enum_E IO_DEBUG_test_enum;  ///< B39:32   Destination: DBG
    float IO_DEBUG_test_float;                ///< B47:40   Destination: DBG
    float IO_DEBUG_test_float_signed;         ///< B63:48  Min: -3276 Max: 3276   Destination: DBG

    dbc_mia_info_t mia_info;
} IO_DEBUG_t;


/// Message: MOTOR_CMD from 'DRIVER', DLC: 1 byte(s), MID: 101
typedef struct {
    int8_t MOTOR_CMD_steer : 4;               ///< B3:0  Min: -5 Max: 5   Destination: MOTOR
    uint8_t MOTOR_CMD_drive : 4;              ///< B7:4  Min: 0 Max: 9   Destination: MOTOR

    dbc_mia_info_t mia_info;
} MOTOR_CMD_t;


/// Message: MOTOR_STATUS from 'MOTOR', DLC: 3 byte(s), MID: 400
typedef struct {
    uint8_t MOTOR_STATUS_wheel_error : 1;     ///< B0:0   Destination: DRIVER,IO
    float MOTOR_STATUS_speed_kph;             ///< B23:8   Destination: DRIVER,IO

    dbc_mia_info_t mia_info;
} MOTOR_STATUS_t;

/// @{ MUX'd message: SENSOR_SONARS

/// Struct for MUX: m0 (used for transmitting)
typedef struct {
    uint16_t SENSOR_SONARS_err_count;         ///< B15:4   Destination: DRIVER,IO
    float SENSOR_SONARS_left;                 ///< B27:16   Destination: DRIVER,IO
    float SENSOR_SONARS_middle;               ///< B39:28   Destination: DRIVER,IO
    float SENSOR_SONARS_right;                ///< B51:40   Destination: DRIVER,IO
    float SENSOR_SONARS_rear;                 ///< B63:52   Destination: DRIVER,IO

    dbc_mia_info_t mia_info;
} SENSOR_SONARS_m0_t;

/// Struct for MUX: m1 (used for transmitting)
typedef struct {
    uint16_t SENSOR_SONARS_err_count;         ///< B15:4   Destination: DRIVER,IO
    float SENSOR_SONARS_no_filt_left;         ///< B27:16   Destination: DBG
    float SENSOR_SONARS_no_filt_middle;       ///< B39:28   Destination: DBG
    float SENSOR_SONARS_no_filt_right;        ///< B51:40   Destination: DBG
    float SENSOR_SONARS_no_filt_rear;         ///< B63:52   Destination: DBG

    dbc_mia_info_t mia_info;
} SENSOR_SONARS_m1_t;

/// Struct with all the child MUX'd signals (Used for receiving)
typedef struct {
    SENSOR_SONARS_m0_t m0; ///< MUX'd structure
    SENSOR_SONARS_m1_t m1; ///< MUX'd structure
} SENSOR_SONARS_t;
/// @} MUX'd message


/// @{ These 'externs' need to be defined in a source file of your project
extern const uint32_t                             DRIVER_HEARTBEAT__MIA_MS;
extern const DRIVER_HEARTBEAT_t                   DRIVER_HEARTBEAT__MIA_MSG;
extern const uint32_t                             IO_DEBUG__MIA_MS;
extern const IO_DEBUG_t                           IO_DEBUG__MIA_MSG;
extern const uint32_t                             MOTOR_CMD__MIA_MS;
extern const MOTOR_CMD_t                          MOTOR_CMD__MIA_MSG;
extern const uint32_t                             MOTOR_STATUS__MIA_MS;
extern const MOTOR_STATUS_t                       MOTOR_STATUS__MIA_MSG;
extern const uint32_t                             SENSOR_SONARS_m0__MIA_MS;
extern const SENSOR_SONARS_m0_t                   SENSOR_SONARS_m0__MIA_MSG;
extern const uint32_t                             SENSOR_SONARS_m1__MIA_MS;
extern const SENSOR_SONARS_m1_t                   SENSOR_SONARS_m1__MIA_MSG;
/// @}


/// Encode DRIVER's 'DRIVER_HEARTBEAT' message
/// @returns the message header of this message
static inline dbc_msg_hdr_t dbc_encode_DRIVER_HEARTBEAT(uint8_t bytes[8], DRIVER_HEARTBEAT_t *from)
{
    uint32_t raw;
    bytes[0]=bytes[1]=bytes[2]=bytes[3]=bytes[4]=bytes[5]=bytes[6]=bytes[7]=0;

    raw = ((uint32_t)(((from->DRIVER_HEARTBEAT_cmd)))) & 0xff;
    bytes[0] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B0

    return DRIVER_HEARTBEAT_HDR;
}

/// Encode and send for dbc_encode_DRIVER_HEARTBEAT() message
static inline bool dbc_encode_and_send_DRIVER_HEARTBEAT(DRIVER_HEARTBEAT_t *from)
{
    uint8_t bytes[8];
    const dbc_msg_hdr_t hdr = dbc_encode_DRIVER_HEARTBEAT(bytes, from);
    return dbc_app_send_can_msg(hdr.mid, hdr.dlc, bytes);
}



/// Encode IO's 'IO_DEBUG' message
/// @returns the message header of this message
static inline dbc_msg_hdr_t dbc_encode_IO_DEBUG(uint8_t bytes[8], IO_DEBUG_t *from)
{
    uint32_t raw;
    bytes[0]=bytes[1]=bytes[2]=bytes[3]=bytes[4]=bytes[5]=bytes[6]=bytes[7]=0;

    raw = ((uint32_t)(((from->IO_DEBUG_test_unsigned1)))) & 0xff;
    bytes[0] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B0

    // Not doing min value check since the signal is unsigned already
    if(from->IO_DEBUG_test_unsigned_minmax > 100) { from->IO_DEBUG_test_unsigned_minmax = 100; } // Max value: 100
    raw = ((uint32_t)(((from->IO_DEBUG_test_unsigned_minmax)))) & 0xff;
    bytes[1] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B8

    raw = ((uint32_t)(((from->IO_DEBUG_test_signed - (-128))))) & 0xff;
    bytes[2] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B16

    raw = ((uint32_t)(((from->IO_DEBUG_test_signed_minmax - (-128))))) & 0xff;
    bytes[3] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B24

    raw = ((uint32_t)(((from->IO_DEBUG_test_enum)))) & 0xff;
    bytes[4] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B32

    raw = ((uint32_t)(((from->IO_DEBUG_test_float) / 0.5) + 0.5)) & 0xff;
    bytes[5] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B40

    if(from->IO_DEBUG_test_float_signed < -3276) { from->IO_DEBUG_test_float_signed = -3276; } // Min value: -3276
    if(from->IO_DEBUG_test_float_signed > 3276) { from->IO_DEBUG_test_float_signed = 3276; } // Max value: 3276
    raw = ((uint32_t)(((from->IO_DEBUG_test_float_signed - (-3276)) / 0.1) + 0.5)) & 0xffff;
    bytes[6] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B48
    bytes[7] |= (((uint8_t)(raw >> 8) & 0xff)); ///< 8 bit(s) starting from B56

    return IO_DEBUG_HDR;
}

/// Encode and send for dbc_encode_IO_DEBUG() message
static inline bool dbc_encode_and_send_IO_DEBUG(IO_DEBUG_t *from)
{
    uint8_t bytes[8];
    const dbc_msg_hdr_t hdr = dbc_encode_IO_DEBUG(bytes, from);
    return dbc_app_send_can_msg(hdr.mid, hdr.dlc, bytes);
}



/// Encode DRIVER's 'MOTOR_CMD' message
/// @returns the message header of this message
static inline dbc_msg_hdr_t dbc_encode_MOTOR_CMD(uint8_t bytes[8], MOTOR_CMD_t *from)
{
    uint32_t raw;
    bytes[0]=bytes[1]=bytes[2]=bytes[3]=bytes[4]=bytes[5]=bytes[6]=bytes[7]=0;

    if(from->MOTOR_CMD_steer < -5) { from->MOTOR_CMD_steer = -5; } // Min value: -5
    if(from->MOTOR_CMD_steer > 5) { from->MOTOR_CMD_steer = 5; } // Max value: 5
    raw = ((uint32_t)(((from->MOTOR_CMD_steer - (-5))))) & 0x0f;
    bytes[0] |= (((uint8_t)(raw) & 0x0f)); ///< 4 bit(s) starting from B0

    // Not doing min value check since the signal is unsigned already
    if(from->MOTOR_CMD_drive > 9) { from->MOTOR_CMD_drive = 9; } // Max value: 9
    raw = ((uint32_t)(((from->MOTOR_CMD_drive)))) & 0x0f;
    bytes[0] |= (((uint8_t)(raw) & 0x0f) << 4); ///< 4 bit(s) starting from B4

    return MOTOR_CMD_HDR;
}

/// Encode and send for dbc_encode_MOTOR_CMD() message
static inline bool dbc_encode_and_send_MOTOR_CMD(MOTOR_CMD_t *from)
{
    uint8_t bytes[8];
    const dbc_msg_hdr_t hdr = dbc_encode_MOTOR_CMD(bytes, from);
    return dbc_app_send_can_msg(hdr.mid, hdr.dlc, bytes);
}



/// Encode MOTOR's 'MOTOR_STATUS' message
/// @returns the message header of this message
static inline dbc_msg_hdr_t dbc_encode_MOTOR_STATUS(uint8_t bytes[8], MOTOR_STATUS_t *from)
{
    uint32_t raw;
    bytes[0]=bytes[1]=bytes[2]=bytes[3]=bytes[4]=bytes[5]=bytes[6]=bytes[7]=0;

    raw = ((uint32_t)(((from->MOTOR_STATUS_wheel_error)))) & 0x01;
    bytes[0] |= (((uint8_t)(raw) & 0x01)); ///< 1 bit(s) starting from B0

    raw = ((uint32_t)(((from->MOTOR_STATUS_speed_kph) / 0.001) + 0.5)) & 0xffff;
    bytes[1] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B8
    bytes[2] |= (((uint8_t)(raw >> 8) & 0xff)); ///< 8 bit(s) starting from B16

    return MOTOR_STATUS_HDR;
}

/// Encode and send for dbc_encode_MOTOR_STATUS() message
static inline bool dbc_encode_and_send_MOTOR_STATUS(MOTOR_STATUS_t *from)
{
    uint8_t bytes[8];
    const dbc_msg_hdr_t hdr = dbc_encode_MOTOR_STATUS(bytes, from);
    return dbc_app_send_can_msg(hdr.mid, hdr.dlc, bytes);
}



/// Encode SENSOR's 'SENSOR_SONARS' MUX(m0) message
/// @returns the message header of this message
static inline dbc_msg_hdr_t dbc_encode_SENSOR_SONARS_m0(uint8_t bytes[8], SENSOR_SONARS_m0_t *from)
{
    uint32_t raw;
    bytes[0]=bytes[1]=bytes[2]=bytes[3]=bytes[4]=bytes[5]=bytes[6]=bytes[7]=0;

    // Set the MUX index value
    raw = ((uint32_t)(((0)))) & 0x0f;
    bytes[0] |= (((uint8_t)(raw) & 0x0f)); ///< 4 bit(s) starting from B0

    // Set non MUX'd signals that need to go out with this MUX'd message
    raw = ((uint32_t)(((from->SENSOR_SONARS_err_count)))) & 0xfff;
    bytes[0] |= (((uint8_t)(raw) & 0x0f) << 4); ///< 4 bit(s) starting from B4
    bytes[1] |= (((uint8_t)(raw >> 4) & 0xff)); ///< 8 bit(s) starting from B8

    // Set the rest of the signals within this MUX (m0)
    raw = ((uint32_t)(((from->SENSOR_SONARS_left) / 0.1) + 0.5)) & 0xfff;
    bytes[2] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B16
    bytes[3] |= (((uint8_t)(raw >> 8) & 0x0f)); ///< 4 bit(s) starting from B24
    raw = ((uint32_t)(((from->SENSOR_SONARS_middle) / 0.1) + 0.5)) & 0xfff;
    bytes[3] |= (((uint8_t)(raw) & 0x0f) << 4); ///< 4 bit(s) starting from B28
    bytes[4] |= (((uint8_t)(raw >> 4) & 0xff)); ///< 8 bit(s) starting from B32
    raw = ((uint32_t)(((from->SENSOR_SONARS_right) / 0.1) + 0.5)) & 0xfff;
    bytes[5] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B40
    bytes[6] |= (((uint8_t)(raw >> 8) & 0x0f)); ///< 4 bit(s) starting from B48
    raw = ((uint32_t)(((from->SENSOR_SONARS_rear) / 0.1) + 0.5)) & 0xfff;
    bytes[6] |= (((uint8_t)(raw) & 0x0f) << 4); ///< 4 bit(s) starting from B52
    bytes[7] |= (((uint8_t)(raw >> 4) & 0xff)); ///< 8 bit(s) starting from B56

    return SENSOR_SONARS_HDR;
}

/// Encode and send for dbc_encode_SENSOR_SONARS_m0() message
static inline bool dbc_encode_and_send_SENSOR_SONARS_m0(SENSOR_SONARS_m0_t *from)
{
    uint8_t bytes[8];
    const dbc_msg_hdr_t hdr = dbc_encode_SENSOR_SONARS_m0(bytes, from);
    return dbc_app_send_can_msg(hdr.mid, hdr.dlc, bytes);
}


/// Encode SENSOR's 'SENSOR_SONARS' MUX(m1) message
/// @returns the message header of this message
static inline dbc_msg_hdr_t dbc_encode_SENSOR_SONARS_m1(uint8_t bytes[8], SENSOR_SONARS_m1_t *from)
{
    uint32_t raw;
    bytes[0]=bytes[1]=bytes[2]=bytes[3]=bytes[4]=bytes[5]=bytes[6]=bytes[7]=0;

    // Set the MUX index value
    raw = ((uint32_t)(((1)))) & 0x0f;
    bytes[0] |= (((uint8_t)(raw) & 0x0f)); ///< 4 bit(s) starting from B0

    // Set non MUX'd signals that need to go out with this MUX'd message
    raw = ((uint32_t)(((from->SENSOR_SONARS_err_count)))) & 0xfff;
    bytes[0] |= (((uint8_t)(raw) & 0x0f) << 4); ///< 4 bit(s) starting from B4
    bytes[1] |= (((uint8_t)(raw >> 4) & 0xff)); ///< 8 bit(s) starting from B8

    // Set the rest of the signals within this MUX (m1)
    raw = ((uint32_t)(((from->SENSOR_SONARS_no_filt_left) / 0.1) + 0.5)) & 0xfff;
    bytes[2] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B16
    bytes[3] |= (((uint8_t)(raw >> 8) & 0x0f)); ///< 4 bit(s) starting from B24
    raw = ((uint32_t)(((from->SENSOR_SONARS_no_filt_middle) / 0.1) + 0.5)) & 0xfff;
    bytes[3] |= (((uint8_t)(raw) & 0x0f) << 4); ///< 4 bit(s) starting from B28
    bytes[4] |= (((uint8_t)(raw >> 4) & 0xff)); ///< 8 bit(s) starting from B32
    raw = ((uint32_t)(((from->SENSOR_SONARS_no_filt_right) / 0.1) + 0.5)) & 0xfff;
    bytes[5] |= (((uint8_t)(raw) & 0xff)); ///< 8 bit(s) starting from B40
    bytes[6] |= (((uint8_t)(raw >> 8) & 0x0f)); ///< 4 bit(s) starting from B48
    raw = ((uint32_t)(((from->SENSOR_SONARS_no_filt_rear) / 0.1) + 0.5)) & 0xfff;
    bytes[6] |= (((uint8_t)(raw) & 0x0f) << 4); ///< 4 bit(s) starting from B52
    bytes[7] |= (((uint8_t)(raw >> 4) & 0xff)); ///< 8 bit(s) starting from B56

    return SENSOR_SONARS_HDR;
}

/// Encode and send for dbc_encode_SENSOR_SONARS_m1() message
static inline bool dbc_encode_and_send_SENSOR_SONARS_m1(SENSOR_SONARS_m1_t *from)
{
    uint8_t bytes[8];
    const dbc_msg_hdr_t hdr = dbc_encode_SENSOR_SONARS_m1(bytes, from);
    return dbc_app_send_can_msg(hdr.mid, hdr.dlc, bytes);
}



/// Decode DRIVER's 'DRIVER_HEARTBEAT' message
/// @param hdr  The header of the message to validate its DLC and MID; this can be NULL to skip this check
static inline bool dbc_decode_DRIVER_HEARTBEAT(DRIVER_HEARTBEAT_t *to, const uint8_t bytes[8], const dbc_msg_hdr_t *hdr)
{
    const bool success = true;
    // If msg header is provided, check if the DLC and the MID match
    if (NULL != hdr && (hdr->dlc != DRIVER_HEARTBEAT_HDR.dlc || hdr->mid != DRIVER_HEARTBEAT_HDR.mid)) {
        return !success;
    }

    uint32_t raw;
    raw  = ((uint32_t)((bytes[0]) & 0xff)); ///< 8 bit(s) from B0
    to->DRIVER_HEARTBEAT_cmd = (DRIVER_HEARTBEAT_cmd_E)((raw));

    to->mia_info.mia_counter_ms = 0; ///< Reset the MIA counter

    return success;
}


/// Decode IO's 'IO_DEBUG' message
/// @param hdr  The header of the message to validate its DLC and MID; this can be NULL to skip this check
static inline bool dbc_decode_IO_DEBUG(IO_DEBUG_t *to, const uint8_t bytes[8], const dbc_msg_hdr_t *hdr)
{
    const bool success = true;
    // If msg header is provided, check if the DLC and the MID match
    if (NULL != hdr && (hdr->dlc != IO_DEBUG_HDR.dlc || hdr->mid != IO_DEBUG_HDR.mid)) {
        return !success;
    }

    uint32_t raw;
    raw  = ((uint32_t)((bytes[0]) & 0xff)); ///< 8 bit(s) from B0
    to->IO_DEBUG_test_unsigned1 = ((raw));
    raw  = ((uint32_t)((bytes[1]) & 0xff)); ///< 8 bit(s) from B8
    to->IO_DEBUG_test_unsigned_minmax = ((raw));
    raw  = ((uint32_t)((bytes[2]) & 0xff)); ///< 8 bit(s) from B16
    to->IO_DEBUG_test_signed = ((raw) + (-128));
    raw  = ((uint32_t)((bytes[3]) & 0xff)); ///< 8 bit(s) from B24
    to->IO_DEBUG_test_signed_minmax = ((raw) + (-128));
    raw  = ((uint32_t)((bytes[4]) & 0xff)); ///< 8 bit(s) from B32
    to->IO_DEBUG_test_enum = (IO_DEBUG_test_enum_E)((raw));
    raw  = ((uint32_t)((bytes[5]) & 0xff)); ///< 8 bit(s) from B40
    to->IO_DEBUG_test_float = ((raw * 0.5));
    raw  = ((uint32_t)((bytes[6]) & 0xff)); ///< 8 bit(s) from B48
    raw |= ((uint32_t)((bytes[7]) & 0xff)) << 8; ///< 8 bit(s) from B56
    to->IO_DEBUG_test_float_signed = ((raw * 0.1) + (-3276));

    to->mia_info.mia_counter_ms = 0; ///< Reset the MIA counter

    return success;
}


/// Decode DRIVER's 'MOTOR_CMD' message
/// @param hdr  The header of the message to validate its DLC and MID; this can be NULL to skip this check
static inline bool dbc_decode_MOTOR_CMD(MOTOR_CMD_t *to, const uint8_t bytes[8], const dbc_msg_hdr_t *hdr)
{
    const bool success = true;
    // If msg header is provided, check if the DLC and the MID match
    if (NULL != hdr && (hdr->dlc != MOTOR_CMD_HDR.dlc || hdr->mid != MOTOR_CMD_HDR.mid)) {
        return !success;
    }

    uint32_t raw;
    raw  = ((uint32_t)((bytes[0]) & 0x0f)); ///< 4 bit(s) from B0
    to->MOTOR_CMD_steer = ((raw) + (-5));
    raw  = ((uint32_t)((bytes[0] >> 4) & 0x0f)); ///< 4 bit(s) from B4
    to->MOTOR_CMD_drive = ((raw));

    to->mia_info.mia_counter_ms = 0; ///< Reset the MIA counter

    return success;
}


/// Decode MOTOR's 'MOTOR_STATUS' message
/// @param hdr  The header of the message to validate its DLC and MID; this can be NULL to skip this check
static inline bool dbc_decode_MOTOR_STATUS(MOTOR_STATUS_t *to, const uint8_t bytes[8], const dbc_msg_hdr_t *hdr)
{
    const bool success = true;
    // If msg header is provided, check if the DLC and the MID match
    if (NULL != hdr && (hdr->dlc != MOTOR_STATUS_HDR.dlc || hdr->mid != MOTOR_STATUS_HDR.mid)) {
        return !success;
    }

    uint32_t raw;
    raw  = ((uint32_t)((bytes[0]) & 0x01)); ///< 1 bit(s) from B0
    to->MOTOR_STATUS_wheel_error = ((raw));
    raw  = ((uint32_t)((bytes[1]) & 0xff)); ///< 8 bit(s) from B8
    raw |= ((uint32_t)((bytes[2]) & 0xff)) << 8; ///< 8 bit(s) from B16
    to->MOTOR_STATUS_speed_kph = ((raw * 0.001));

    to->mia_info.mia_counter_ms = 0; ///< Reset the MIA counter

    return success;
}


/// Decode SENSOR's 'SENSOR_SONARS' message
/// @param hdr  The header of the message to validate its DLC and MID; this can be NULL to skip this check
static inline bool dbc_decode_SENSOR_SONARS(SENSOR_SONARS_t *to, const uint8_t bytes[8], const dbc_msg_hdr_t *hdr)
{
    const bool success = true;
    // If msg header is provided, check if the DLC and the MID match
    if (NULL != hdr && (hdr->dlc != SENSOR_SONARS_HDR.dlc || hdr->mid != SENSOR_SONARS_HDR.mid)) {
        return !success;
    }

    uint32_t raw;
    // Decode the MUX
    raw  = ((uint32_t)((bytes[0]) & 0x0f)); ///< 4 bit(s) from B0
    const uint8_t MUX = ((raw));

    if (0 == MUX) {
        // Non Muxed signals (part of all MUX'd structures)
        raw  = ((uint32_t)((bytes[0] >> 4) & 0x0f)); ///< 4 bit(s) from B4
        raw |= ((uint32_t)((bytes[1]) & 0xff)) << 4; ///< 8 bit(s) from B8
        to->m0.SENSOR_SONARS_err_count = ((raw));

        raw  = ((uint32_t)((bytes[2]) & 0xff)); ///< 8 bit(s) from B16
        raw |= ((uint32_t)((bytes[3]) & 0x0f)) << 8; ///< 4 bit(s) from B24
        to->m0.SENSOR_SONARS_left = ((raw * 0.1));
        raw  = ((uint32_t)((bytes[3] >> 4) & 0x0f)); ///< 4 bit(s) from B28
        raw |= ((uint32_t)((bytes[4]) & 0xff)) << 4; ///< 8 bit(s) from B32
        to->m0.SENSOR_SONARS_middle = ((raw * 0.1));
        raw  = ((uint32_t)((bytes[5]) & 0xff)); ///< 8 bit(s) from B40
        raw |= ((uint32_t)((bytes[6]) & 0x0f)) << 8; ///< 4 bit(s) from B48
        to->m0.SENSOR_SONARS_right = ((raw * 0.1));
        raw  = ((uint32_t)((bytes[6] >> 4) & 0x0f)); ///< 4 bit(s) from B52
        raw |= ((uint32_t)((bytes[7]) & 0xff)) << 4; ///< 8 bit(s) from B56
        to->m0.SENSOR_SONARS_rear = ((raw * 0.1));

        to->m0.mia_info.mia_counter_ms = 0; ///< Reset the MIA counter
    }
    else if (1 == MUX) {
        // Non Muxed signals (part of all MUX'd structures)
        raw  = ((uint32_t)((bytes[0] >> 4) & 0x0f)); ///< 4 bit(s) from B4
        raw |= ((uint32_t)((bytes[1]) & 0xff)) << 4; ///< 8 bit(s) from B8
        to->m1.SENSOR_SONARS_err_count = ((raw));

        raw  = ((uint32_t)((bytes[2]) & 0xff)); ///< 8 bit(s) from B16
        raw |= ((uint32_t)((bytes[3]) & 0x0f)) << 8; ///< 4 bit(s) from B24
        to->m1.SENSOR_SONARS_no_filt_left = ((raw * 0.1));
        raw  = ((uint32_t)((bytes[3] >> 4) & 0x0f)); ///< 4 bit(s) from B28
        raw |= ((uint32_t)((bytes[4]) & 0xff)) << 4; ///< 8 bit(s) from B32
        to->m1.SENSOR_SONARS_no_filt_middle = ((raw * 0.1));
        raw  = ((uint32_t)((bytes[5]) & 0xff)); ///< 8 bit(s) from B40
        raw |= ((uint32_t)((bytes[6]) & 0x0f)) << 8; ///< 4 bit(s) from B48
        to->m1.SENSOR_SONARS_no_filt_right = ((raw * 0.1));
        raw  = ((uint32_t)((bytes[6] >> 4) & 0x0f)); ///< 4 bit(s) from B52
        raw |= ((uint32_t)((bytes[7]) & 0xff)) << 4; ///< 8 bit(s) from B56
        to->m1.SENSOR_SONARS_no_filt_rear = ((raw * 0.1));

        to->m1.mia_info.mia_counter_ms = 0; ///< Reset the MIA counter
    }
    else {
        return !success;
    }

    return success;
}


/// Handle the MIA for DRIVER's DRIVER_HEARTBEAT message
/// @param   time_incr_ms  The time to increment the MIA counter with
/// @returns true if the MIA just occurred
/// @post    If the MIA counter reaches the MIA threshold, MIA struct will be copied to *msg
static inline bool dbc_handle_mia_DRIVER_HEARTBEAT(DRIVER_HEARTBEAT_t *msg, uint32_t time_incr_ms)
{
    bool mia_occurred = false;
    const dbc_mia_info_t old_mia = msg->mia_info;
    msg->mia_info.is_mia = (msg->mia_info.mia_counter_ms >= DRIVER_HEARTBEAT__MIA_MS);

    if (!msg->mia_info.is_mia) { // Not MIA yet, so keep incrementing the MIA counter
        msg->mia_info.mia_counter_ms += time_incr_ms;
    }
    else if(!old_mia.is_mia)   { // Previously not MIA, but it is MIA now
        // Copy MIA struct, then re-write the MIA counter and is_mia that is overwriten
        *msg = DRIVER_HEARTBEAT__MIA_MSG;
        msg->mia_info.mia_counter_ms = DRIVER_HEARTBEAT__MIA_MS;
        msg->mia_info.is_mia = true;
        mia_occurred = true;
    }

    return mia_occurred;
}

/// Handle the MIA for IO's IO_DEBUG message
/// @param   time_incr_ms  The time to increment the MIA counter with
/// @returns true if the MIA just occurred
/// @post    If the MIA counter reaches the MIA threshold, MIA struct will be copied to *msg
static inline bool dbc_handle_mia_IO_DEBUG(IO_DEBUG_t *msg, uint32_t time_incr_ms)
{
    bool mia_occurred = false;
    const dbc_mia_info_t old_mia = msg->mia_info;
    msg->mia_info.is_mia = (msg->mia_info.mia_counter_ms >= IO_DEBUG__MIA_MS);

    if (!msg->mia_info.is_mia) { // Not MIA yet, so keep incrementing the MIA counter
        msg->mia_info.mia_counter_ms += time_incr_ms;
    }
    else if(!old_mia.is_mia)   { // Previously not MIA, but it is MIA now
        // Copy MIA struct, then re-write the MIA counter and is_mia that is overwriten
        *msg = IO_DEBUG__MIA_MSG;
        msg->mia_info.mia_counter_ms = IO_DEBUG__MIA_MS;
        msg->mia_info.is_mia = true;
        mia_occurred = true;
    }

    return mia_occurred;
}

/// Handle the MIA for DRIVER's MOTOR_CMD message
/// @param   time_incr_ms  The time to increment the MIA counter with
/// @returns true if the MIA just occurred
/// @post    If the MIA counter reaches the MIA threshold, MIA struct will be copied to *msg
static inline bool dbc_handle_mia_MOTOR_CMD(MOTOR_CMD_t *msg, uint32_t time_incr_ms)
{
    bool mia_occurred = false;
    const dbc_mia_info_t old_mia = msg->mia_info;
    msg->mia_info.is_mia = (msg->mia_info.mia_counter_ms >= MOTOR_CMD__MIA_MS);

    if (!msg->mia_info.is_mia) { // Not MIA yet, so keep incrementing the MIA counter
        msg->mia_info.mia_counter_ms += time_incr_ms;
    }
    else if(!old_mia.is_mia)   { // Previously not MIA, but it is MIA now
        // Copy MIA struct, then re-write the MIA counter and is_mia that is overwriten
        *msg = MOTOR_CMD__MIA_MSG;
        msg->mia_info.mia_counter_ms = MOTOR_CMD__MIA_MS;
        msg->mia_info.is_mia = true;
        mia_occurred = true;
    }

    return mia_occurred;
}

/// Handle the MIA for MOTOR's MOTOR_STATUS message
/// @param   time_incr_ms  The time to increment the MIA counter with
/// @returns true if the MIA just occurred
/// @post    If the MIA counter reaches the MIA threshold, MIA struct will be copied to *msg
static inline bool dbc_handle_mia_MOTOR_STATUS(MOTOR_STATUS_t *msg, uint32_t time_incr_ms)
{
    bool mia_occurred = false;
    const dbc_mia_info_t old_mia = msg->mia_info;
    msg->mia_info.is_mia = (msg->mia_info.mia_counter_ms >= MOTOR_STATUS__MIA_MS);

    if (!msg->mia_info.is_mia) { // Not MIA yet, so keep incrementing the MIA counter
        msg->mia_info.mia_counter_ms += time_incr_ms;
    }
    else if(!old_mia.is_mia)   { // Previously not MIA, but it is MIA now
        // Copy MIA struct, then re-write the MIA counter and is_mia that is overwriten
        *msg = MOTOR_STATUS__MIA_MSG;
        msg->mia_info.mia_counter_ms = MOTOR_STATUS__MIA_MS;
        msg->mia_info.is_mia = true;
        mia_occurred = true;
    }

    return mia_occurred;
}

/// Handle the MIA for SENSOR's SENSOR_SONARS for MUX "m0" message
/// @param   time_incr_ms  The time to increment the MIA counter with
/// @returns true if the MIA just occurred
/// @post    If the MIA counter reaches the MIA threshold, MIA struct will be copied to *msg
static inline bool dbc_handle_mia_SENSOR_SONARS_m0(SENSOR_SONARS_m0_t *msg, uint32_t time_incr_ms)
{
    bool mia_occurred = false;
    const dbc_mia_info_t old_mia = msg->mia_info;
    msg->mia_info.is_mia = (msg->mia_info.mia_counter_ms >= SENSOR_SONARS_m0__MIA_MS);

    if (!msg->mia_info.is_mia) { // Not MIA yet, so keep incrementing the MIA counter
        msg->mia_info.mia_counter_ms += time_incr_ms;
    }
    else if(!old_mia.is_mia)   { // Previously not MIA, but it is MIA now
        // Copy MIA struct, then re-write the MIA counter and is_mia that is overwriten
        *msg = SENSOR_SONARS_m0__MIA_MSG;
        msg->mia_info.mia_counter_ms = SENSOR_SONARS_m0__MIA_MS;
        msg->mia_info.is_mia = true;
        mia_occurred = true;
    }

    return mia_occurred;
}

/// Handle the MIA for SENSOR's SENSOR_SONARS for MUX "m1" message
/// @param   time_incr_ms  The time to increment the MIA counter with
/// @returns true if the MIA just occurred
/// @post    If the MIA counter reaches the MIA threshold, MIA struct will be copied to *msg
static inline bool dbc_handle_mia_SENSOR_SONARS_m1(SENSOR_SONARS_m1_t *msg, uint32_t time_incr_ms)
{
    bool mia_occurred = false;
    const dbc_mia_info_t old_mia = msg->mia_info;
    msg->mia_info.is_mia = (msg->mia_info.mia_counter_ms >= SENSOR_SONARS_m1__MIA_MS);

    if (!msg->mia_info.is_mia) { // Not MIA yet, so keep incrementing the MIA counter
        msg->mia_info.mia_counter_ms += time_incr_ms;
    }
    else if(!old_mia.is_mia)   { // Previously not MIA, but it is MIA now
        // Copy MIA struct, then re-write the MIA counter and is_mia that is overwriten
        *msg = SENSOR_SONARS_m1__MIA_MSG;
        msg->mia_info.mia_counter_ms = SENSOR_SONARS_m1__MIA_MS;
        msg->mia_info.is_mia = true;
        mia_occurred = true;
    }

    return mia_occurred;
}

#endif
