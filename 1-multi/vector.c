#include "vector.h"

Vector *create_vector(int capacity)
{
	Vector *vector = (Vector *)calloc(1, sizeof(Vector));

	if (vector == NULL)
		return NULL;

	vector->capacity = capacity;
	vector->length = 0;
	vector->values = (char **)calloc(capacity, sizeof(char *));
	if (vector->values == NULL) {
		free(vector);
		return NULL;
	}

	return vector;
}

// double the maximum capacity of the vector
static int resize(Vector *v)
{
	char **values = realloc(v->values, 2 * v->capacity * sizeof(char *));

	if (values == NULL)
		return 12;

	v->values = values;
	v->capacity *= 2;
	return 0;
}

int append(Vector *v, char *value)
{
	if (v->capacity == v->length)
		if (resize(v) == 12)
			return 12;
	v->values[v->length++] = (char *)
		calloc(strlen(value) + 1, sizeof(char));
	if (v->values[v->length - 1] == NULL)
		return 12;
	memcpy(v->values[v->length - 1], value, strlen(value));
	return 0;
}

void destroy_vector(Vector *v)
{
	unsigned int i;

	for (i = 0; i < v->length; i++)
		free(v->values[i]);
	free(v->values);
	free(v);
}
