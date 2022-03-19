#include "utils.h"

void remove_extra_spaces(char *str)
{
	int length = strlen(str);
	int i, j = 0;

	for (i = 0; i < length; i++) {
		if (str[i] != ' ' || (i < length - 1 && str[i + 1] != ' '))
			str[j++] = str[i];
	}

	memset(str + j, 0, length - j);
}

char *replace_string(char *haystack, char *old_word, char *new_word)
{
	int length = strlen(haystack);
	int i, number_of_appearances = 0, between_quotes = 0;
	char *result = (char *)calloc(1 + strlen(haystack) + number_of_appearances * (strlen(new_word) - strlen(old_word)), sizeof(char));

	for (i = 0; i < length; i++) {
		if (haystack[i] == '"')
			between_quotes = between_quotes == 1 ? 0 : 1;
		// the word that we want to replace is at the beginning of the text
		// so a word is found
		if (strstr(haystack + i, old_word) == haystack + i && !between_quotes) {
			number_of_appearances++;
			// jump the found word
			i += strlen(old_word);
		}
	}

	if (result == NULL)
		return NULL;

	i = 0;
	between_quotes = 0;
	while (*haystack) {
		// if the new word is found in the haystack replace it
		if (*haystack == '"')
			between_quotes = between_quotes == 1 ? 0 : 1;
		if (strstr(haystack, old_word) == haystack && !between_quotes) {
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

char *my_strdup(char *s)
{
	char *s_dup = (char *)calloc(strlen(s) + 1, sizeof(char));

	if (s_dup == NULL)
		return NULL;

	memcpy(s_dup, s, strlen(s) + 1);
	return s_dup;
}
