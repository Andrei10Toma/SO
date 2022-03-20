#include <stdlib.h>
#include <string.h>

typedef struct {
	char **values;
	unsigned int length;
	unsigned int capacity;
} Vector;

// allocate memory for a vector
Vector *create_vector(int capacity);

// append a value at the end of the vector
int append(Vector *v, char *value);

// free the allocated memory for the vector
void destroy_vector(Vector *v);
