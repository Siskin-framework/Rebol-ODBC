//
// Test Rebol extension
// ====================================
// Use on your own risc!

#include "odbc-rebol-extension.h"

int cmd_odbc_prepare(RXIFRM* frm, void* reb_ctx) {
	REBHOB       *hob;
	REBHOB       *hobStmt;
	REBSER       *sql;
	ODBC_CONTEXT *ctx;
	ODBC_STMT    *ctxStmt;
	SQLHDBC      *hDbc;

	RESOLVE_ODBC_CTX(ctx, 1);
	sql = RXA_SERIES(frm, 2);

	hDbc = ctx->hDbc;

	debug_print("prep SQL: %s\n", SERIES_TEXT(sql));

	hobStmt = RL_MAKE_HANDLE_CONTEXT(Handle_ODBC_STMT);
	ctxStmt = (ODBC_STMT*)hobStmt->data;
	ctxStmt->text = ODBC_StringToSqlChar(sql, &ctxStmt->length);

	TRYODBC(hEnv,
		SQL_HANDLE_ENV,
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &ctxStmt->stmt)
	);

	TRYODBC(ctxStmt->stmt,
		SQL_HANDLE_STMT,
		SQLPrepare(ctxStmt->stmt, ctxStmt->text, ctxStmt->length)
	); 


	hobStmt->flags |= HANDLE_CONTEXT; //@@ temp fix!
	RXA_HANDLE      (frm, 1) = hobStmt;
	RXA_HANDLE_TYPE (frm, 1) = hobStmt->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hobStmt->flags;

	return RXR_VALUE;
error:
	return RXR_FALSE;
}
