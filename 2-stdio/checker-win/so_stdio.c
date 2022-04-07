#include <stdlib.h>
#include <Windows.h>
#include <string.h>
#include "so_stdio.h"

#define BUFSIZE 4096
#define SO_STDIN_FILENO 0
#define SO_STDOUT_FILENO 1

typedef enum {
	READ, WRITE
} Operation;

struct _so_file {
	HANDLE fd; /* file descriptor */
	int cursor; /* file cursor */
	/* last operation performed on the file */
	Operation last_operation;
	int eof; /*set if the end of file is reached */

	char read_buffer[BUFSIZE]; /* used for read buffering */
	unsigned int read_buffer_length; /* read buffer length */
	/* current position in the read buffer */
	unsigned int read_buffer_offset;
	int read_error; /* set if an error occurs while reading */

	char write_buffer[BUFSIZE]; /* used for write buffering */
	/* current position in the write buffer */
	unsigned int write_buffer_offset;
	int write_error; /* set if an error occurs while writing */
};

/* Allocate memory for a file structure and initialize the fields */
SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	HANDLE fd;
	SO_FILE *file = (SO_FILE *)calloc(1, sizeof(SO_FILE));

	if (file == NULL)
		return NULL;
	if (!strcmp(mode, "r")) {
		fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	} else if (!strcmp(mode, "r+")) {
		fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	} else if (!strcmp(mode, "w")) {
		fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	} else if (!strcmp(mode, "w+")) {
		fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	} else if (!strcmp(mode, "a")) {
		fd = CreateFile(pathname,
			FILE_APPEND_DATA,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	} else if (!strcmp(mode, "a+")) {
		fd = CreateFile(pathname,
			FILE_APPEND_DATA | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	} else {
		free(file);
		return NULL;
	}
	if (fd == INVALID_HANDLE_VALUE) {
		free(file);
		return NULL;
	}
	file->fd = fd;
	memset(file->read_buffer, 0, BUFSIZE);
	memset(file->write_buffer, 0, BUFSIZE);
	return file;
}

/* Close the file and free the used memory for the structure */
int so_fclose(SO_FILE *stream)
{
	int rc = 0;

	if (stream == NULL)
		return SO_EOF;
	if (stream->write_buffer_offset > 0)
		rc = so_fflush(stream);
	if (rc < 0) {
		CloseHandle(stream->fd);
		free(stream);
		return SO_EOF;
	}
	rc = CloseHandle(stream->fd);
	free(stream);
	if (rc == 0)
		return SO_EOF;
	return 0;
}

/* Return the file descriptor value */
HANDLE so_fileno(SO_FILE *stream)
{
	if (stream == NULL)
		return (HANDLE) SO_EOF;
	return stream->fd;
}

/* Move the file cursor in the file */
int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int rc;

	if (stream == NULL)
		return SO_EOF;

	if (stream->last_operation == READ) {
		memset(stream->read_buffer, 0, stream->read_buffer_length);
		stream->read_buffer_length = 0;
		stream->read_buffer_offset = 0;
	}

	if (stream->last_operation == WRITE)
		so_fflush(stream);
	rc = SetFilePointer(stream->fd, offset, NULL, whence);
	if (rc == INVALID_SET_FILE_POINTER)
		return SO_EOF;
	stream->cursor = rc;
	return 0;
}

long so_ftell(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;
	return stream->cursor;
}

/* Fill the read buffer for buffering read operations */
static int fill_read_buffer(SO_FILE *stream)
{
	size_t bytes_read;

	memset(stream->read_buffer, 0, BUFSIZE);
	stream->read_buffer_length = 0;
	stream->read_buffer_offset = 0;
	ReadFile(stream->fd,
		stream->read_buffer,
		BUFSIZE,
		&bytes_read,
		NULL);

	if (bytes_read < 0)
		return SO_EOF;
	if (bytes_read == 0) {
		stream->eof = SO_EOF;
		return SO_EOF;
	}
	stream->read_buffer_length = bytes_read;
	return 0;
}

/* Write the contents of the write in the openend file */
static int empty_write_buffer(SO_FILE *stream)
{
	int rc = 0;
	unsigned int write_information_size = 0;
	unsigned int write_size;

	while (write_information_size < stream->write_buffer_offset) {
		rc = WriteFile(stream->fd,
			stream->write_buffer + write_information_size,
			stream->write_buffer_offset - write_information_size,
			&write_size,
			NULL);
		if (rc == 0)
			return SO_EOF;
		write_information_size += write_size;
	}

	memset(stream->write_buffer, 0, stream->write_buffer_offset);
	stream->write_buffer_offset = 0;
	return 0;
}

/* Read from the buffer the given number of bytes */
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int rc;
	size_t characters_read = 0;
	char *use_ptr = (char *)ptr;

	if (ptr == NULL)
		return 0;

	while (characters_read != nmemb * size) {
		rc = so_fgetc(stream);
		if (stream->read_error == 1)
			return characters_read / size;
		memcpy(use_ptr + characters_read, &rc, sizeof(char));
		characters_read++;
	}

	return nmemb;
}

/* Write in the buffer the given number of bytes */
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	char *buf = (char *) ptr;
	size_t characters_written = 0;

	if (ptr == NULL || stream == NULL)
		return 0;
	while (characters_written != nmemb * size) {
		so_fputc(buf[characters_written], stream);
		if (stream->write_error == 1)
			return 0;
		characters_written++;
	}

	return nmemb;
}

int so_fgetc(SO_FILE *stream)
{
	int rc;
	unsigned char character;

	rc = so_feof(stream);
	if (rc == SO_EOF)
		return SO_EOF;

	stream->last_operation = READ;
	if (stream->read_buffer_length <= stream->read_buffer_offset) {
		rc = fill_read_buffer(stream);
		if (rc < 0) {
			stream->read_error = 1;
			return SO_EOF;
		}
	}

	character = stream->read_buffer[stream->read_buffer_offset];
	stream->read_buffer_offset++;
	stream->cursor++;
	return (int) character;
}

/* Put a character in the write buffer, if it is full flush the buffer */
int so_fputc(int c, SO_FILE *stream)
{
	int rc;

	stream->last_operation = WRITE;

	if (stream->write_buffer_offset == BUFSIZE) {
		rc = empty_write_buffer(stream);
		if (rc < 0) {
			stream->write_error = 1;
			return SO_EOF;
		}
	}

	stream->write_buffer[stream->write_buffer_offset] = c;
	stream->write_buffer_offset++;
	stream->cursor++;
	return c;
}

/* Flush the write buffer */
int so_fflush(SO_FILE *stream)
{
	int rc;

	if (stream == NULL)
		return SO_EOF;

	rc = empty_write_buffer(stream);
	if (rc < 0)
		return SO_EOF;
	return 0;
}

/* Return -1 if the end of the file was reached */
int so_feof(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	return stream->eof;
}

/* Return -1 if an error occured during writing or reading */
int so_ferror(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	return (stream->read_error == 1 || 
		stream->write_error == 1) ? SO_EOF : 0;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	return 0;
}
