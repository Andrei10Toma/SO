#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "so_stdio.h"
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 4096
#define SO_STDIN_FILENO 0
#define SO_STDOUT_FILENO 1

typedef enum {
	READ, WRITE
} Operation;

struct _so_file {
	int fd;
	int cursor;
	Operation last_operation;
	int eof;
	pid_t child_pid;

	unsigned char read_write_append_flags;
	char read_buffer[BUFSIZE];
	unsigned int read_buffer_length;
	unsigned int read_buffer_offset;
	int read_error;

	char write_buffer[BUFSIZE];
	unsigned int write_buffer_offset;
	int write_error;
};

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	int fd;
	SO_FILE *file = (SO_FILE *)calloc(1, sizeof(SO_FILE));

	if (file == NULL)
		return NULL;
	if (!strcmp(mode, "r")) {
		fd = open(pathname, O_RDONLY);
		file->read_write_append_flags |= O_RDONLY;
	} else if (!strcmp(mode, "r+")) {
		fd = open(pathname, O_RDWR);
		file->read_write_append_flags |= O_RDWR;
	} else if (!strcmp(mode, "w")) {
		fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
		file->read_write_append_flags |= O_WRONLY | O_CREAT | O_TRUNC;
	} else if (!strcmp(mode, "w+")) {
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC);
		file->read_write_append_flags |= O_RDWR | O_CREAT | O_TRUNC;
	} else if (!strcmp(mode, "a")) {
		fd = open(pathname, O_WRONLY | O_CREAT | O_APPEND);
		file->read_write_append_flags |= O_WRONLY | O_CREAT | O_APPEND;
	} else if (!strcmp(mode, "a+")) {
		fd = open(pathname, O_RDWR | O_CREAT | O_APPEND);
		file->read_write_append_flags |= O_RDWR | O_CREAT | O_APPEND;
	} else {
		free(file);
		return NULL;
	}
	if (fd < 0) {
		free(file);
		return NULL;
	}
	file->fd = fd;
	memset(file->read_buffer, 0, BUFSIZE);
	memset(file->write_buffer, 0, BUFSIZE);
	return file;
}

int so_fclose(SO_FILE *stream)
{
	int rc = 0;

	if (stream == NULL)
		return SO_EOF;
	if (stream->write_buffer_offset > 0)
		rc = so_fflush(stream);
	if (rc < 0) {
		close(stream->fd);
		free(stream);
		return SO_EOF;
	}
	rc = close(stream->fd);
	free(stream);
	return rc;
}

int so_fileno(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;
	return stream->fd;
}

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

	rc = lseek(stream->fd, offset, whence);
	if (rc < 0)
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

static int fill_read_buffer(SO_FILE *stream)
{
	ssize_t bytes_read;

	memset(stream->read_buffer, 0, BUFSIZE);
	stream->read_buffer_length = 0;
	stream->read_buffer_offset = 0;
	bytes_read = read(stream->fd, stream->read_buffer, BUFSIZE);

	if (bytes_read < 0)
		return SO_EOF;
	if (bytes_read == 0) {
		stream->eof = SO_EOF;
		return SO_EOF;
	}
	stream->read_buffer_length = bytes_read;
	return 0;
}

static int empty_write_buffer(SO_FILE *stream)
{
	int rc;
	unsigned int write_information_size = 0;

	while (write_information_size < stream->write_buffer_offset) {
		rc = write(stream->fd, stream->write_buffer + write_information_size, stream->write_buffer_offset - write_information_size);
		if (rc < 0)
			return SO_EOF;
		write_information_size += rc;
	}

	memset(stream->write_buffer, 0, stream->write_buffer_offset);
	stream->write_buffer_offset = 0;
	return 0;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int rc;
	size_t characters_read = 0;

	if (ptr == NULL)
		return 0;

	while (characters_read != nmemb * size) {
		rc = so_fgetc(stream);
		if (stream->read_error == 1)
			return characters_read / size;
		memcpy(ptr + characters_read, &rc, sizeof(char));
		characters_read++;
	}

	return nmemb;
}

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

	if (stream == NULL || (((stream->read_write_append_flags & O_RDONLY) != O_RDONLY) && ((stream->read_write_append_flags & O_RDWR) != O_RDWR)))
		return SO_EOF;

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

int so_fputc(int c, SO_FILE *stream)
{
	int rc;

	if (stream == NULL || (((stream->read_write_append_flags & O_WRONLY) != O_WRONLY) && ((stream->read_write_append_flags & O_RDWR) != O_RDWR)))
		return SO_EOF;

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

int so_feof(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	return stream->eof;
}

int so_ferror(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	return (stream->read_error == 1 || stream->write_error == 1) ? SO_EOF : 0;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	int pipe_fds[2], rc;
	pid_t pid;
	SO_FILE *file;

	rc = pipe(pipe_fds);
	if (rc < 0)
		return NULL;
	pid = fork();
	switch (pid) {
	case -1:
		return NULL;
	case 0:
		if (!strcmp(type, "r")) {
			close(pipe_fds[0]);
			rc = dup2(pipe_fds[1], SO_STDOUT_FILENO);
			close(pipe_fds[1]);
			if (rc < 0)
				return NULL;
		} else if (!strcmp(type, "w")) {
			close(pipe_fds[1]);
			rc = dup2(pipe_fds[0], SO_STDIN_FILENO);
			close(pipe_fds[0]);
			if (rc < 0)
				return NULL;
		}
		const char *arguments[4] = { "sh", "-c", command, NULL };

		execvp("sh", (char * const *)arguments);
		return NULL;
	default:
		file = (SO_FILE *)calloc(1, sizeof(SO_FILE));
		if (file == NULL)
			return NULL;
		if (!strcmp(type, "r")) {
			close(pipe_fds[1]);
			file->fd = pipe_fds[0];
			file->read_write_append_flags |= O_RDONLY;
			memset(file->read_buffer, 0, BUFSIZE);
		} else if (!strcmp(type, "w")) {
			close(pipe_fds[0]);
			file->fd = pipe_fds[1];
			file->read_write_append_flags |= O_WRONLY;
			memset(file->write_buffer, 0, BUFSIZE);
		}
		file->child_pid = pid;
		return file;
	}
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	int rc;
	pid_t pid = stream->child_pid;

	rc = so_fflush(stream);
	close(stream->fd);
	free(stream);
	rc = waitpid(pid, NULL, 0);
	if (rc < 0)
		return -1;
	return 0;
}
