//
// Test Rebol extension
// ====================================
// Use on your own risc!

#include "odbc-rebol-extension.h"

int cmd_odbc_step(RXIFRM* frm, void* reb_ctx) {
	REBHOB       *hob;
	REBHOB       *hobStmt;
	REBSER       *sql;
	ODBC_CONTEXT *ctx;
	ODBC_STMT    *ctxStmt;
	SQLHDBC      *hDbc;
	REBINT        hasRows;
	REBI64        rows;
	SQLSMALLINT   sColumns;

	RESOLVE_ODBC_CTX(ctx, 1);
	hasRows = RXA_REF(frm, 2);
	rows =  RXA_INT64(frm, 3);
	if (rows < 1) rows = 1;

	if (ctx->hStmt == NULL) return RXR_NONE;

	TRYODBC(ctx->hStmt,
			SQL_HANDLE_STMT,
			SQLNumResultCols(ctx->hStmt, &sColumns));
	
	if (sColumns == 0) return RXR_NONE;

	printf("Columns: %i\n", sColumns);



	return RXR_VALUE;
error:
	return RXR_FALSE;
}
