#include <string.h>
#include <stdlib.h>

#define BUFSIZE 256
#define INPUT_DIR "_test/inputs/"
#define DEFINE "#define"
#define UNDEF "#undef"
#define IF "#if"
#define ELSE "#else"
#define ELIF "#elif"
#define ENDIF "#endif"
#define IFDEF "#ifdef"
#define IFNDEF "#ifndef"
#define INCLUDE "#include"
#define DELIMITERS "\n\r\t []{}<>=+-*/%!&|^.,:;()\\"
#define D_ARG "-D"
#define I_ARG "-I"

void remove_extra_spaces(char *str);

char *replace_string(char *haystack, char *old_word, char *new_word);

char *my_strdup(char *s);
