/*
 * version 1.4: 2004-nov-19 23:50
 */

#ifndef WS13_INCL
#define WS13_INCL
#define	BUFLEN	512
#define	YES	1
#define	NO	0

struct link {
		char	    *word;		/* the string	*/
		char	    *val;		/* the `value'	*/
		struct link *next;		/* the next one	*/
	};

struct symtab {
		struct link head;
		struct link *current_link;
	};

typedef struct symtab symtab_t;

symtab_t  *new_table();
int	  in_table(symtab_t *,char []);
int	  insert(symtab_t *, char [], char []);
char      *lookup(symtab_t *,char []);
int	  update(symtab_t *,char [], char []);
char     *firstword(symtab_t *);
char     *nextword(symtab_t *);
int      table_len(symtab_t *);
void     delete(symtab_t *,char []);
void     clear_table(symtab_t *);
void     free_table(symtab_t *);
void	 show_table(symtab_t *);
int	 table_export(symtab_t *);

#endif	/* WS13_INCL */
