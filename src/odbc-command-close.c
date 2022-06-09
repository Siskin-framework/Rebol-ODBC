//
// Test Rebol extension
// ====================================
// Use on your own risc!

#include "odbc-rebol-extension.h"

int cmd_odbc_close(RXIFRM* frm, void* reb_ctx) {
	REBHOB       *hob;
	ODBC_CONTEXT *ctx;

	RESOLVE_ODBC_CTX(ctx, 1);
	if(ctx && ctx->hDbc) {
		SQLDisconnect(ctx->hDbc);
		SQLFreeHandle(SQL_HANDLE_DBC, ctx->hDbc);
		ctx->hDbc = NULL;
		trace("Connection disconnected.");
	}
	return RXR_UNSET;
}
