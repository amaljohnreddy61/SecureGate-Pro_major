#ifndef __R305_DEFINES_H
#define __R305_DEFINES_H

#include "board_config.h"

//packet header (the module address comes from board_config.h)
#define R305_HEADER_HI   0xEF
#define R305_HEADER_LO   0x01
#define R305_ADDR_0      ((R305_ADDR >> 24) & 0xFF)
#define R305_ADDR_1      ((R305_ADDR >> 16) & 0xFF)
#define R305_ADDR_2      ((R305_ADDR >>  8) & 0xFF)
#define R305_ADDR_3      ( R305_ADDR        & 0xFF)

//package identifier (PID) values
#define R305_PID_COMMAND 0x01

//instruction codes
#define R305_INSTR_GEN_IMG   0x01   //collect finger image -> ImageBuffer
#define R305_INSTR_GEN_CHAR  0x02   //generate char file from image
#define R305_INSTR_STORE     0x06   //store template to flash library
#define R305_INSTR_SEARCH    0x04   //search whole library for match
#define R305_INSTR_DELETE    0x0C   //delete N templates from library
#define R305_INSTR_EMPTY     0x0D   //empty the whole fingerprint library

//response packet layout (simple ack: 12 bytes total)
#define R305_RESP_LEN         12
#define R305_RESP_LEN_SEARCH  16
#define R305_CONFIRM_IDX      9    //confirmation/ack code byte
#define R305_OK               0x00 //confirmation code: command executed OK

//search-command fixed parameters (search whole library, pages 0-99)
#define R305_SEARCH_START_PAGE 0x0000
#define R305_SEARCH_PAGE_NUM   0x0064

//delete-command fixed parameter (delete a single template)
#define R305_DELETE_COUNT      1

//extra instruction codes / results
#define R305_INSTR_REG_MODEL 0x05   //merge CharBuffer1+2 into one template
#define R305_INSTR_VFY_PWD   0x13   //handshake / verify module password
#define R305_NO_FINGER       0x02   //GenImg: no finger on the sensor
#define R305_TIMEOUT         0xFF   //our own code: module did not answer
#define R305_XFER_TIMEOUT_MS 1500

#endif
