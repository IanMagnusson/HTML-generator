#include	"ws13.h"
/*
 *	some constants
 */

#define	MAXFLD	40
#define	MAXVAL	120

//a struct for option variables

struct optvars {
	char	field_delim;
	char	record_delim;
};

/*
 *	function declarlations
 */

int	get_record(symtab_t *, FILE *, struct optvars);
void	mailmerge( symtab_t *, FILE *);
void	process(FILE*, FILE*, struct optvars);
void	fatal(char *, char *);
