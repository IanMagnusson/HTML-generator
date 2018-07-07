#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"fl.h"
#include	"ws13.h"

//helper functions
static int strcatchar(char *, char, int);
static void putfield(symtab_t *,char *,char *);
static int pre_process(symtab_t *, FILE *, struct optvars, char *, char *);
static int field_process(symtab_t *, FILE *, struct optvars, char *, char *);
static int val_process(symtab_t *, FILE *, struct optvars, char *, char *);
static int field_insert(symtab_t *, FILE *);
static void read_insert_point(FILE *, char *);
static void run_shell_cmd(symtab_t *, char *);

/**
 *	process(fmt, data)
 *
 *	Purpose: read from datafile, format and output selected records
 *	Input:   fmt 		- input stream from format file
 *		 data		- stream from datafile
 *		 ov		- a struct of option variables
 *	Output:  copied fmt to stdout with insertions
 *	Errors:  not reported, functions call fatal() and die
 *	history: 2012-11-28 added free_table (10q BW)
 *		 2017-11-11 Jack implemented get_record and mailmerge
 **/

void
process(FILE *fmt, FILE *data, struct optvars ov)
{
	symtab_t *tab;

	if ( (tab = new_table()) == NULL )
		fatal("Cannot create storage object","");

	while ( get_record(tab,data,ov) != NO )	// while more data	
	{
		mailmerge( tab, fmt );		// merge with format	
		clear_table(tab);		// discard data	
	}

	free_table(tab);			// no memory leaks!	
}
    
/*----------------------------------------------------------------------------*
 *			get_record and its helpers			      *
 *----------------------------------------------------------------------------*/

/*
 *	int get_record(tp, fp, ov)
 *		code by Jack
 *		purpose: reads the next data record from the stream at *fp
 *			and stores it in table at *tp
 *		input:	tp - a pointer to symtab_t where record will be stored
 *			fp - a pointer to data file stream
 *			ov - a struct of option variables
 *		returns: YES if record read, NO if EOF without read
 *		errors: no memory available or bad input (calls fatal())
 *		notes: strips preceding whitespace from fieldnames, truncates
 *			fields and vals larger than MAXFLD and MAXVAL, ignores 
 *			blank lines, blank data, and records of blank data	
 */

int
get_record(symtab_t *tp, FILE *fp, struct optvars ov)
{
	char field[MAXFLD] = "", val[MAXVAL] = "";
	
	/*
	 * The *_process functions are loops nested in the sequence below.
	 * Each is a state in an FSM that reads dat file char by char.
	 */
	
	return pre_process(tp, fp, ov, field, val);
	//------->calls field_process() at begining of fieldname
	//-------------->calls val_process() at begining of val
}


/*
 *	static int pre_process(tp, fp, ov, field, val)
 *		code by Jack
 *		purpose: initial state, skips whitespace until field start
 *		input: tp, fp, ov,
 *		       field and val are pointers to strings for read buffer
 *		returns: YES if record read, NO if EOF without read
 *		errors: blank fieldname, calls fatal
 *		use: helper in get_record
 */

static int
pre_process(symtab_t *tp, FILE *fp, struct optvars ov, char *field, char *val)
{
	char c, field_delim = ov.field_delim,
		record_delim = ov.record_delim;
	int anything_read = NO;
	
	while( (c=fgetc(fp)) != EOF ){
		//bad data
		if (c == '='){
			fatal("Bad Data:", "blank fieldname");
		}
		
		//if blank last record
		else if (c == record_delim && anything_read){
			return YES;
		}

		//if totaly blank field, ready for next
		else if (c == field_delim){
			continue;
		}
		else if (c != ' ' && c != '\t' && c != '\n'){
			strcatchar(field, c, MAXFLD);
			if(field_process(tp, fp, ov, field, val)){
				return YES;
			}
			anything_read = YES;
		}	
	}
	return anything_read;
}

/*
 *	static int field_process(tp, fp, ov, field, val)
 *		code by Jack
 *		purpose: second state, reads field until '='
 *		input: tp, fp, ov,
 *		       field and val are pointers to strings for read buffer
 *		returns: Only non-error path is to call val_process
 *			returns the return of val_process
 *			(YES if record read complete, NO if more to read)
 *		errors: field with no =, calls fatal
 *		use: helper in get_record
 */

static int
field_process(symtab_t *tp, FILE *fp, struct optvars ov, char *field, char *val)
{
	char c, field_delim = ov.field_delim,
		record_delim = ov.record_delim;
	
	while( (c=fgetc(fp)) != EOF ){
		//end of fieldname, send report back up
		if (c == '='){
			return val_process(tp, fp, ov, field, val);
		}
		
		//bad data
		else if (c == field_delim || c == record_delim){
			fatal("Bad data:", "field with no =");
		}
		
		//read
		else{
			strcatchar(field, c, MAXFLD);
		}
	}
	//bad data
	fatal("Bad Data:", "field with no =");
	return 0;	//gets around warning	
}

/*
 *	static int val_process(tp, fp, ov, field, val)
 *		code by Jack
 *		purpose: final state, records val until field end, writes to tp
 *		input: tp, fp, ov,
 *		       field and val are pointers to strings for read buffer
 *		returns: YES if record read complete, NO if more to read
 *		errors: none
 *		use: helper in get_record
 */

static int
val_process(symtab_t *tp, FILE *fp, struct optvars ov, char *field, char *val)
{
	char c, field_delim = ov.field_delim,
		record_delim = ov.record_delim;
	
	while( (c=fgetc(fp)) != EOF ){
		//if end of fieldd, write to table, and report more to read
		if (c == field_delim){
			putfield(tp, field, val);
					
			//reinit for new field read
			field[0] = '\0';
			val[0] = '\0';

			return NO;
		}

		//if end of record, write to table, and report done reading
		else if (c == record_delim){
			putfield(tp, field, val);
			return YES;
		}
				
		//if space left put c in field, read
		else {
			strcatchar(val, c, MAXVAL);
		}
	}
	
	//if EOF while reading, write to table, and report done reading
	putfield(tp, field, val);
	return YES;
}

/*----------------------------------------------------------------------------*
 *			mailmerge and its helpers			      *
 *----------------------------------------------------------------------------*/

/*
 *	void mailmerge(tp, fmt)
 *		code by Jack
 *		purpose: prints out the report format and includes data values 
 *			from table at *tp in the appropriate spots, or runs
 *			shell cmds prefixed with !
 *		input: tp (pointer to symtab_t), fmt (pointer to format file)
 *		errors: no terminating %, no mem for table_export; calls fatal
 *		note: % escapes itself, insert point fmt: %fieldname% or %!cmd%
 */

void
mailmerge(symtab_t *tp, FILE *fmt)
{
	char c;
	
	//copies out fmt, calls field_insert for insert points	
	while( (c=fgetc(fmt)) != EOF ){
		if(c == '%'){
			field_insert(tp, fmt);
		}
		else{
			putchar(c);
		}
	}
	//rewind fmt filestream to beginning for next use
	fseek(fmt,0L,SEEK_SET);
}

/*
 *	static int field_insert(tp, fmt)
 *		code by Jack
 *		purpose: reads insert point and inserts val from tp or runs cmd
 *		input: tp (pointer to table), fmt (pointer to format file)
 *		returns: YES if insert or cmd run, NO just % escape
 *		errors: no terminating %, calls fatal
 *		note: % escapes itself
 *		use: helper in mailmerge
 */	

static int
field_insert(symtab_t *tp, FILE *fmt)
{	
	char c;
	char field[MAXFLD] = "";
	char *val;
	
	while( (c=fgetc(fmt)) != EOF ){
		if(c == '%'){	//% escapes itself
			putchar(c);
			return NO;
		}
		else if (c == '!'){	//embedded shell cmds
			read_insert_point(fmt, field);	//record cmd
			run_shell_cmd(tp, field);
			field[0] = '\0';	//reinit field buffer
			return YES;
		}
		else{
			strcatchar(field, c, MAXFLD); //record first char
			read_insert_point(fmt, field);	//record rest
			if((val = (lookup(tp, field)))){
				printf("%s", val);	//if in tp, print val
			}
			field[0] = '\0';	//reinit field buffer
			return YES;
		}
	}
	fatal("no end \% at insert point:", field);
	return NO;	//gets around warning
}

/*
 *	static void read_insert_point(fmt, field)
 *		code by Jack
 *		purpose: reads name of insert_point into string at field
 *		inputs: fmt (pointer to file), (pointer to field str)
 *		errors: no terminating %, calls fatal
 *		use: helper in mailmerge
 */		

static void
read_insert_point(FILE *fmt, char *field)
{
	char c;
	
	while((c=fgetc(fmt)) != '%'){
		if (c == EOF){
			fatal("no end \% at insert point:", field);
		}
		strcatchar(field, c, MAXFLD);
	}
}	

/*
 *	static void run_shell_cmd(tp, field)
 *		code by Jack
 *		purpose: exports table to environment, runs field as shell cmd
 *		errors: no memory for table_export, calls fatal
 *		use: helper in mailmerge
 */
	
static void
run_shell_cmd(symtab_t *tp, char *field)
{
	//export table vals to environ
	if(table_export(tp) == 0){
		fatal("no mem for table_export", "");
	}
	//get output in order
	fflush(stdout);
	//call cmd
	system(field);
}

/*----------------------------------------------------------------------------*
 *			   Other Helper Functions			      *
 *----------------------------------------------------------------------------*/

/*
 *	static int strcatchar(str, c, max)
 *		code by Jack
 *		purpose: appends a char and a new null sentinel to a string
 *		input: str, c (char to append), max (int for maxlen)
 *		returns: YES if success, NO if no more room in str
 *		use: helper in get_record and mailmerge
 */

static int
strcatchar(char *str, char c, int max)
{
	int len = strlen(str);
	// if room for one more and sentinel
	if ((len + 1) < max){
		str[len] = c;
		str[len + 1] = '\0';
		return YES;
	}
	return NO;
}

/*
 *	static void putfield(tp, field, val)
 *		code by Jack
 *		purpose: puts or updates field and val strings into table at *tp
 *		input: tp (pointer to symtab_t), field, val
 *		error: no more memory, calls fatal()
 *		use: helper in get_record
 */

static void
putfield(symtab_t *tp,char *field,char *val)
{
	if (in_table(tp, field)){
		if (update(tp, field, val) == NO){
			fatal("no more memory for field:", field);
		}
	}
	else{
		if (insert(tp, field, val) == NO){
			fatal("no more memory for field:", field);
		}
	}
}
