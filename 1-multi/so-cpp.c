#include "hash_table.h"
#include "vector.h"
#include "utils.h"

char *parse_value(THashTable *hash_table, char *value, int *changed)
{
	char *copy_value = (char *)calloc(strlen(value) + 1, sizeof(char));
	char *new_value = NULL;
	char *token = NULL;
	char *token_value = NULL;

	if (copy_value == NULL)
		return NULL;

	memcpy(copy_value, value, strlen(value));

	token = strtok(copy_value, DELIMITERS);

	while (token) {
		token_value = get(hash_table, token);

		if (token_value != NULL) {
			if (new_value != NULL)
				free(new_value);
			new_value = replace_string(value, token, token_value);
			if (new_value == NULL) {
				free(copy_value);
				return NULL;
			}
		}
		token = strtok(NULL, DELIMITERS);
	}
	free(copy_value);
	if (new_value == NULL) {
		*changed = 0;
		new_value = value;
	}
	return new_value;
}

int parse_arguments(THashTable *hash_table, FILE **input_file, FILE **output_file, int argc, char *argv[], Vector *include_directories)
{
	int i, rc;
	char *symbol_mapping = NULL, *key = NULL, *value = NULL, *include_directory = NULL;

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], D_ARG, strlen(D_ARG))) {
			if (argv[i][strlen(D_ARG)] == '\0') {
				symbol_mapping = argv[++i];

				if (strchr(symbol_mapping, '=') != NULL) {
					key = strtok(symbol_mapping, "=");
					value = strtok(NULL, "=");

					rc = put(hash_table, key, value);
					if (rc == 12)
						return 12;
				} else {
					rc = put(hash_table, symbol_mapping, "");
					if (rc == 12)
						return 12;
				}
			} else {
				symbol_mapping = argv[i];

				symbol_mapping += 2;
				if (strchr(symbol_mapping, '=') != NULL) {
					key = strtok(symbol_mapping, "=");
					value = strtok(NULL, "=");

					rc = put(hash_table, key, value);
					if (rc == 12)
						return 12;
				} else {
					rc = put(hash_table, symbol_mapping, "");
					if (rc == 12)
						return 12;
				}
			}
		} else if (!strncmp(argv[i], I_ARG, strlen(I_ARG))) {
			if (argv[i][strlen(I_ARG)] == '\0') {
				include_directory = argv[++i];

				rc = append(include_directories, include_directory);
				if (rc == 12)
					return 12;
			}
		} else if (*input_file == stdin) {
			*input_file = fopen(argv[i], "r");
			if (*input_file == NULL)
				return 12;
		} else if (*output_file == stdout) {
			*output_file = fopen(argv[i], "w+");
			if (*output_file == NULL)
				return 12;
		} else {
			return 12;
		}
	}

	return 0;
}

int parse_multi_line_define(char **multi_line_define_value, char **multi_line_define_key, char *buffer, THashTable *hash_table)
{
	int add_to_hash_map = 0, rc;
	char *multi_line_new_value = (char *)calloc(strlen(*multi_line_define_value) + strlen(buffer) + 1, sizeof(char));

	if (multi_line_new_value == NULL)
		return 12;
	memcpy(multi_line_new_value, *multi_line_define_value, strlen(*multi_line_define_value));
	if (buffer[strlen(buffer) - 2] == '\\')
		memset(buffer + strlen(buffer) - 2, 0, 2);
	else {
		buffer[strlen(buffer) - 1] = '\0';
		add_to_hash_map = 1;
	}
	strcat(multi_line_new_value, buffer);
	remove_extra_spaces(multi_line_new_value);
	free(*multi_line_define_value);
	*multi_line_define_value = multi_line_new_value;
	if (add_to_hash_map) {
		rc = put(hash_table, *multi_line_define_key, *multi_line_define_value);
		free(*multi_line_define_value);
		free(*multi_line_define_key);
		*multi_line_define_key = NULL;
		if (rc == 12)
			return 12;
	}

	return 0;
}

FILE *get_include_file(char *buffer, Vector *include_directories)
{
	unsigned int i;
	char *file_name = strchr(buffer, ' ');
	char *file_path = NULL;
	FILE *include_file = NULL;

	file_name += 2;
	memset(file_name + strlen(file_name) - 2, 0, 2);
	file_path = (char *)calloc(strlen(INPUT_DIR) + strlen(file_name) + 1, sizeof(char));

	if (file_path == NULL)
		return NULL;
	strcpy(file_path, INPUT_DIR);
	strcat(file_path, file_name);
	include_file = fopen(file_path, "r");

	if (include_file == NULL) {
		free(file_path);
		for (i = 0; i < include_directories->length; i++) {
			file_path = (char *)calloc(strlen(include_directories->values[i]) + strlen(file_name) + 2, sizeof(char));
			if (file_path == NULL)
				return NULL;
			strcpy(file_path, include_directories->values[i]);
			strcat(file_path, "/");
			strcat(file_path, file_name);
			include_file = fopen(file_path, "r");
			if (include_file != NULL)
				break;
		}

		if (include_file == NULL && include_directories->length != 0) {
			free(file_path);
			return NULL;
		} else if (include_file == NULL)
			return NULL;
	}
	free(file_path);
	return include_file;
}

int parse_define(char *buffer, char **multi_line_define_key, char **multi_line_define_value, THashTable *hash_table)
{
	char *key_value_mapping = strchr(buffer, ' ');
	int rc;
	int changed = 1;
	char *key_value_mapping_copy = NULL, *key = NULL, *value = NULL;

	key_value_mapping++;
	key_value_mapping_copy = (char *)calloc(strlen(key_value_mapping), sizeof(char));

	if (key_value_mapping_copy == NULL)
		return 12;
	memcpy(key_value_mapping_copy, key_value_mapping, strlen(key_value_mapping));
	key = strtok(key_value_mapping_copy, DELIMITERS);
	value = strchr(key_value_mapping, ' ');

	if (value) {
		value++;
		value[strlen(value) - 1] = '\0';
		if (value[strlen(value) - 1] == '\\') {
			*multi_line_define_key = (char *)calloc(strlen(key) + 1, sizeof(char));
			if (*multi_line_define_key == NULL) {
				free(key_value_mapping_copy);
				return 12;
			}
			*multi_line_define_value = (char *)calloc(strlen(value) + 1, sizeof(char));
			if (*multi_line_define_value == NULL) {
				free(*multi_line_define_key);
				free(key_value_mapping_copy);
				return 12;
			}
			memcpy(*multi_line_define_key, key, strlen(key));
			value[strlen(value) - 1] = '\0';
			memcpy(*multi_line_define_value, value, strlen(value));
			free(key_value_mapping_copy);
			return 0;
		}

		value = parse_value(hash_table, value, &changed);
		if (!value) {
			free(key_value_mapping_copy);
			return 12;
		}
		rc = put(hash_table, key, value);
		free(key_value_mapping_copy);
		if (changed)
			free(value);
		if (rc == -1)
			return 12;
	} else {
		rc = put(hash_table, key, "");
		free(key_value_mapping_copy);
		if (rc == 12)
			return 12;
	}

	return 0;
}

int preprocess_line(char *buffer, THashTable *hash_table, FILE *output_file)
{
	char *replace_buffer = NULL;
	char buffer_copy[BUFSIZE];
	char *token = NULL;
	char *hash_table_value = NULL;
	char *new_replace_buffer = NULL;

	memset(buffer_copy, 0, BUFSIZE);
	memcpy(buffer_copy, buffer, BUFSIZE);
	// check if something should be changed in the line of code
	token = strtok(buffer_copy, DELIMITERS);

	while (token)  {
		hash_table_value = get(hash_table, token);

		if (hash_table_value != NULL) {
			// replace the word in the text
			if (replace_buffer == NULL)
				replace_buffer = replace_string(buffer, token, hash_table_value);
			else {
				new_replace_buffer = replace_string(replace_buffer, token, hash_table_value);
				free(replace_buffer);
				replace_buffer = new_replace_buffer;
			}
			if (replace_buffer == NULL)
				return 12;
		}
		token = strtok(NULL, DELIMITERS);
	}
	if (replace_buffer) {
		fprintf(output_file, "%s", replace_buffer);
		free(replace_buffer);
	} else
		fprintf(output_file, "%s", buffer);

	return 0;
}

int parse_if_elif_line(char *buffer, int *stop_parsing_if_branch, int *parse_if_branch, int *inside_if_branch, THashTable *hash_table)
{
	char *cond = NULL;
	char *cond_value = NULL;
	int eval_cond;

	if (!*stop_parsing_if_branch && !*parse_if_branch) {
		cond = strchr(buffer, ' ');

		cond[strlen(cond) - 1] = '\0';
		cond++;
		cond_value = get(hash_table, cond);

		if (cond_value != NULL)
			cond = cond_value;
		eval_cond = atoi(cond);

		*inside_if_branch = 1;
		*parse_if_branch = eval_cond == 0 ? 0 : 1;
	} else
		*stop_parsing_if_branch = 1;
	return 0;
}

int preprocess_code(FILE *input_file, FILE *output_file, THashTable *hash_table, Vector *include_directories)
{
	char buffer[BUFSIZE];
	int rc;
	int inside_if_branch = 0, parse_if_branch = 0, stop_parsing_if_branch = 0;
	char *multi_line_define_key = NULL;
	char *multi_line_define_value = NULL;
	char *symbol = NULL;
	char *symbol_value = NULL;
	char *undefined_key = NULL;

	memset(buffer, 0, BUFSIZE);
	while (fgets(buffer, BUFSIZE, input_file)) {

		if (buffer[0] != '\n') {
			if (multi_line_define_key) {
				rc = parse_multi_line_define(&multi_line_define_value, &multi_line_define_key, buffer, hash_table);
				if (rc == 12)
					return 12;
			} else if (!strncmp(buffer, INCLUDE, strlen(INCLUDE)) && ((inside_if_branch && parse_if_branch) || (!inside_if_branch && !parse_if_branch))) {
				FILE *include_file = get_include_file(buffer, include_directories);

				if (include_file == NULL)
					return 12;
				rc = preprocess_code(include_file, output_file, hash_table, include_directories);
				fclose(include_file);
				if (rc == 12)
					return 12;
			} else if (!strncmp(buffer, DEFINE, strlen(DEFINE)) && ((inside_if_branch && parse_if_branch) || (!inside_if_branch && !parse_if_branch))) {
				rc = parse_define(buffer, &multi_line_define_key, &multi_line_define_value, hash_table);
				if (rc == 12)
					return 12;
			} else if (!strncmp(buffer, IFDEF, strlen(IFDEF))) {
				symbol = strchr(buffer, ' ');

				symbol[strlen(symbol) - 1] = '\0';
				symbol++;
				symbol_value = get(hash_table, symbol);

				inside_if_branch = 1;
				if (symbol_value)
					parse_if_branch = 1;
				else
					parse_if_branch = 0;
			} else if (!strncmp(buffer, IFNDEF, strlen(IFNDEF))) {
				symbol = strchr(buffer, ' ');

				symbol[strlen(symbol) - 1] = '\0';
				symbol++;
				symbol_value = get(hash_table, symbol);

				inside_if_branch = 1;
				if (!symbol_value)
					parse_if_branch = 1;
				else
					parse_if_branch = 0;
			} else if (!strncmp(buffer, IF, strlen(IF)) || !strncmp(buffer, ELIF, strlen(ELIF))) {
				parse_if_elif_line(buffer, &stop_parsing_if_branch, &parse_if_branch, &inside_if_branch, hash_table);
			} else if (!strncmp(buffer, ELSE, strlen(ELSE))) {
				if (!stop_parsing_if_branch)
					parse_if_branch = parse_if_branch == 1 ? 0 : 1;
			} else if (!strncmp(buffer, ENDIF, strlen(ENDIF))) {
				inside_if_branch = 0;
				parse_if_branch = 0;
				stop_parsing_if_branch = 0;
			} else if (!strncmp(buffer, UNDEF, strlen(UNDEF)) && ((inside_if_branch && parse_if_branch) || (!inside_if_branch && !parse_if_branch))) {
				strtok(buffer, DELIMITERS);
				undefined_key = strtok(NULL, DELIMITERS);

				remove_key(hash_table, undefined_key);
			} else if ((inside_if_branch && parse_if_branch) || (!inside_if_branch && !parse_if_branch)) {
				rc = preprocess_line(buffer, hash_table, output_file);
				if (rc == 12)
					return 12;
			}
			memset(buffer, 0, BUFSIZE);
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	THashTable *hash_table = NULL;
	Vector *include_directories = NULL;
	char buffer[BUFSIZE];
	int rc;
	FILE *input_file = stdin;
	FILE *output_file = stdout;

	hash_table = create_hash_table();

	if (hash_table == NULL)
		return 12;

	include_directories = create_vector(5);

	if (include_directories == NULL) {
		destroy_hash_table(hash_table);
		return 12;
	}

	memset(buffer, 0, BUFSIZE);

	rc = parse_arguments(hash_table, &input_file, &output_file, argc, argv, include_directories);

	if (rc == 12) {
		if (input_file != NULL && input_file != stdin)
			fclose(input_file);
		if (output_file != NULL && output_file != stdout)
			fclose(output_file);
		destroy_hash_table(hash_table);
		destroy_vector(include_directories);
		return 12;
	}

	rc = preprocess_code(input_file, output_file, hash_table, include_directories);
	destroy_vector(include_directories);
	destroy_hash_table(hash_table);
	fclose(input_file);
	fclose(output_file);
	return rc;
}
