#include	<stdio.h>
#include	<ctype.h>
#include	<strings.h>
#include	<string.h>
#include	<stdlib.h>

/*
 *	query parser version 2.2
 *
 *	this reads the REQUEST_METHOD and QUERY_STRING things
 *	from the environment and outputs a sequence of strings
 *	of the form  NAME="Value"
 *	suitable for using with the . operator in /bin/sh
 *
 *	to handle un-=named fields (as in ISINDEX things) invoke
 *	with the -unnamed option
 *
 *	version: 02 sep 2017 - block \ as a badchar
 *	version: 28 jan 2016 - now- report on bad var names
 *	version: 06 sep 2012 - refuse to over-ride any env variables(thebb)
 *	version: 13 jan 2006 - fixed the embedded semicolons
 *	version: 12 jan 2006 - required strings to be name=value
 *			       added semicolons to end of each line
 *	version: 17 Jan 2001 - made ~ and ? ok chars (for ewp.cgi)
 *	version: 11 Dec 2000 - added commandarg
 *	version: 10 Dec 2000 - fixed %## problem
 */


static	char	version[] = "qryparse.c 2.4 2017-09-02";
/* static	char	version[] = "qryparse.c 2.3 2016-01-28"; */
/* static	char	version[] = "qryparse.c 2.1 2006-01-13"; */

void	do_get(), do_post(), oops(char *, char *), convert();

#define	TILDE_QMARK_OK
#define	ERROR		0
#define	OK		1

int	has_names   = 1;

int main(int ac, char *av[])
{
	char	*cp;
	int	i;

	for( i = 1 ; i<ac ; i++ ){
		if ( strcmp( av[i], "-unnamed" ) == 0 )
			has_names = 0;
	}
	if ( ac > 1 )
		convert( av[1] );
	else
	{
		cp = getenv( "REQUEST_METHOD" );
		if ( cp == NULL )
			oops("No REQUEST_METHOD in environment!","");
		if ( strcmp( cp, "GET" ) == 0 )
			do_get();
		else if ( strcmp(cp, "POST") == 0 )
			do_post();
		else
			oops("unknown REQUEST_METHOD", cp);
	}
	return 0;
}

void
do_get()
{
	char	*cp;

	cp = getenv( "QUERY_STRING" );
	if ( cp == NULL )
		oops("No QUERY_STRING in environment!","");
	convert( cp );
}


void
do_post()
{
	char	*buffer;
	int	thelen ;
	char	*cp;
	int	c;
	
	cp = getenv( "CONTENT_LENGTH" );
	if ( cp == NULL )
		oops("No CONTENT_LENGTH in environment","");
	thelen = atoi( cp );
	if ( thelen <= 0 )
		oops("Content Length is not positive", "" );
	cp = buffer = malloc( thelen + 1 );
	if ( buffer == NULL )
		oops("Not enough memory for QUERY_STRING", "" );
	while( thelen-- ){
		c = getchar();
		if ( c == EOF )
			oops( "Insufficient data from stream", "" );
		*cp++ = c ;
	}
	*cp = '\0';
	convert( buffer );
}

void
oops(char *s1, char *s2)
{
	printf("PARSE_ERROR=YES\n");
	printf("PARSE_REASON=\"%s %s\"\n", s1, s2);
	exit(1);
}

/* this list ought to cover all the potential problems */
/* note: \r and \n are as important as ; and ` */


#ifdef	TILDE_QMARK_OK
char	badchars[] = "&;`'\"|*<>^()[]{}$\n\r\\";
#else
char	badchars[] = "&;`'\"|*?~<>^()[]{}$\n\r\\";
#endif

void
convert( char *s )
{
	char	*cp;
	int	print_command(char *);

	for( cp = strtok(s, "&") ; cp != NULL ; cp = strtok(NULL, "&" ) )
	{
		if ( print_command(cp) == ERROR )
			break;
	}
}

/*
 * expect something like name=value
 * with + for space, %## for hex chars
 *
 * BUT, ensure that the name is a legit shell variable
 * and then quote everthing in the right side
 * and remove backticks and dollar signs
 *
 * returns ERROR for errors, OK for ok
 */

int
print_command(char *s)
{
	char	*eqp;		/* where the = sign is 	*/
	char	*namep;		/* the name part	*/
	char	*valp;		/* the val part		*/
	int	valid_name(char*);
	void	output(char*);

	eqp = strchr(s, '=');
	if ( ! eqp )
		return ERROR;	/* oops...		*/

	*eqp  = '\0';
	valp  = eqp+1;		/* set valptr		*/
	namep = s;

	if ( ! valid_name(namep) ){
		oops("Invalid variable name: ", namep);
		return ERROR;
	}

	printf("%s=\'", namep);

	output(valp);

	printf("';\n");
	return OK;
}

/*
 * convert the string at *v into plain text making sure to
 * quote single quotes and remove non-printing chars like 
 */

void
output(char *s)
{
	int	c;
	int	x2c(char *);

	while ( (c = *s++) != '\0' ){
		switch( c ){
			case '+':	putchar(' ');
					break;
			case '%':	c = x2c(s);
					s += 2;
					/* fall thru */
			default:	if ( c == '\'' )
						printf("'\\''");
					else
						putchar(c);
		}
	}
}

/*
 * valid_name -- check if variable is a valid shell name
 *
 *       args: string to be tested
 *       rets: 0 if not valid, 1 if is valid
 *       note: considers any existing env variables to be invalid
 *             Thanks to tom hebb for catching this (2012-09-06)
 */
int
valid_name(char *s)
{
	if ( strlen(s) == 0 )
		return 0;

	if ( getenv(s) != NULL )		/* thebb fix 2012-09-06 */
		return 0;

	if ( *s != '_' && !isalpha(*s) )
		return 0;

	while ( isalnum(*++s) || *s == '_' )
		;

	if ( *s == '\0' )
		return 1;
	return 0;
}

int
x2c(char *s)
{
	int	hexval(char c);
	return (hexval(s[0])<<4) + hexval(s[1]);
}

int
hexval(char c)
{
	if ( c >= '0' && c <= '9' )
		return c - '0';
	if ( c >= 'A' && c <= 'F' )
		return 10 + c - 'A' ;
	if ( c >= 'a' && c <= 'f' )
		return 10 + c - 'a' ;
	return 0;
}
