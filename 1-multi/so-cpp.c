#include "hash_table.h"

#define BUFSIZE 256
#define DEFINE_LENGTH 6
#define DEFINE "#define"
#define UNDEF "#undef"
#define DELIMITERS "\n\r\t []{}<>=+-*/%!&|^.,:;()\\"
#define QUOTES "\""
#define D_ARG "-D"

char *replace_string(char *haystack, char *old_word, char *new_word) {
    int length = strlen(haystack);
    int i, number_of_appearances = 0;
    for (i = 0; i < length; i++) {
        // the word that we want to replace is at the beginning of the text
        // so a word is found
        if (strstr(haystack + i, old_word) == &haystack[i]) {
            number_of_appearances++;
            // jump the found word
            i += strlen(old_word);
        }
    }

    char *result = (char *)calloc(1 + strlen(haystack) + number_of_appearances * (strlen(new_word) - strlen(old_word)), sizeof(char));
    if (result == NULL)
        return NULL;

    i = 0;
    while (*haystack) {
        // if the new word is found in the haystack replace it
        if (strstr(haystack, old_word) == haystack) {
            memcpy(result + i, new_word, strlen(new_word));
            i += strlen(new_word);
            haystack += strlen(old_word);
        } else {
            result[i] = *haystack;
            i++;
            haystack++;
        }
    }

    return result;
}

int parse_arguments(THashTable *hash_table, FILE **input_file, FILE **output_file, int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], D_ARG, strlen(D_ARG))) {
            char *symbol_mapping = argv[++i];
            if (strchr(symbol_mapping, '=') != NULL) {
                char *key = strtok(symbol_mapping, "=");
                char *value = strtok(NULL, "=");
                put(hash_table, key, value);
            } else {

            }
        } else if (*input_file == stdin) {
            *input_file = fopen(argv[i], "r");
            if (*input_file == NULL) {
                return 12;
            }
        } else if (*output_file == stdout) {
            *output_file = fopen(argv[i], "w+");
            if (*output_file == NULL) {
                return 12;
            }
        } else {
            return 12;
        }
    }

    return 0;
}

int preprocess_code(FILE *input_file, FILE *output_file, THashTable *hash_table) {
    char buffer[BUFSIZE];
    int rc;
    memset(buffer, 0, BUFSIZE);
    while (fgets(buffer, BUFSIZE, input_file)) {
        char *replace_buffer = NULL;
        // define was read
        if (buffer[0] != '\n') {
            if (!strncmp(buffer, DEFINE, DEFINE_LENGTH)) {
                strtok(buffer, DELIMITERS);
                char *key = strtok(NULL, DELIMITERS);
                char *value = strtok(NULL, DELIMITERS);
                rc = put(hash_table, key, value);
                if (rc == -1)
                    return 12;
            } else if (!strncmp(buffer, UNDEF, strlen(UNDEF))) {
                strtok(buffer, DELIMITERS);
                char *undefined_key = strtok(NULL, DELIMITERS);
                remove_key(hash_table, undefined_key);
            } else {
                // preprocess the line of code
                char buffer_copy[BUFSIZE];
                memset(buffer_copy, 0, BUFSIZE);
                memcpy(buffer_copy, buffer, BUFSIZE);
                // check if something should be changed in the line of code
                char *token = strtok(buffer_copy, DELIMITERS);
                while (token)  {
                    char *hash_table_value = get(hash_table, token);
                    if (hash_table_value != NULL) {
                        // replace the word in the text
                        replace_buffer = replace_string(buffer, token, hash_table_value);
                        break;
                    }
                    token = strtok(NULL, DELIMITERS);
                }
                if (replace_buffer) {
                    fprintf(output_file, "%s", replace_buffer);
                    free(replace_buffer);
                }
                else
                    fprintf(output_file, "%s", buffer);
            }
            memset(buffer, 0, BUFSIZE);
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    THashTable *hash_table = create_hash_table();
    char buffer[BUFSIZE];
    int rc;
    memset(buffer, 0, BUFSIZE);
    
    if (hash_table == NULL)
        return 12;

    FILE *input_file = stdin;
    FILE *output_file = stdout;
    rc = parse_arguments(hash_table, &input_file, &output_file, argc, argv);
    if (rc == 12) {
        if (input_file != NULL && input_file != stdin)
            fclose(input_file);
        if (output_file != NULL && output_file != stdout)
            fclose(output_file);
        destroy_hash_table(hash_table);
        return 12;
    }
    
    rc = preprocess_code(input_file, output_file, hash_table);
    destroy_hash_table(hash_table);
    fclose(input_file);
    fclose(output_file);
    return rc;
}
