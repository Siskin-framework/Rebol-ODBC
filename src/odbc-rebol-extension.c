//
// test Rebol extension
// ====================================
// Use on your own risc!

#include "odbc-rebol-extension.h"

RL_LIB *RL; // Link back to reb-lib from embedded extensions

//==== Globals ===============================//
u32*   words_odbc_cmd;
u32*   words_odbc_arg;
REBCNT Handle_ODBC_DB;
REBCNT Handle_ODBC_STMT;

REBDEC doubles[DOUBLE_BUFFER_SIZE];
RXIARG arg[ARG_BUFFER_SIZE];

SQLHENV hEnv = NULL;
//============================================//

static const char* init_block = EXT_ODBC_INIT_CODE;


void* releaseODBC_DB_Handle(void* hndl) {
	ODBC_CONTEXT *ctx = (ODBC_CONTEXT*)hndl;
//	debug_print("releasing odbc db: %p\n", ctx->db);
//	if(ctx->db) odbc_close((odbc*)ctx->db);
	return NULL;
}
void* releaseODBC_STMT_Handle(void* hndl) {
	ODBC_STMT *ctx = (ODBC_STMT*)hndl;
//	debug_print("releasing odbc stmt: %p\n", ctx->stmt);
//	if(ctx->stmt) odbc_finalize((odbc_stmt*)ctx->stmt);
	return NULL;
}


RXIEXT const char *RX_Init(int opts, RL_LIB *lib) {
    RL = lib;
	REBYTE ver[8];
	RL_VERSION(ver);

	debug_print("RXinit odbc-extension; Rebol v%i.%i.%i\n", ver[1], ver[2], ver[3]);

	if (MIN_REBOL_VERSION > VERSION(ver[1], ver[2], ver[3])) {
		debug_print("Needs at least Rebol v%i.%i.%i!\n", MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD);
		return 0;
	}
    if (!CHECK_STRUCT_ALIGN) {
    	trace("CHECK_STRUCT_ALIGN failed!");
    	return 0;
    }
	Handle_ODBC_DB   = RL_REGISTER_HANDLE((REBYTE*)"odbc-db", sizeof(ODBC_CONTEXT), releaseODBC_DB_Handle);
	Handle_ODBC_STMT = RL_REGISTER_HANDLE((REBYTE*)"odbc-stmt", sizeof(ODBC_STMT), releaseODBC_STMT_Handle);

//	odbc_initialize();
    return init_block;
}


RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
	switch (cmd) {
//	case CMD_ODBC_INFO:    return cmd_odbc_info(frm, ctx);
	case CMD_ODBC_OPEN:    return cmd_odbc_open(frm, ctx);
	case CMD_ODBC_EXEC:    return cmd_odbc_exec(frm, ctx);
//	case CMD_ODBC_FINALIZE:return cmd_odbc_finalize(frm, ctx);
	case CMD_ODBC_CLOSE:   return cmd_odbc_close(frm, ctx);
	case CMD_ODBC_PREPARE: return cmd_odbc_prepare(frm, ctx);
//	case CMD_ODBC_RESET:   return cmd_odbc_reset(frm, ctx);
	case CMD_ODBC_STEP:    return cmd_odbc_step(frm, ctx);
//	case CMD_ODBC_TRACE:   return cmd_odbc_trace(frm, ctx);
//
//	case CMD_ODBC_INITIALIZE: return cmd_odbc_initialize(frm, ctx);
//	case CMD_ODBC_SHUTDOWN: return cmd_odbc_shutdown(frm, ctx);
//
	case CMD_ODBC_INIT_WORDS:
		words_odbc_cmd = RL_MAP_WORDS(RXA_SERIES(frm,1));
		words_odbc_arg = RL_MAP_WORDS(RXA_SERIES(frm,2));
		return RXR_UNSET;
	} // end of commands

	return RXR_ERROR; //@@ FIXME!! This is not correct!
}

RXIEXT int RX_Quit(int opts) {
	trace("ODBC extension shutdown.");
	if (hEnv != NULL) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    return 0;
}


