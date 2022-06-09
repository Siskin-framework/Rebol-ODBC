//
// ODBC Rebol extension
// ====================================
// Use on your own risc!

#include <stdlib.h>
#include <math.h>
#include "reb-host.h"
#include "host-lib.h"
#include <stdio.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#define USE_TRACES
#ifdef  USE_TRACES
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

#define DOUBLE_BUFFER_SIZE 16
#define ARG_BUFFER_SIZE    8

extern SQLHENV hEnv;


typedef struct reb_odbc_context {
	SQLHDBC   hDbc;
	SQLCHAR  *connect;
	SQLHSTMT *hStmt;
	RETCODE   last_result_code;
} ODBC_CONTEXT;

typedef struct reb_odbc_stmt {
	HSTMT     stmt;
	SQLCHAR  *text;
	REBINT    length;
	REBINT    last_result_code;
} ODBC_STMT;

void PrintStmtInfo (SQLHSTMT *hStmt, RETCODE code);

SQLCHAR* ODBC_StringToSqlChar   (REBSER   *source, int *length);
int      ODBC_UnCamelCase       (SQLWCHAR *source, SQLWCHAR *target);
REBSER*  ODBC_SqlWCharToString  (SQLWCHAR *source);
REBSER*  ODBC_SqlBinaryToBinary (char     *source, int length);


extern u32* words_odbc_cmd;
extern u32* words_odbc_arg;


extern REBDEC doubles[DOUBLE_BUFFER_SIZE];
extern RXIARG arg[ARG_BUFFER_SIZE];


//==============================================================//
// Some useful defines                                          //
//==============================================================//



#define RETURN_STR_ERROR(str) do {RXA_SERIES(frm, 1) = str; return RXR_ERROR;} while(0);


#define RESOLVE_UTF8_STRING(n, i) \
	n = RXA_SERIES(frm, i);        \
	n = RL_ENCODE_UTF8_STRING(SERIES_DATA(n), SERIES_TAIL(n), SERIES_WIDE(n) > 1, FALSE);

#define RESOLVE_UCS_STRING(n, i) \
	n = RXA_SERIES(frm, i);        \
	n = RL_ENCODE_UTF8_STRING(SERIES_DATA(n), SERIES_TAIL(n), SERIES_WIDE(n) > 1, FALSE);

#define RESOLVE_ODBC_CTX(n, i)                     \
			hob = RXA_HANDLE(frm, i);               \
			n = (ODBC_CONTEXT*)hob->data;         \
			if(!n || hob->sym != Handle_ODBC_DB )  \
				RETURN_STR_ERROR("Invalid ODBC DB handle!");

#define RESOLVE_ODBC_STMT(n, i)                   \
			hobStmt = RXA_HANDLE(frm, i);           \
			n = (ODBC_STMT*)hobStmt->data;        \
			if(!n || hobStmt->sym != Handle_ODBC_STMT || !(n)->stmt) \
				RETURN_STR_ERROR("Invalid ODBC STMT handle!");



void HandleDiagnosticRecord (SQLHANDLE      hHandle,
                             SQLSMALLINT    hType,
                             RETCODE        RetCode);

//*******************************************
//* Macro to call ODBC functions and        *
//* report an error on failure.             *
//* Takes handle, handle type, and stmt     *
//*******************************************

#define TRYODBC(h, ht, x)   {   RETCODE rc = x;\
                                if (rc != SQL_SUCCESS) \
                                { \
                                    HandleDiagnosticRecord (h, ht, rc); \
                                } \
                                if (rc == SQL_ERROR) \
                                { \
                                    fwprintf(stderr, L"Error in " L#x L"\n"); \
                                    goto error;  \
                                }  \
                            }

//******************************************/
//* Structure to store information about   */
//* a column.
//******************************************/

typedef struct STR_BINDING {
    SQLSMALLINT         cDisplaySize;           /* size to display  */
    WCHAR               *wszBuffer;             /* display buffer   */
    SQLLEN              indPtr;                 /* size or null     */
    BOOL                fChar;                  /* character col?   */
    struct STR_BINDING  *sNext;                 /* linked list      */
} BINDING;



//******************************************/
//* Forward references                     */
//******************************************/

void HandleDiagnosticRecord (SQLHANDLE      hHandle,
                 SQLSMALLINT    hType,
                 RETCODE        RetCode);

void DisplayResults(HSTMT       hStmt,
                    SQLSMALLINT cCols);

void AllocateBindings(HSTMT         hStmt,
                      SQLSMALLINT   cCols,
                      BINDING**     ppBinding,
                      SQLSMALLINT*  pDisplay);


void DisplayTitles(HSTMT    hStmt,
                   DWORD    cDisplaySize,
                   BINDING* pBinding);

void SetConsole(DWORD   cDisplaySize,
                BOOL    fInvert);

//*****************************************/
//* Some constants                        */
//*****************************************/


#define DISPLAY_MAX 50          // Arbitrary limit on column width to display
#define DISPLAY_FORMAT_EXTRA 3  // Per column extra display bytes (| <data> )
#define DISPLAY_FORMAT      L"%c %*.*s "
#define DISPLAY_FORMAT_C    L"%c %-*.*s "
#define NULL_SIZE           6   // <NULL>
#define SQL_QUERY_SIZE      1000 // Max. Num characters for SQL Query passed in.

#define PIPE                L'|'


                            