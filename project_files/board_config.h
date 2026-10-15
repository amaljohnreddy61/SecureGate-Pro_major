/* board_config.h - every wiring-dependent choice in one place */
#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

/* ---- UART assignment ---- */
#define R305_UART        1
#define R305_BAUD        57600
#define LOG_BAUD         9600
#define R305_PROBE_MS    300        /* per-probe reply timeout during auto-detect */

/* ---- R305 identity ---- */
#define R305_ADDR        0xFFFFFFFFUL
#define R305_PASS        0x00000000UL

/* ---- LCD ---- */
#define LCD_DATA_PORT      1
#define LCD_DATA_BASE_PIN  24
#define LCD_RS_PIN         12       /* P0.12 */
#define LCD_EN_PIN         13       /* P0.13 */

/* ---- Door motor (L293D) ----*/
#define MOTOR_IN1          (1UL << 4)
#define MOTOR_IN2          (1UL << 5)
#define MOTOR_EN_ENABLED   1
#define MOTOR_EN           (1UL << 7)

/* ---- Door-closed limit switch */
#define DOOR_SWITCH_ENABLED 0
#define DOOR_SW             (1UL << 6)

#define DOOR_OPEN_MS     3000       /* forward run time                         */
#define DOOR_HOLD_MS     2000       /* door stays open                          */
#define DOOR_CLOSE_MS    3000       /* reverse run time (or time-out with switch) */
#define DOOR_DENY_MS     1500       /* reverse pulse on denied / locked access  */

/* ---- Fingerprint timing ---- */
#define FP_WAIT_MS       10000      /* how long to wait for a finger */

/* build-time safety: the LCD data bus must not sit on the UART1 pins */
#if (LCD_DATA_PORT == 0) && (LCD_DATA_BASE_PIN <= 9) && (LCD_DATA_BASE_PIN + 7 >= 8)
#error LCD data pins overlap P0.8/P0.9 (R305 UART1) - change LCD_DATA_PORT / LCD_DATA_BASE_PIN
#endif

#endif /* __BOARD_CONFIG_H */
