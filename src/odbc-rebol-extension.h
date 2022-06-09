// auto-generated file, do not modify! //

#include "odbc-command.h"

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 5
#define MIN_REBOL_UPD 4
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

extern REBCNT Handle_ODBC_DB;
extern REBCNT Handle_ODBC_STMT;

enum odbc_commands {
	CMD_ODBC_INIT_WORDS,
	CMD_ODBC_INFO,
	CMD_ODBC_OPEN,
	CMD_ODBC_EXEC,
	CMD_ODBC_FINALIZE,
	CMD_ODBC_TRACE,
	CMD_ODBC_PREPARE,
	CMD_ODBC_RESET,
	CMD_ODBC_STEP,
	CMD_ODBC_CLOSE,
	CMD_ODBC_INITIALIZE,
	CMD_ODBC_SHUTDOWN,
};
enum words_odbc_cmd {W_ODBC_CMD_0,
};
enum words_odbc_arg {W_ODBC_ARG_0,
};

int cmd_odbc_init_words(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_info(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_open(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_exec(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_finalize(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_trace(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_prepare(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_reset(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_step(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_close(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_initialize(RXIFRM* frm, void* reb_ctx);
int cmd_odbc_shutdown(RXIFRM* frm, void* reb_ctx);

#define EXT_ODBC_INIT_CODE \
	"REBOL [Title: \"Rebol ODBC Extension\" Name: odbc Type: module Exports: [] Version: 0.1.0 Author: Oldes Date: 11-Jun-2022/6:52:42+2:00 License: Apache-2.0 Url: https://github.com/Siskin-framework/Rebol-ODBC]\n"\
	"init-words: command [cmd-words [block!] arg-words [block!]]\n"\
	"info: command [\"Returns info about ODBC extension library\" /of handle [handle!] \"ODBC Extension handle\"]\n"\
	"open: command [\"Opens a new database connection\" conn [string!]]\n"\
	"exec: command [{Runs zero or more semicolon-separate SQL statements} db [handle!] \"odbc-db\" sql [string!] \"statements\"]\n"\
	"finalize: command [\"Deletes prepared statement\" stmt [handle!] \"odbc-stmt\"]\n"\
	"trace: command [\"Traces debug output\" db [handle!] \"odbc-db\" mask [integer!]]\n"\
	"prepare: command [\"Prepares SQL statement\" db [handle!] \"odbc-db\" sql [string!] \"statement\"]\n"\
	"reset: command [\"Resets prepared statement\" stmt [handle!] \"odbc-stmt\"]\n"\
	"step: command [\"Executes prepared statement\" stmt [handle!] \"odbc-stmt\" /rows {Multiple times if there is enough rows in the result} count [integer!] /with parameters [block!]]\n"\
	"close: command [\"Closes a database connection\" db [handle!] \"odbc-db\"]\n"\
	"initialize: command [\"Initializes the ODBC library\"]\n"\
	"shutdown: command [\"Deallocate any resources that were allocated\"]\n"\
	"init-words words: [] []\n"\
	"protect/hide 'init-words\n"\
