//r305.c
#include "types.h"
#include "board_config.h"
#include "r305.h"
#include "r305_defines.h"
#include "uart0.h"
#include "uart1.h"
#include "delay.h"

u8  r305_uart = R305_UART;          /* picked by R305_AutoDetect() */
u32 r305_baud = R305_BAUD;
u32 r305_pass = R305_PASS;
u8  r305_pwd_ok = 0;
static u32 r305_timeout_ms = R305_XFER_TIMEOUT_MS;

static void R305_Flush(void)            { if (r305_uart == 0) U0_Flush();        else UART1_Flush(); }
static void R305_Send(u8 c)             { if (r305_uart == 0) U0_Tx(c);          else UART1_SendChar(c); }
static u8   R305_Read(u8 *p, u32 ms)    { return (r305_uart == 0) ? U0_ReadTimeout(p, ms) : UART1_ReadTimeout(p, ms); }


static u8 r305_xfer(const u8 *cmd, u8 cmd_len, u8 *resp, u8 resp_len)
{
	u8 i, ok = 1;

	R305_Flush();
	for (i = 0; i < cmd_len; i++)
		R305_Send(cmd[i]);
	for (i = 0; i < resp_len; i++) {
		if (!R305_Read(&resp[i], r305_timeout_ms)) { ok = 0; break; }
	}
	if (ok && (resp[0] != R305_HEADER_HI || resp[1] != R305_HEADER_LO))
		ok = 0;
	return ok ? resp[R305_CONFIRM_IDX] : R305_TIMEOUT;
}

static u8 r305_build(u8 *cmd, u8 len_field, const u8 *params, u8 nparams)
{
	u16 sum = R305_PID_COMMAND + len_field;
	u8 i, n = 0;
	cmd[n++] = R305_HEADER_HI;  cmd[n++] = R305_HEADER_LO;
	cmd[n++] = R305_ADDR_0;     cmd[n++] = R305_ADDR_1;
	cmd[n++] = R305_ADDR_2;     cmd[n++] = R305_ADDR_3;
	cmd[n++] = R305_PID_COMMAND;
	cmd[n++] = 0x00;            cmd[n++] = len_field;
	for (i = 0; i < nparams; i++) { cmd[n++] = params[i]; sum += params[i]; }
	cmd[n++] = (u8)(sum >> 8);
	cmd[n++] = (u8)(sum & 0xFF);
	return n;
}

static u8 r305_verify_pwd(u32 pw)
{
	u8 p[5];
	u8 cmd[9 + 5 + 2], resp[R305_RESP_LEN], n;
	p[0] = R305_INSTR_VFY_PWD;
	p[1] = (u8)(pw >> 24); p[2] = (u8)(pw >> 16); p[3] = (u8)(pw >> 8); p[4] = (u8)pw;
	n = r305_build(cmd, 0x07, p, 5);
	return r305_xfer(cmd, n, resp, R305_RESP_LEN);
}

u8 R305_CollectImage(void)
{
	u8 p[1] = {R305_INSTR_GEN_IMG};
	u8 cmd[9 + 1 + 2], resp[R305_RESP_LEN], n;
	n = r305_build(cmd, 0x03, p, 1);
	return r305_xfer(cmd, n, resp, R305_RESP_LEN);
}

u8 R305_GenerateChar(u8 buffer_id)
{
	u8 p[2] = {R305_INSTR_GEN_CHAR, 0};
	u8 cmd[9 + 2 + 2], resp[R305_RESP_LEN], n;
	p[1] = buffer_id;
	n = r305_build(cmd, 0x04, p, 2);
	return r305_xfer(cmd, n, resp, R305_RESP_LEN);
}

u8 R305_RegModel(void)
{
	u8 p[1] = {R305_INSTR_REG_MODEL};
	u8 cmd[9 + 1 + 2], resp[R305_RESP_LEN], n;
	n = r305_build(cmd, 0x03, p, 1);
	return r305_xfer(cmd, n, resp, R305_RESP_LEN);
}

u8 R305_StoreTemplate(u8 buffer_id, u16 page_id)
{
	u8 p[4];
	u8 cmd[9 + 4 + 2], resp[R305_RESP_LEN], n;
	p[0] = R305_INSTR_STORE; p[1] = buffer_id;
	p[2] = (u8)(page_id >> 8); p[3] = (u8)(page_id & 0xFF);
	n = r305_build(cmd, 0x06, p, 4);
	return r305_xfer(cmd, n, resp, R305_RESP_LEN);
}

s32 R305_SearchFinger(u8 buffer_id, u16 *matched_id)
{
	u8 p[6];
	u8 cmd[9 + 6 + 2], resp[R305_RESP_LEN_SEARCH], c, n;
	p[0] = R305_INSTR_SEARCH; p[1] = buffer_id;
	p[2] = (u8)(R305_SEARCH_START_PAGE >> 8); p[3] = (u8)(R305_SEARCH_START_PAGE & 0xFF);
	p[4] = (u8)(R305_SEARCH_PAGE_NUM >> 8);   p[5] = (u8)(R305_SEARCH_PAGE_NUM & 0xFF);
	n = r305_build(cmd, 0x08, p, 6);
	c = r305_xfer(cmd, n, resp, R305_RESP_LEN_SEARCH);
	if (c == R305_OK) {
		*matched_id = (u16)((resp[10] << 8) | resp[11]);
		return 0;
	}
	return -1;
}

u8 R305_DeleteTemplate(u16 page_id)
{
	u8 p[5];
	u8 cmd[9 + 5 + 2], resp[R305_RESP_LEN], n;
	p[0] = R305_INSTR_DELETE;
	p[1] = (u8)(page_id >> 8); p[2] = (u8)(page_id & 0xFF);
	p[3] = 0x00;               p[4] = R305_DELETE_COUNT;
	n = r305_build(cmd, 0x07, p, 5);
	return r305_xfer(cmd, n, resp, R305_RESP_LEN);
}

u8 R305_EmptyLibrary(void)
{
	u8 p[1] = {R305_INSTR_EMPTY};
	u8 cmd[9 + 1 + 2], resp[R305_RESP_LEN], n;
	n = r305_build(cmd, 0x03, p, 1);
	return r305_xfer(cmd, n, resp, R305_RESP_LEN);
}

u8 R305_WaitFinger(u32 timeout_ms)
{
	u8 c = R305_NO_FINGER;
	u32 waited = 0;
	while (waited < timeout_ms) {
		c = R305_CollectImage();
		if (c == R305_OK || c == R305_TIMEOUT) return c;   /* got image, or module is dead */
		delay_ms(100);
		waited += 100;
	}
	return R305_NO_FINGER;
}

u8 R305_WaitFingerRemoved(u32 timeout_ms)
{
	u8 c;
	u32 waited = 0;
	while (waited < timeout_ms) {
		c = R305_CollectImage();
		if (c == R305_NO_FINGER) return R305_OK;
		if (c == R305_TIMEOUT)   return R305_TIMEOUT;
		delay_ms(100);
		waited += 100;
	}
	return R305_NO_FINGER;
}

/* Probe UART0/UART1 at 9600 and 57600 (the configured guess first). Any reply, even an error code,
   means the module is there. The other UART is then set up for the PC log. */
u8 R305_AutoDetect(void)
{
	static const u32 bauds[2] = { 9600, 57600 };
	u8 uart, b, t, c, k;
	u8 first = R305_UART;

	r305_timeout_ms = R305_PROBE_MS;
	for (k = 0; k < 4; k++) {
		uart = (k < 2) ? first : (u8)(1 - first);
		b    = (u8)(k & 1);
		r305_uart = uart;
		r305_baud = bauds[b];
		if (uart == 0) U0_SetBaud(r305_baud); else UART1_Init(r305_baud);
		for (t = 0; t < 2; t++) {
			c = r305_verify_pwd(r305_pass);
			if (c != R305_TIMEOUT) goto found;
			delay_ms(100);
		}
	}
	
	r305_uart = R305_UART;
	r305_baud = R305_BAUD;
	r305_pwd_ok = 0;
	if (r305_uart == 0) { U0_SetBaud(r305_baud); UART1_Init(LOG_BAUD); }
	else                { UART1_Init(r305_baud); U0_SetBaud(LOG_BAUD); }
	r305_timeout_ms = R305_XFER_TIMEOUT_MS;
	return 0;
found:
	
	r305_pwd_ok = (c == R305_OK);
	if (!r305_pwd_ok) {
		static const u32 alt[3] = { R305_PASS, 0x00000000UL, 0xFFFFFFFFUL };
		for (k = 0; k < 3 && !r305_pwd_ok; k++) {
			if (r305_verify_pwd(alt[k]) == R305_OK) { r305_pass = alt[k]; r305_pwd_ok = 1; }
		}
	}
	if (r305_uart == 0) UART1_Init(LOG_BAUD); else U0_SetBaud(LOG_BAUD);
	r305_timeout_ms = R305_XFER_TIMEOUT_MS;
	return 1;
}
