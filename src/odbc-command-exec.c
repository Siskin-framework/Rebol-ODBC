//
// Test Rebol extension
// ====================================
// Use on your own risc!

#include "odbc-rebol-extension.h"


int cmd_odbc_exec(RXIFRM* frm, void* reb_ctx) {
	REBHOB       *hob;
	REBSER       *sql;
	ODBC_CONTEXT *ctx;
	SQLCHAR      *text;
	SQLHSTMT     *hStmt;
	REBINT        len = 0;
	

	RESOLVE_ODBC_CTX(ctx, 1);
	sql = RXA_SERIES(frm, 2);

	debug_print("exec SQL: %s\n", (char*)SERIES_TEXT(sql));

	if(ctx->hStmt == NULL) {
		trace("Allocating internal statement.");
		TRYODBC(ctx->hDbc,
				SQL_HANDLE_DBC,
				SQLAllocHandle(SQL_HANDLE_STMT, ctx->hDbc, &ctx->hStmt));
	}
	hStmt = ctx->hStmt;

	text = ODBC_StringToSqlChar(sql, &len);
	ctx->last_result_code = SQLExecDirect(ctx->hStmt, text, len);
	free(text);

	PrintStmtInfo(ctx->hStmt, ctx->last_result_code);

//	TRYODBC(hStmt,
//            SQL_HANDLE_STMT,
//            SQLFreeStmt(hStmt, SQL_CLOSE));


	return RXR_TRUE;
error:
	return RXR_FALSE;
}
