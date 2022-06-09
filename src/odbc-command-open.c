//
// Test Rebol extension
// ====================================
// Use on your own risc!

#include "odbc-rebol-extension.h"

int cmd_odbc_open(RXIFRM* frm, void* reb_ctx) {
	REBHOB       *hob;
	SQLRETURN     rc;
	ODBC_CONTEXT *ctx;
	REBSER       *string;
	REBINT        length;


	// Allocate an environment
	if(hEnv == NULL) {
		rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
		if (rc == SQL_ERROR) {
			puts("Unable to allocate an environment handle");
			goto error;
		}
		// Register this as an application that expects 3.x behavior,
		// you must register something if you use AllocHandle
		TRYODBC(hEnv,
			SQL_HANDLE_ENV,
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0)
		);
	}

	hob = RL_MAKE_HANDLE_CONTEXT(Handle_ODBC_DB);
	if (!hob) goto error;
	ctx = (ODBC_CONTEXT*)hob->data;

	TRYODBC(hEnv,
		SQL_HANDLE_ENV,
		SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &ctx->hDbc)
	);


    // Connect to the driver.  Use the connection string if supplied
    // on the input, otherwise let the driver manager prompt for input.
    ctx->connect = ODBC_StringToSqlChar(RXA_SERIES(frm, 1), &length);


    TRYODBC(ctx->hDbc,
        SQL_HANDLE_DBC,
        SQLDriverConnect(ctx->hDbc,
                         NULL,
                         ctx->connect,
                         length,
                         NULL,
                         0,
                         NULL,
                         SQL_DRIVER_COMPLETE));
    trace("Connection established!");


	hob->flags |= HANDLE_CONTEXT; //@@ temp fix!
	RXA_HANDLE      (frm, 1) = hob;
	RXA_HANDLE_TYPE (frm, 1) = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE        (frm, 1) = RXT_HANDLE;

	return RXR_VALUE;

error:
	return RXR_NONE;
}
