#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"ws13.h"

/*
 *	wordstore version 1.3.3
 *
 *	history: 11 nov 2017 (Jack implemented clear_table and fixed delete)
 *      history: 21 nov 2006 (fixed memory overrun in table_export)	
 *	history: 19 nov 2004 (added table_export and show_table)
 *	history: 19 nov 2003 (added memleak protection in insert)
 *	history: 22 nov 2001 (added symtab_t)
 *
 *	storage system for storing name:address pairs
 *	the function in this file are stolen directly from the wlfiler
 *	system
 *
 *	functions are:
 *			new_table()  	     - create a new symtab
 *			free_table(tp)	     - deallocates a table
 *			in_table(tp,str)     - sees if word is in table at tp
 *			insert(tp,str,val)   - insert value into table
 *			lookup(tp,str)	     - retrieve value
 *			update(tp,str,val)   - update value in table
 *			firstword(tp)	     - move to top of table
 *			nextword(tp)	     - return word at current row
 *			table_len(tp)	     - returns length of table
 *                      delete(tp,str)       - removes an item from the list
 *			clear_table(tp)	     - deallocates all links in table
 */

/*
 *	data structure for this unit:  a linked list of structs
 */

/*
 * declarations of internal functions
 */

static struct link *find_node(symtab_t *,char []);
static char        *new_string(char[]);

/*
 *	new_table(): creates an initialized a symtab
 */

symtab_t *new_table()
{
	symtab_t *rv;

	rv = malloc(sizeof(struct symtab));
	if ( rv != NULL ){
		rv->head.next = NULL;
		rv->current_link = NULL;
	}
	return rv;
}

/*
 * 	free_table(): deallocate an allocated table
 *	args: ptr to a symtab
 *	rets: nothing
 */

void free_table( symtab_t * p)
{
	free(p);
}

/* ----------------------------------------------------------------
 *	int in_table(symtab_t*,char str[])
 */
int 
in_table( symtab_t *tp, char str[] )
{
	return ( find_node(tp, str) ? YES : NO );
}

/* ----------------------------------------------------------------
 *	int insert(tp, str, val)
 *		 params:  tp - pointer to a symtab
 *                        str - the key
 *                        val - the associated value
 *
 *		purpose: add the pair str=val to the symtab at *tp
 *		returns: NO if no more memory, YES if OK
 *		 method: add new node to head of list.  It's easy
 */

int
insert( symtab_t *tp,  char str[], char val[] )
{
	struct link	*newlink;

	newlink = malloc(sizeof(struct link));	/* get mem for link	*/
	if ( newlink == NULL )			/* or quit		*/
		return NO;

	newlink->word = new_string( str );	/* allocate + copy	*/
	if ( newlink->word == NULL )		/* or quit		*/
	{
		free(newlink);			/* avoid memleaks	*/
		return NO;
	}

	newlink->val = new_string( val );	/* allocate + copy	*/
	if ( newlink->val == NULL )		/* or quit		*/
	{
		free(newlink->word);		/* avoid memleaks	*/
		free(newlink);			/* thanks to jj and rs	*/
		return NO;
	}

	newlink->next  = tp->head.next;		/* attach list to link	*/
	tp->head.next  = newlink;		/* make head pt to link */
	return YES;
}

/* ----------------------------------------------------------------
 *	char *lookup( symtab_t*, str )  returns string at value or NULL
 */
char *
lookup( symtab_t *tp, char str[] )
{
	struct link *linkp;

	linkp = find_node( tp, str );
	return ( linkp ? linkp->val : NULL );
}

/* ----------------------------------------------------------------
 * 	int update( str, val )
 */

int
update( symtab_t *tp, char str[], char val[] )
{
	struct link *linkp;

	linkp = find_node( tp, str );
	if ( linkp != NULL )				/* if there	*/
	{
		free( linkp->val );			/* free old mem	*/

		linkp->val = new_string(val);		/* make new str */
		if ( linkp->val == NULL )
			return NO;			/* or bail out	*/
		return YES;				/* and go	*/
	}
	return YES;					/* no errors	*/
}

/* ----------------------------------------------------------------
 *	char *firstword()
 */

char *
firstword(symtab_t *tp)
{
	tp->current_link = tp->head.next ;
	return nextword(tp);
}

/* ----------------------------------------------------------------
 *	char *nextword()
 */

char *
nextword(symtab_t *tp)
{
	char *retval = NULL;

	if ( tp->current_link != NULL ){
		retval = tp->current_link->word;
		tp->current_link = tp->current_link->next;
	}
	return retval;
}

/* ----------------------------------------------------------------
 *	int	table_len()
 */
int
table_len(symtab_t *tp)
{
	struct link *linkp;			/* a cursor	*/
	int	len = 0;			/* a counter	*/

	for( linkp = tp->head.next ; linkp ; linkp = linkp->next )
		len++;

	return len;
}

/* ----------------------------------------------------------------
 *	void delete()
 *	removes an item from the table and frees the storage
 *   	code modified by Jack
 *
 */

void
delete( symtab_t *tp, char str[] )
{
	struct link *linkp;		//prev link
	struct link *one_to_delete;	

	//itterates thru links, stops before last link
	for( linkp = &(tp->head) ; linkp->next ; linkp = linkp->next )
		if ( strcmp(linkp->next->word, str) == 0 ) //checks next link
		{
			//relinks prev link with link after
			one_to_delete = linkp->next;
			linkp->next = one_to_delete->next;
			//memory clean up
			free( one_to_delete->word );
			free( one_to_delete->val );
			free( one_to_delete );
			break;
		}
}

/*
 *  clear_table(tp) - clears out all nodes from the table
 *		    - and resets head pointer to NULL
 *		    - and resets current_link to NULL
 *  code by Jack
 */
void
clear_table(symtab_t *tp)
{
	struct link *linkp, *nextlink;
	for (linkp = tp->head.next; linkp ; linkp = nextlink){
		nextlink = linkp->next;
		free( linkp->word );
		free( linkp->val );
		free( linkp );
	}
	tp->head.next = NULL;
	tp->current_link = NULL;
}
	
/*
 * table_export(tp) - exports current table to the environment
 *
 * arg: a symtab - export that table to the environment
 * ret: 1 for ok, 0 for memory error
 * 		      
 * note: keeps a static copy of the original environment
 */

int
table_export(symtab_t *tp)
{
	extern char **environ;			/* the current environ 	*/
	static char **env_orig = NULL;		/* orig environment	*/
	static int  env_orig_len = 0;		/* length of orig env	*/
	int	len, i;				/* temp counters etc	*/
	struct link *linkp;			/* traverse list	*/
	char	*s;				/* new strings in env	*/
	void	free_strings();

	/* if first call, record orig environ and its length */
	if ( env_orig == NULL ){
		env_orig = environ;
		while( environ[env_orig_len] != NULL )
			env_orig_len++;
	}
	else 
		free_strings(environ+env_orig_len);

	/* then compute new size and build new environ */
        /* need one extra for terminating NULL */
	len = env_orig_len + table_len(tp) + 1;
	environ = (char **) malloc(len * sizeof(char *));
	if ( environ == NULL )
		return 0;
	for(i=0; i<env_orig_len; i++)
		environ[i] = env_orig[i];

	for( linkp = tp->head.next ; linkp ; linkp = linkp->next ){
		s = malloc(2+strlen(linkp->word)+strlen(linkp->val));
		if ( s == NULL )
			return 0;
		sprintf(s, "%s=%s", linkp->word, linkp->val);
		environ[i++] = s;
	}
	environ[i] = NULL;
	return 1;
}

void free_strings(char *e[])
{
	int	i;

	for(i=0; e[i] != NULL; i++)
		free(e[i]);
}

/* ----------------------------------------------------------------- *
 * internal helper functions to reduce repeated code in the
 * functions above.  These are all declared static to make them
 * private to this file
 * The functions are:
 *
 *   struct link *find_node(symtab_t *tp, str)
 *   char        *new_string(str)
 */

/*
 *  internal helper function:  struct link *find_node(str)
 *  used to find a node with the specified string
 *   method: loop until at end of list or until a match
 *  returns: node where loop ended (ie at null or a match)
 */

static struct link *
find_node(symtab_t *tp,char str[])
{
	struct link *lp;

	for(lp=tp->head.next; lp && strcmp(lp->word,str) != 0; lp = lp->next)
		;
	return lp;
}

/*
 * allocate space for a string and copy the string into it
 * returns ptr to new string or NULL if no memory
 */

static char *
new_string(char str[])
{
	char *strp;

	strp = malloc( 1 + strlen(str) );	/* allocate	*/
	if ( strp != NULL )			/* if ok	*/
		strcpy( strp, str );		/* copy		*/
	return strp;				/* return ptr	*/
}

/*
 * will display a table, useful for debugging
 */

void
show_table(symtab_t *tab)
{
	struct link *lp;

	for(lp = tab->head.next; lp ; lp = lp->next)
	{
		printf("  [%s]->[%s]\n", lp->word, lp->val);
	}
}
