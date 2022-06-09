REBOL [
	title: "ODBC module builder"
	type: module
]

cmd-words: []
arg-words: []

commands: [
	init-words: [cmd-words [block!] arg-words [block!]]
	;--------------------------

	info: [
		{Returns info about ODBC extension library}
		/of handle [handle!] {ODBC Extension handle}
	]
	open: [
		{Opens a new database connection}
		conn [string!]
	]
	exec: [
		{Runs zero or more semicolon-separate SQL statements}
		db   [handle!] "odbc-db"
		sql  [string!] "statements"
	]
	finalize: [
		"Deletes prepared statement"
		stmt [handle!] "odbc-stmt"
	]
	trace: [
		{Traces debug output}
		db   [handle!] "odbc-db"
		mask [integer!]
	]
	prepare: [
		"Prepares SQL statement"
		db   [handle!] "odbc-db"
		sql  [string!] "statement"
	]
	reset: [
		"Resets prepared statement"
		stmt [handle!] "odbc-stmt"
	]
	step: [
		"Executes prepared statement"
		stmt [handle!] "odbc-stmt"
		/rows "Multiple times if there is enough rows in the result"
		 count [integer!]
		/with
		 parameters [block!]
	]
	close: [
		{Closes a database connection}
		db   [handle!] "odbc-db"
	]

	initialize: [
		{Initializes the ODBC library}
	]
	shutdown: [
		{Deallocate any resources that were allocated}
	]

	;--------------------------
]


header: ajoin [
	{REBOL [Title: "Rebol ODBC Extension"}
	{ Name: odbc Type: module Exports: []}
	{ Version: 0.1.0}
	{ Author: Oldes}
	{ Date: } now
	{ License: Apache-2.0}
	{ Url: https://github.com/Siskin-framework/Rebol-ODBC}
	#"]"
]
enum-commands:  {enum odbc_commands ^{}
enum-cmd-words: {enum words_odbc_cmd ^{W_ODBC_CMD_0,}
enum-arg-words: {enum words_odbc_arg ^{W_ODBC_ARG_0,}
func-commands: clear {}

foreach word cmd-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	append enum-cmd-words ajoin ["^/^-W_ODBC_CMD_" word #","]
]
foreach word arg-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	append enum-arg-words ajoin ["^/^-W_ODBC_ARG_" word #","]
]

foreach [name spec] commands [
	name: form name
	append header ajoin [lf name ": command " mold/flat spec]

	replace/all name #"-" #"_"
	append func-commands ajoin ["^/int cmd_odbc_" name "(RXIFRM* frm, void* reb_ctx);"]
	append enum-commands ajoin ["^/^-CMD_ODBC_" uppercase name #","]
]

new-line/all cmd-words false
new-line/all arg-words false
append header rejoin [{^/init-words words: } mold cmd-words #" " mold arg-words]
append header {^/protect/hide 'init-words}



;print header

out: make string! 2000
append out {// auto-generated file, do not modify! //

#include "odbc-command.h"

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 5
#define MIN_REBOL_UPD 4
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

extern REBCNT Handle_ODBC_DB;
extern REBCNT Handle_ODBC_STMT;

}
append out join enum-commands "^/};^/"
append out join enum-cmd-words "^/};^/"
append out join enum-arg-words "^/};^/"
append out func-commands
append out {^/^/#define EXT_ODBC_INIT_CODE \}


foreach line split header lf [
	replace/all line #"^"" {\"}
	append out ajoin [{^/^-"} line {\n"\}] 
]
append out "^/"


;print out

write %odbc-rebol-extension.h out
