#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"fl.h"

/*
 *	formletter program
 *		code modified from class code by Jack on Nov 11, 2017
 *		(added get_opts, open_file)
 *
 *	usage: fl format [options] [datafile(s)]
 *
 *	Takes data form stdin if not specified, output goes to stdout
 *	
 *	options: -d<char>	(sets field_delim to <char>)
 *		 -D 		(sets field_delim to \n)
 *		 -r<char>	(sets record_delim to <char>)
 */

//helper functions
static int get_opts(char *, struct optvars *);
static int open_file(FILE **, FILE **, char *);

static char	*myname ;		/* used by fatal() */

int main(int ac, char *av[])
{
	FILE *fpfmt = NULL;
	FILE *fpdat = NULL;
	struct optvars ov = { ';', '\n'};	//struct for delims

	myname = *av;
	
	//check that there is at least one arg: the format file
	if ( ac == 1 )
		fatal("usage: fl format [datafile..]","");
	
	//iterates thru args until NULL sentinel
	while(*(++av)){
		if (get_opts(*av, &ov)){ //if option, sets optvars
			continue;
		}
		else if (open_file(&fpfmt, &fpdat, *av)){
			//if data opened, fpfmt has to have been set
			process(fpfmt, fpdat, ov);
			fclose(fpdat);
		}
	}
	
	//if no dat file(s) specified, read dat from stdin
	if (fpdat == NULL){
		if (fpfmt == NULL){	//err if no format
			fatal("no format file provided","");
		}
		process(fpfmt, stdin, ov);
	}

	fclose(fpfmt);		// release this memory
	return 0;
}

/*
 *	static int open_file(fpfmt, fpdat, filename)
 *		code by Jack
 *		purpose: attempts to open file as format (if none yet) or data
 *		returns: 1 if data file opened, 0 if format file opened
 *		inputs: fpfmt and fp are pointers to the file pointers of the
 *			same name in main; filename is a pointer to a str
 *		errors: if file not openable, calls fatal
 *		use: helper in main
 */

static int
open_file(FILE **fpfmt, FILE **fpdat, char *filename)
{
	if(*fpfmt == NULL){
		if(  (*fpfmt = fopen( filename , "r")) == NULL ){
			fatal("Cannot open format file:", filename);
		}
		return 0;
	}
	else{
		if( (*fpdat = fopen( filename , "r")) == NULL ){
			fatal("Cannot open data file:", filename);
		}
		return 1;
	}
}

/*
 *	static int get_opts(arg, ov)
 *		code by Jack
 *		purpose: checks if arg is an option and stores in ov
 *		input: arg (addr of string), ov (addr of optvars struct)
 *		returns: YES if option read, NO if not
 *		errors: if invalid option calls fatal
 *		use: helper in main
 */

static int
get_opts(char *arg, struct optvars *ov)
{
	if(*arg != '-'){
		return NO;	//not an option
	}
	switch(*(++arg)){	//analyize next char
		case 'd':
			//check format
			if(strlen(arg) != 2){
				fatal("invalid option: ","usage: -d<delim>");
			}
			//write delim to struct
			ov->field_delim = *(++arg);
			break;
		case 'D':
			//check format
			if(strlen(arg) != 1){
				fatal("invalid option: ","usage: -D");
			}
			//write delim to struct
			ov->field_delim = '\n';
			break;
		case 'r':
			//check format
			if(strlen(arg) != 2){
				fatal("invalid option: ","usage: -r<delim>");
			}
			//write delim to struct
			ov->record_delim = *(++arg);
			break;
		default:
			fatal("invalid option: ", arg);
			break;
	}
	return YES;
}	
void fatal(char *s1, char *s2)
/*
 *	fatal error handler
 *	purpose: print error message to stderr then exit
 *	input:   two strings that are printed 
 *	never returns
 */
{
	fprintf(stderr, "%s: %s%s\n",  myname, s1, s2 );
	exit(1);
}
