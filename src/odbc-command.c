//
// SQLite Rebol extension
// ====================================
// Use on your own risc!

#include "odbc-command.h"

SHORT   gHeight = 80;       // Users screen height

/*******************************************************************************
**
*/  SQLCHAR* ODBC_StringToSqlChar(REBSER *source, int *length)
/*
*******************************************************************************/
{
	int i, len;
	SQLCHAR *target = NULL;

	len = RL_SERIES(source, RXI_SER_TAIL);

	//debug_print("malloc %i\n", len);
	target = malloc(sizeof(SQLCHAR) * (1 + len));
	if (target == NULL) return NULL;

	for (i = 0; i < len; i++) target[i] = RL_GET_CHAR(source, i);
	target[i] = 0;
	*length = len;
	return target;
}


/*******************************************************************************
**
*/	REBSER* ODBC_SqlWCharToString(SQLWCHAR *source)
/*
********************************************************************************/
{
	int     i, length = lstrlenW(source);
	REBSER *target = RL_MAKE_STRING(length, TRUE); // UTF-8 for REBOL3

	for (i = 0; i < length; i++) RL_SET_CHAR(target, i, source[i]);

	return target;
}


/*******************************************************************************
**
*/	REBSER* ODBC_SqlBinaryToBinary(char *source, int length)
/*
*******************************************************************************/
{
	int     i;
	REBSER *target = RL_MAKE_STRING(length, FALSE);

	for (i = 0; i < length; i++) RL_SET_CHAR(target, i, source[i]);

	return target;
}


/*******************************************************************************
**
*/	int ODBC_UnCamelCase(SQLWCHAR *source, SQLWCHAR *target)
/*
*******************************************************************************/
{
	int length = lstrlenW(source), s, t = 0;
	WCHAR *hyphen = L"-", *underscore = L"_", *space = L" ";

	for (s = 0; s < length; s++)
	{
		target[t++] = (source[s] == *underscore || source[s] == *space) ? *hyphen : towlower(source[s]);

		if (
			(s < length - 2 && iswupper(source[s]) && iswupper(source[s + 1]) && iswlower(source[s + 2])) ||
			(s < length - 1 && iswlower(source[s]) && iswupper(source[s + 1]))
		){
			target[t++] = *hyphen;
		}
	}
	target[t++] = 0;

	return t;
}


//************************************************************************
//* HandleDiagnosticRecord : display error/warning information
//*
//* Parameters:
//*      hHandle     ODBC handle
//*      hType       Type of handle (HANDLE_STMT, HANDLE_ENV, HANDLE_DBC)
//*      RetCode     Return code of failing command
//************************************************************************

void HandleDiagnosticRecord (SQLHANDLE      hHandle,
                             SQLSMALLINT    hType,
                             RETCODE        RetCode)
{
    SQLSMALLINT iRec = 0;
    SQLINTEGER  iError;
    WCHAR       wszMessage[1000];
    WCHAR       wszState[SQL_SQLSTATE_SIZE+1];


    if (RetCode == SQL_INVALID_HANDLE)
    {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }

    while (SQLGetDiagRecW(hType,
                         hHandle,
                         ++iRec,
                         wszState,
                         &iError,
                         wszMessage,
                         (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)),
                         (SQLSMALLINT *)NULL) == SQL_SUCCESS)
    {
        // Hide data truncated..
        if (wcsncmp(wszState, L"01004", 5))
        {
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
        }
    }
}

void PrintStmtInfo (SQLHSTMT *hStmt, RETCODE code) {
	SQLSMALLINT   sNumResults;

	switch(code) {
	case SQL_SUCCESS_WITH_INFO:
		HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, code);
		// fall through
	case SQL_SUCCESS:
		// If this is a row-returning query, display results
		TRYODBC(hStmt,
				SQL_HANDLE_STMT,
				SQLNumResultCols(hStmt,&sNumResults));

		if (sNumResults > 0)
		{
			printf("Columns: %i\n", sNumResults);
			DisplayResults(hStmt,sNumResults);
		}
		else
		{
			SQLLEN cRowCount;

			TRYODBC(hStmt,
				SQL_HANDLE_STMT,
				SQLRowCount(hStmt,&cRowCount));

			if (cRowCount >= 0)
			{
				debug_print("%Id %s affected.\n", cRowCount,cRowCount == 1 ? "row":"rows");
				//wprintf(L"%Id %s affected\n",
				//		 cRowCount,
				//		 cRowCount == 1 ? L"row" : L"rows");
			}
		}
		break;
	case SQL_ERROR:
		HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, code);
		break;
	default:
		fwprintf(stderr, L"Unexpected return code %hd!\n", code);
	}

error: // required by TRYODBC macro
	return;
}


/************************************************************************
/* DisplayResults: display results of a select query
/*
/* Parameters:
/*      hStmt      ODBC statement handle
/*      cCols      Count of columns
/************************************************************************/

void DisplayResults(HSTMT       hStmt,
                    SQLSMALLINT cCols)
{
    BINDING         *pFirstBinding, *pThisBinding;
    SQLSMALLINT     cDisplaySize;
    RETCODE         RetCode = SQL_SUCCESS;
    int             iCount = 0;

    // Allocate memory for each column

    AllocateBindings(hStmt, cCols, &pFirstBinding, &cDisplaySize);

    // Set the display mode and write the titles

    DisplayTitles(hStmt, cDisplaySize+1, pFirstBinding);


    // Fetch and display the data

    BOOL fNoData = FALSE;

    do {
        // Fetch a row

        if (iCount++ >= gHeight - 2)
        {
            int     nInputChar;
            BOOL    fEnterReceived = FALSE;

            while(!fEnterReceived)
            {
                wprintf(L"              ");
                SetConsole(cDisplaySize+2, TRUE);
                wprintf(L"   Press ENTER to continue, Q to quit (height:%hd)", gHeight);
                SetConsole(cDisplaySize+2, FALSE);

                nInputChar = _getch();
                wprintf(L"\n");
                if ((nInputChar == 'Q') || (nInputChar == 'q'))
                {
                    goto error;
                }
                else if ('\r' == nInputChar)
                {
                    fEnterReceived = TRUE;
                }
                // else loop back to display prompt again
            }

            iCount = 1;
            DisplayTitles(hStmt, cDisplaySize+1, pFirstBinding);
        }

        TRYODBC(hStmt, SQL_HANDLE_STMT, RetCode = SQLFetch(hStmt));

        if (RetCode == SQL_NO_DATA_FOUND)
        {
            fNoData = TRUE;
        }
        else
        {

            // Display the data.   Ignore truncations

            for (pThisBinding = pFirstBinding;
                pThisBinding;
                pThisBinding = pThisBinding->sNext)
            {
                if (pThisBinding->indPtr != SQL_NULL_DATA)
                {
                    wprintf(pThisBinding->fChar ? DISPLAY_FORMAT_C:DISPLAY_FORMAT,
                        PIPE,
                        pThisBinding->cDisplaySize,
                        pThisBinding->cDisplaySize,
                        pThisBinding->wszBuffer);
                }
                else
                {
                    wprintf(DISPLAY_FORMAT_C,
                        PIPE,
                        pThisBinding->cDisplaySize,
                        pThisBinding->cDisplaySize,
                        L"<NULL>");
                }
            }
            wprintf(L" %c\n",PIPE);
        }
    } while (!fNoData);

    SetConsole(cDisplaySize+2, TRUE);
    wprintf(L"%*.*s", cDisplaySize+2, cDisplaySize+2, L" ");
    SetConsole(cDisplaySize+2, FALSE);
    wprintf(L"\n");

error:
    // Clean up the allocated buffers

    while (pFirstBinding)
    {
        pThisBinding = pFirstBinding->sNext;
        free(pFirstBinding->wszBuffer);
        free(pFirstBinding);
        pFirstBinding = pThisBinding;
    }
}

//************************************************************************
//* AllocateBindings:  Get column information and allocate bindings
//* for each column.
//*
//* Parameters:
//*      hStmt      Statement handle
//*      cCols       Number of columns in the result set
//*      *lppBinding Binding pointer (returned)
//*      lpDisplay   Display size of one line
//************************************************************************/

void AllocateBindings(HSTMT         hStmt,
                      SQLSMALLINT   cCols,
                      BINDING       **ppBinding,
                      SQLSMALLINT   *pDisplay)
{
    SQLSMALLINT     iCol;
    BINDING         *pThisBinding, *pLastBinding = NULL;
    SQLLEN          cchDisplay, ssType;
    SQLSMALLINT     cchColumnNameLength;

    *pDisplay = 0;

    for (iCol = 1; iCol <= cCols; iCol++)
    {
        pThisBinding = (BINDING *)(malloc(sizeof(BINDING)));
        if (!(pThisBinding))
        {
            fwprintf(stderr, L"Out of memory!\n");
            exit(-100);
        }

        if (iCol == 1)
        {
            *ppBinding = pThisBinding;
        }
        else
        {
            pLastBinding->sNext = pThisBinding;
        }
        pLastBinding = pThisBinding;


        // Figure out the display length of the column (we will
        // bind to char since we are only displaying data, in general
        // you should bind to the appropriate C type if you are going
        // to manipulate data since it is much faster...)

        TRYODBC(hStmt,
                SQL_HANDLE_STMT,
                SQLColAttribute(hStmt,
                    iCol,
                    SQL_DESC_DISPLAY_SIZE,
                    NULL,
                    0,
                    NULL,
                    &cchDisplay));


        // Figure out if this is a character or numeric column; this is
        // used to determine if we want to display the data left- or right-
        // aligned.

        // SQL_DESC_CONCISE_TYPE maps to the 1.x SQL_COLUMN_TYPE.
        // This is what you must use if you want to work
        // against a 2.x driver.

        TRYODBC(hStmt,
                SQL_HANDLE_STMT,
                SQLColAttribute(hStmt,
                    iCol,
                    SQL_DESC_CONCISE_TYPE,
                    NULL,
                    0,
                    NULL,
                    &ssType));

        pThisBinding->fChar = (ssType == SQL_CHAR ||
                                ssType == SQL_VARCHAR ||
                                ssType == SQL_LONGVARCHAR);

        pThisBinding->sNext = NULL;

        // Arbitrary limit on display size
        if (cchDisplay > DISPLAY_MAX)
            cchDisplay = DISPLAY_MAX;

        // Allocate a buffer big enough to hold the text representation
        // of the data.  Add one character for the null terminator

        pThisBinding->wszBuffer = (WCHAR *)malloc((cchDisplay+1) * sizeof(WCHAR));

        if (!(pThisBinding->wszBuffer))
        {
            fwprintf(stderr, L"Out of memory!\n");
            exit(-100);
        }

        // Map this buffer to the driver's buffer.   At Fetch time,
        // the driver will fill in this data.  Note that the size is
        // count of bytes (for Unicode).  All ODBC functions that take
        // SQLPOINTER use count of bytes; all functions that take only
        // strings use count of characters.

        TRYODBC(hStmt,
                SQL_HANDLE_STMT,
                SQLBindCol(hStmt,
                    iCol,
                    SQL_C_TCHAR,
                    (SQLPOINTER) pThisBinding->wszBuffer,
                    (cchDisplay + 1) * sizeof(WCHAR),
                    &pThisBinding->indPtr));


        // Now set the display size that we will use to display
        // the data.   Figure out the length of the column name

        TRYODBC(hStmt,
                SQL_HANDLE_STMT,
                SQLColAttribute(hStmt,
                    iCol,
                    SQL_DESC_NAME,
                    NULL,
                    0,
                    &cchColumnNameLength,
                    NULL));

        pThisBinding->cDisplaySize = max((SQLSMALLINT)cchDisplay, cchColumnNameLength);
        if (pThisBinding->cDisplaySize < NULL_SIZE)
            pThisBinding->cDisplaySize = NULL_SIZE;

        *pDisplay += pThisBinding->cDisplaySize + DISPLAY_FORMAT_EXTRA;

    }

    return;

error:

    exit(-1);

    return;
}


//************************************************************************
//* DisplayTitles: print the titles of all the columns and set the
//*                shell window's width
//*
//* Parameters:
//*      hStmt          Statement handle
//*      cDisplaySize   Total display size
//*      pBinding        list of binding information
//************************************************************************/

void DisplayTitles(HSTMT     hStmt,
                   DWORD     cDisplaySize,
                   BINDING   *pBinding)
{
    WCHAR           wszTitle[DISPLAY_MAX];
    SQLSMALLINT     iCol = 1;

    SetConsole(cDisplaySize+2, TRUE);

    for (; pBinding; pBinding = pBinding->sNext)
    {
        TRYODBC(hStmt,
                SQL_HANDLE_STMT,
                SQLColAttribute(hStmt,
                    iCol++,
                    SQL_DESC_NAME,
                    wszTitle,
                    sizeof(wszTitle), // Note count of bytes!
                    NULL,
                    NULL));

        wprintf(DISPLAY_FORMAT_C,
                 PIPE,
                 pBinding->cDisplaySize,
                 pBinding->cDisplaySize,
                 wszTitle);
    }

error:

    wprintf(L" %c", PIPE);
    SetConsole(cDisplaySize+2, FALSE);
    wprintf(L"\n");

}


//************************************************************************
//* SetConsole: sets console display size and video mode
//*
//*  Parameters
//*      siDisplaySize   Console display size
//*      fInvert         Invert video?
//************************************************************************/

void SetConsole(DWORD dwDisplaySize,
                BOOL  fInvert)
{
    HANDLE                          hConsole;
    CONSOLE_SCREEN_BUFFER_INFO      csbInfo;

    // Reset the console screen buffer size if necessary

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hConsole != INVALID_HANDLE_VALUE)
    {
        if (GetConsoleScreenBufferInfo(hConsole, &csbInfo))
        {
            if (csbInfo.dwSize.X <  (SHORT) dwDisplaySize)
            {
                csbInfo.dwSize.X =  (SHORT) dwDisplaySize;
                SetConsoleScreenBufferSize(hConsole, csbInfo.dwSize);
            }

            gHeight = csbInfo.dwSize.Y;
        }

        if (fInvert)
        {
            SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes | BACKGROUND_BLUE));
        }
        else
        {
            SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes & ~(BACKGROUND_BLUE)));
        }
    }
}

