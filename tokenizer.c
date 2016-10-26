
 // tokenizer.c
 // Naeem Hossain
 // nh302
 // September 29, 2016
 // Computer Architecture
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// A decimal integer constant token is a digit (1-9) followed by any number of digits.

// An octal integer constant token is a 0 followed by any number of octal digits (i.e. 0-7).

// A hexadecimal integer constant token is 0x (or 0X) followed by any number of hexadecimal digits
// (i.e. 0-9, a-f, A-F).

// A floating-point constant token is follows the rules for floating-point constants in Java or C.

// Your implementation must not modify the original string in any way.

/*
 * Tokenizer type.  You need to fill in the type as part of your implementation.
 */
struct TokenizerT_ {
    /*
     * Token Stream provided in argv[1]. Remains unchanged
     */
    char * token;

    /*
     * String of the unread token, updated when TKGetNextToken is called.
     */
    char * streamptr;

};
typedef struct TokenizerT_ TokenizerT;

/*
 * Hash table implementation of c operators
   Char Array operators - all operators defined in the referenced sheet; total of 41
   Char Array c_operator_pool - all chars that make up all the operators, used to determine if a char may be a c operator token
   Char Array hasht - a populated array whose key is the sum of ascii values that make up a c operator, used to find the token name
*/

char* operators[42] = {"( left paranthesis\0", ") right paranthesis\0","[ left brace\0","] right brace\0",". structure member\0","-> structure pointer\0","* indirect or multiply\0","& address or bitwise and\0","- minus or subtract\0","! negate\0","~ 1's comp\0","++ inc\0","-- dec\0","/ divide\0","% modulus\0","+ add\0",">> shift right\0","<< shift left\0","< less than\0","> greater than\0","<= less or equal\0",">= greater or equal\0","== equals\0","!= not equals\0","^ bitwise exclusive or\0","| bitwise or\0","&& logical and\0","|| logical or\0","? conditional true\0",": conditional false\0","+= plusequals\0","-= minusequals\0","*= multiplyequals\0","/= divideequals\0","%= modolusequals\0",">>= shiftrightequals\0","<<= shiftleftequals\0","&= andequals\0","^= exlusiveorequals\0","|= orequals\0",", comma operator\0", "= assignment\0"};
char c_operator_pool[] = "()[].->*&!~+/%<=^&:?,\0";
char *hasht[250] = {NULL};

void initializeTable(){
    int i;
    for (i = 0; i<42; ++i){
        int op_len,sum=0;
        for(op_len = 0; op_len <4; ++op_len)// max length is 4 after a space
        {  
            if (operators[i][op_len] == ' ')
            {
                break;
	        }
            sum += operators[i][op_len]; // Add ascii value 
        }

        // Three collisions occur in this implementation, changing keys solves
        if (sum == 124 || sum == 185 || sum == 94)
        {
        	switch(sum)
            {
        	case 124:   // '|' goes in sum+1
        	  if (op_len==1)
        	    sum += op_len;
        	  break;
                case 185:  // "|="" goes in sum+2
        	  if (op_len==2)
                    sum += op_len;
                  break;
                case 94:
        	  if (op_len==1) // "^"" goes in sum+1
                    sum += op_len;
                  break;
        	default:
        	  break;
        	}
        }
      hasht[sum] = operators[i]; // once the sum is determined to be unique, place in the hash table
    }
    return;
}

/*
 * TKCreate creates a new TokenizerT object for a given token stream
 * (given as a string).
 *
 * TKCreate should copy the arguments so that it is not dependent on
 * them staying immutable after returning.  (In the future, this may change
 * to increase efficiency.)
 *
 * If the function succeeds, it returns a non-NULL TokenizerT.
 * Else it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */
TokenizerT *TKCreate( char * ts ) {
    if (ts) 
    {
        TokenizerT * tokenizer = (TokenizerT*)malloc(sizeof(tokenizer));
        tokenizer->token = ts;
        tokenizer->streamptr = tokenizer->token;
        return tokenizer;
    }
    return NULL;
}

/*
 * TKDestroy destroys a TokenizerT object.  It should free all dynamically
 * allocated memory that is part of the object being destroyed.
 *
 * You need to fill in this function as part of your implementation.
 */
void TKDestroy( TokenizerT * tk ) {
    free(tk);
}

/*
 * Checks if a character is a delimiter. Returns 1 if it is, 0 if it isn't
 */
int isDelimiter(char s) {
    switch (s) {
        case ' ':
            return 1;
            break;
        case '\t':
            return 1;
            break;
        case '\v':
            return 1;
            break;
        case '\f':
            return 1;
            break;
        case '\n':
            return 1;
            break;
        case '\r':
            return 1;
            break;
        default:
            return 0;
    }

}

int isOperatorChar(char s){
    if (strchr(c_operator_pool, s) != NULL && s != '\0'){
        return 1;
    }
    else
        return 0;
}

/*
 * Reads token stream and returns the octal integer token
 */
char * getOctalToken(TokenizerT * tk) {
    int count = 0;
    while (tk->streamptr[count]>=48 && tk->streamptr[count]<=55) {
        count++;
    }

    char * token = malloc((count+1)*sizeof(char));
    token[count] = '\0';
    for (count = count-1; count>=0; count--) {
        token[count] = tk->streamptr[count];
    }
    tk->streamptr+=strlen(token);
    return token;
}


/*
 * Reads token stream and returns the dec integer token
 */
char * getDectoken(TokenizerT * tk) {
    int count = 0;
    char * type = "decimal ";
    while (isdigit(tk->streamptr[count]) || tk->streamptr[count]=='.' || tk->streamptr[count] == 'e') {
        if (tk->streamptr[count] == '.') { //this decimal integer might be a float
            if (isdigit(tk->streamptr[count+1])) { // check if the char after the period is a digit. If it is, then it's a float. If not, you've reached the end of a decimal integer.
                type = "float";
            } else {
                break;
            }
        }
        if (tk->streamptr[count] == 'e') { // the token might be in scientific notation
            if (strcmp(type, "float") == 0) { // scientific notation is only valid if the type was a float
                if (isdigit(tk->streamptr[count+1])) { //check if the char after the 'e' is a digit

                } else if (tk->streamptr[count+1] != '\0') { //maybe there is a negative exponent. check for a negative sign followed by a digit
                    if (tk->streamptr[count+1] == '-' && isdigit(tk->streamptr[count+2])) {
                        count++;
                    } else {
                        break;
                    }
                }
            } else {
                break;
            }
        }
        count++;
    }

    char * token = malloc((count+1)*sizeof(char));
    token[count] = '\0';
    for (count = count-1; count>=0; count--) {
        token[count] = tk->streamptr[count];
    }
    tk->streamptr+=strlen(token);
    printf("%s ", type);
    return token;
}


/*
 * Reads token stream and returns the hex integer token
 */
char * getHexToken(TokenizerT * tk) {
    int count = 2;
    while (isxdigit(tk->streamptr[count])) {
        count++;
    }

    char * token = malloc((count+1)*sizeof(char));
    token[count] = '\0';
    for (count = count-1; count>=0; count--) {
        token[count] = tk->streamptr[count];
    }
    tk->streamptr+=strlen(token);
    return token;
}


/*
 * Reads token stream and returns the operator token
 */
char * getOpToken(TokenizerT * tk){
    int count = 0, sum = 0;

    while(isOperatorChar(tk->streamptr[count])){
        sum += tk->streamptr[count];
        if (hasht[sum] == NULL){
            sum -= tk->streamptr[count];
            break;
        }
        count++;

    }

    // make the token
    char *token = malloc((count+1)*sizeof(char));
    token[count] = '\0';
    int f;
    for (f= 0; f < count; f++){
        token[f] = tk->streamptr[f];
    }

    // identify operator token after checking collision cases
    if (sum == 124 || sum == 185 || sum == 94){
         switch(sum){
             case 124:   // op | goes in sum+1
                if (strlen(token)==1)
                    sum += 1;
                break;
            case 185:  // op |= goes in sum+2
                if (strlen(token)==2)
                    sum += 2;
                break;
            case 94:
                if (strlen(token)==1) // op ^ goes in sum+1
                    sum += 1;
                break;
            default:
                break;
            }
    }

    // THIS ASSIGNMENT IS STUPID
    char *type = "INVALID TOKEN";
    if (hasht[sum] != NULL){
        type = hasht[sum];
        type += count+1;
    }

    printf("%s ", type);
    tk->streamptr+=strlen(token);

    return token;
}


/*
 * TKGetNextToken returns the next token from the token stream as a
 * character string.  Space for the returned token should be dynamically
 * allocated.  The caller is responsible for freeing the space once it is
 * no longer needed.
 *
 * If the function succeeds, it returns a C string (delimited by '\0')
 * containing the token.  Else it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */
char *TKGetNextToken( TokenizerT * tk ) {

     if (strlen(tk->streamptr)) {
         while (isDelimiter(tk->streamptr[0])) {
             tk->streamptr+=1;
         }
         if (strcmp(tk->streamptr, "") == 0) {
             return NULL;
         }
         if (isalnum(tk->streamptr[0])) {            //The token not a C operator or delimiter, since the first character is alphanumeric
            if (isalpha(tk->streamptr[0])) {    //It's a word

                //move forward until you reach a non-alphanum char
                int count = 0;
                while (isalnum(tk->streamptr[count])) {
                    count++;
                }

                //store token in new string, and return token
                int i = 0;
                char * token = (char *)malloc((count+1)*sizeof(char));
                for (i=0; i<count; i++) {
                    token[i] = tk->streamptr[i];
                }
                token[count] = '\0';
                tk->streamptr = tk->streamptr+count;
                printf("word ");
                return token;
            }

            if (tk->streamptr[0] == '0') {      //First character is a zero so it probably is a Hex, Octal, or Floating Point. Can still be error
                if (tk->streamptr[1] == '\0') {     //It's a zero decimal integer, since it is followed by nothing
                    tk->streamptr+=1;
                    printf("decimal ");
                    return "0";
                }
                if (tk->streamptr[1]>=48 && tk->streamptr[1]<=55) //It's an octal, because it has a digit following the zero
                { 
                    printf("octal ");
                    return getOctalToken(tk);
                }
                if (tk->streamptr[1] == 'x'|| tk->streamptr[1] == 'X') //probably a hex
                { 
                    if (!isxdigit(tk->streamptr[2])) //If the character after 0x is not a hexa character, print out '0'
                    {       
                        tk->streamptr+=1;
                        printf("decimal ");
                        return "0";
                    }
                    printf("hex ");
                    return getHexToken(tk);
                }
            }
            if (isdigit(tk->streamptr[0])) //Either decimal int or float point
            { 
                return getDectoken(tk);
            }
         }
         if (isOperatorChar(tk->streamptr[0])){
             return getOpToken(tk);
         } else 
         {
             printf("Bad token ");
             char * s = (char *)malloc(2*sizeof(char));
             s[0] = tk->streamptr[0];
             s[1] = '\0';
             tk->streamptr+=1;
             return s;
         }
     }
     return NULL;
}



/*
 * main will have a string argument (in argv[1]).
 * The string argument contains the tokens.
 * Print out the tokens in the second string in left-to-right order.
 * Each token should be printed on a separate line.
 */
int main(int argc, char **argv) {
    if (argc != 2){
        printf("Please enter exactly one argument.\n");
        return 0;
    }

    if(argv[1] == NULL){
        printf("Null stream\n");
        return 0;
    }

    TokenizerT * tokenizer = TKCreate(argv[1]);//argv[1]); //argv[1]);

    //printf("Token Stream: %s\n", tokenizer->token);
    if (tokenizer) {
        initializeTable();

        char * word = TKGetNextToken(tokenizer);
        while (word != NULL) {
            printf("\"%s\"\n",word);
            free(word);
            word = TKGetNextToken(tokenizer);
        }
    }
    return 0;
}
