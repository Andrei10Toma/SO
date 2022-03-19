#include <stdlib.h>
#include <string.h>

typedef struct {
	char **values;
	unsigned int length;
	unsigned int capacity;
} Vector;

Vector *create_vector(int capacity);

int append(Vector *v, char *value);

void destroy_vector(Vector *v);
