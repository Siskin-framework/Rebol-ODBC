//
// Test Rebol extension
// ====================================
// Use on your own risc!

#include "odbc-rebol-extension.h"

int cmd_odbc_initialize(RXIFRM* frm, void* reb_ctx) {
	int rc = sqlite3_initialize();
	//debug_print("initialize result: %i\n", rc);
	return (SQLITE_OK == rc ? RXR_TRUE : RXR_FALSE);
}
