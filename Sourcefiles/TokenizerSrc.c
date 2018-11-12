#include "Tokenizer.h"


struct LinkedLexeme *createLexeme(char *value)
{
	struct LinkedLexeme *retval = malloc(sizeof(struct LinkedLexeme));
	retval->value = value;
	retval->next = NULL;
	return retval;
}

/*
Creates an empty LinkedParent list with an identifier. Branches represent
tokens so the identifier will indicate which type of tokens
this branch contains. This method copies the id passed trough
the parameters up until the point indicated by the 'idsize' variable
and terminates it with a NUL character.
*/
struct LinkedToken *createLinkedToken(char *id, int idsize)
{
	struct LinkedToken *retval = malloc(sizeof(struct LinkedToken));
	retval->next = NULL;
	retval->id = malloc((sizeof(char) * idsize) + 1);
	retval->sons = NULL;
	memset(retval->id, 0x00, idsize + 1);
	memcpy(retval->id, id, idsize);
	return retval;
}


int insertToken(struct LinkedToken *branch, struct LinkedToken *head)
{
	if (head->next == NULL) {
		head->next = branch;
		return 1;
	}
	else {
		struct LinkedToken *temp = malloc(sizeof(struct LinkedToken));
		temp->id = head->next->id;
		temp->next = head->next->next;
		temp->sons = head->next->sons;
		free(head->next);

		branch->next = temp;
		head->next = branch;
	}
	return 1;
}

/*
Adds a new word to the branch. Returns 1 if it was able to add the word to the branch.
Cero if it was not. If the method is not able to add the word it means that the 
word already exists.
*/
int insertTokenValue(char *word, int wordsize, struct LinkedToken *branch)
{
	char *allocword = malloc(sizeof(char) * (wordsize + 1));
	memset(allocword, 0x00, wordsize + 1);
	memcpy(allocword, word, wordsize);

	if (branch->sons == NULL) {
		branch->sons = createLexeme(allocword);
	}
	else if (branch->sons->next == NULL) {
		branch->sons->next = createLexeme(allocword);
	}
	else {
		struct LinkedLexeme *temp = malloc(sizeof(struct LinkedLexeme));
		temp->value = branch->sons->value;
		temp->next = branch->sons->next;
		free(branch->sons);

		struct LinkedLexeme *head = createLexeme(allocword);
		head->next = temp;
		branch->sons = head;
	}
	return 1;
}

/*
Reads a whole file and returns a char buffer with the text it contained. The
FILE pointer must be opened in text form. Sets the integer pointer passed
trough the parameters to the numbers of chars read.
*/
char *readAll(FILE *fl, int *size)
{
	char *rdata = NULL;
	int sector = 512;
	int count = 1;
	int dataread;

	do {
		fseek(fl, 0, SEEK_SET);
		rdata = realloc(rdata, sizeof(char) * (sector * count));
		dataread = fread(rdata, 1, (sector * count), fl);
		count++;
	} while (feof(fl) == 0);

	rdata = realloc(rdata, dataread);
	return rdata;
}

/*
Skips all the chars in the char pointer until it finds the char
passed trough the parameters or it reaches the limit passed trough the
parameters. Returns the amount of characters it skipped over if it 
eventually found the char or -1 if it did not find it but reached
the limit. DOES NOT change the pointer passed trough the parameters as
this just recieves a copy of it.
*/
int skipchar(char *ptr, char until)
{
	for (int x = 0;; ptr++, x++) {
		if (*ptr == until)
			return x;
	}
}

/*
*/
struct LinkedToken *parsetoken(char **grammar) 
{
	int idlen = skipchar(*grammar, TOKEN_SEPARATOR);
	struct LinkedToken *retroot = createLinkedToken(*grammar, idlen);
	idlen++; //Skip comma.
	*grammar += idlen;
    
    int len = 0;
	for (; **grammar != TOKEN_LIMIT; (*grammar)++, len++) {
		if (**grammar == TOKEN_SEPARATOR) {
			insertTokenValue(*grammar - len, len, retroot);
			(*grammar)++;
			len = 0;
		    continue;
		}
	}	
	insertTokenValue(*grammar - len, len, retroot);
	(*grammar)++;
	return retroot;

}

/*
*/
struct LinkedToken *createTokenizer(char *grammar)
{
	int len = skipchar(grammar, TOKEN_LIMIT);
	len++;
	grammar += len;
	struct LinkedToken *retroot = parsetoken(&grammar);

	for(; *grammar != TOKEN_END; grammar++){
		if (*grammar == TOKEN_LIMIT) {
			grammar++;
			insertToken(parsetoken(&grammar), retroot);
		}
	}
	return retroot;
}

/*
Frees all memory used by the tokenizer passed trough the parameters.
*/
void destroyTokenizer(struct LinkedToken *head)
{
	struct LinkedToken *nextHead;
	struct LinkedLexeme *nextSon;
	while (head != NULL) {
		while (head->sons != NULL) {
			nextSon = head->sons->next;
            free(head->sons->value);
			free(head->sons);
			head->sons = nextSon;
		}
		nextHead = head->next;
		free(head->id);
        free(head);
		head = nextHead;
	}
}

void debug_print_token(struct LinkedToken *holder) 
{
	struct LinkedToken *head = holder;
	struct LinkedLexeme *childhead;
	while (holder != NULL) {
		printf("TOKENS: %s\n", holder->id);
		childhead = holder->sons;
		while (holder->sons != NULL) {
			printf("%s\n", holder->sons->value);
			holder->sons = holder->sons->next;
		}
		holder->sons = childhead;
		printf("----------\n");
		holder = holder->next;
	}
	holder = head;
}