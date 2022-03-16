#include "hash_table.h"

#define BUFSIZE 256
#define DEFINE_LENGTH 6
#define DEFINE "#define"
#define DELIMITERS "\t []{}<>=+-*/%!&|^.,:;()\\"

int main(int argc, char *argv[]) {
    THashTable *hash_table = create_hash_table();
    char buffer[BUFSIZE];
    int rc;
    memset(buffer, 0, BUFSIZE);
    
    if (hash_table == NULL)
        return 12;

    int i;
    FILE *input_file = stdin;
    FILE *output_file = stdout;
    for (i = 1; i < argc; i++) {
        if (input_file == stdin) {
            input_file = fopen(argv[i], "r");
            if (input_file == NULL) {
                destroy_hash_table(hash_table);
                return 12;
            }
        } else if (output_file == stdout) {
            output_file = fopen(argv[i], "w+");
            if (output_file == NULL) {
                fclose(input_file);
                destroy_hash_table(hash_table);
                return 12;
            }
        } else {
            fclose(input_file);
            fclose(output_file);
            destroy_hash_table(hash_table);
            return 12;
        }
    }
    while (fgets(buffer, BUFSIZE, input_file)) {
        // define was read
        if (buffer[0] != '\n') {
            if (!strncmp(buffer, DEFINE, DEFINE_LENGTH)) {
                strtok(buffer, DELIMITERS);
                char *key = strtok(NULL, DELIMITERS);
                char *value = strtok(NULL, DELIMITERS);
                rc = put(hash_table, key, value);
                if (rc == -1)
                    return 12;
            } else {
                // preprocess the line of code
                fprintf(output_file, "%s", buffer);
                char buffer_copy[BUFSIZE];
                memset(buffer_copy, 0, BUFSIZE);
                memcpy(buffer_copy, buffer, BUFSIZE);
            }
        }
        memset(buffer, 0, BUFSIZE);
    }
    destroy_hash_table(hash_table);
    fclose(input_file);
    fclose(output_file);
    return 0;
}
