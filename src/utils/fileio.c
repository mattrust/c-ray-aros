//
//  fileio.c
//  C-ray
//
//  Created by Valtteri Koskivuori on 28/02/2015.
//  Copyright © 2015-2022 Valtteri Koskivuori. All rights reserved.
//

#include "../includes.h"
#include "fileio.h"
#include "../utils/logging.h"
#include "assert.h"
#ifndef WINDOWS
#include <libgen.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "string.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "filecache.h"
#include "textbuffer.h"
#include "args.h"

static char *getFileExtension(const char *fileName) {
	char buf[LINEBUFFER_MAXSIZE];
	lineBuffer line = { .buf = buf };
	fillLineBuffer(&line, fileName, '.');
	if (line.amountOf.tokens != 2) {
		return NULL;
	}
	char *extension = stringCopy(lastToken(&line));
	return extension;
}

enum fileType guessFileType(const char *filePath) {
	char *fileName = getFileName(filePath);
	char *extension = getFileExtension(fileName);
	char *lower = stringToLower(extension);
	free(extension);
	extension = lower;
	free(fileName);
	
	if (!extension) return unknown;
	
	enum fileType type = unknown;
	if (stringEquals(extension, "bmp"))
		type = bmp;
	if (stringEquals(extension, "png"))
		type = png;
	if (stringEquals(extension, "hdr"))
		type = hdr;
	if (stringEquals(extension, "obj"))
		type = obj;
	if (stringEquals(extension, "mtl"))
		type = mtl;
	if (stringEquals(extension, "jpg"))
		type = jpg;
	if (stringEquals(extension, "tiff"))
		type = tiff;
	if (stringEquals(extension, "qoi"))
		type = qoi;
	if (stringEquals(extension, "gltf"))
		type = gltf;
	if (stringEquals(extension, "glb"))
		type = glb;
	free(extension);
	
	return type;
}

char *loadFile(const char *filePath, size_t *bytes, struct file_cache *cache) {
	if (cache && cache_contains(cache, filePath)) return cache_load(cache, filePath, bytes);
	FILE *file = fopen(filePath, "rb");
	if (!file) {
		logr(warning, "Can't access '%.*s': %s\n", (int)strlen(filePath), filePath, strerror(errno));
		return NULL;
	}
	size_t fileBytes = getFileSize(filePath);
	if (!fileBytes) {
		fclose(file);
		return NULL;
	}
	char *buf = malloc(fileBytes + 1 * sizeof(char));
	size_t readBytes = fread(buf, sizeof(char), fileBytes, file);
	ASSERT(readBytes == fileBytes);
	if (ferror(file) != 0) {
		logr(warning, "Error reading file\n");
	} else {
		buf[fileBytes] = '\0';
	}
	fclose(file);
	if (bytes) *bytes = readBytes;
	if (cache) cache_store(cache, filePath, buf, readBytes);
	return buf;
}

void writeFile(const unsigned char *buf, size_t bufsize, const char *filePath) {
	FILE *file = fopen(filePath, "wb" );
	char *backupPath = NULL;
	if(!file) {
		char *name = getFileName(filePath);
#ifdef __AROS__
		backupPath = stringConcat("", name);
#else
		backupPath = stringConcat("./", name);
#endif
		free(name);
		file = fopen(backupPath, "wb");
		if (file) {
			char *path = getFilePath(filePath);
			logr(warning, "The specified output directory \"%s\" was not writeable, dumping the file in CWD instead.\n", path);
			free(path);
		} else {
			logr(warning, "Neither the specified output directory nor the current working directory were writeable. Image can't be saved. Fix your permissions!");
			return;
		}
	}
	logr(info, "Saving result in %s\'%s\'%s\n", KGRN, backupPath ? backupPath : filePath, KNRM);
	fwrite(buf, 1, bufsize, file);
	fclose(file);
	
	//We determine the file size after saving, because the lodePNG library doesn't have a way to tell the compressed file size
	//This will work for all image formats
	unsigned long bytes = getFileSize(backupPath ? backupPath : filePath);
	char *sizeString = humanFileSize(bytes);
	logr(info, "Wrote %s to file.\n", sizeString);
	free(sizeString);
}


bool isValidFile(char *path, struct file_cache *cache) {
	if (!isSet("use_clustering") && cache) return cache_contains(cache, path);
#if !defined(WINDOWS) && !defined(__AROS__)
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
#else
	FILE *f = fopen(path, "r");
	if (f) {
		fclose(f);
		return true;
	}
	return false;
#endif
}

void wait_for_stdin(int seconds) {
#if !defined(WINDOWS) && !defined(__AROS__)
	fd_set set;
	struct timeval timeout;
	int rv;
	FD_ZERO(&set);
	FD_SET(0, &set);
	timeout.tv_sec = seconds;
	timeout.tv_usec = 1000;
	rv = select(1, &set, NULL, NULL, &timeout);
	if (rv == -1) {
		logr(error, "Error on stdin timeout\n");
	} else if (rv == 0) {
		logr(error, "No input found after %i seconds. Hint: Try `./bin/c-ray input/scene.json`.\n", seconds);
	} else {
		return;
	}
#endif
}

/**
 Extract the filename from a given file path

 @param input File path to be processed
 @return Filename string, including file type extension
 */
//FIXME: Just return a pointer to the first byte of the filename? Why do we do all this
char *getFileName(const char *input) {
	//FIXME: We're doing two copies here, maybe just rework the algorithm instead.
	char *copy = stringCopy(input);
	char *fn;
	
	/* handle trailing '/' e.g.
	 input == "/home/me/myprogram/" */
	if (copy[(strlen(copy) - 1)] == '/')
		copy[(strlen(copy) - 1)] = '\0';
	
	(fn = strrchr(copy, '/')) ? ++fn : (fn = copy);
	
	char *ret = stringCopy(fn);
	free(copy);
	
	return ret;
}

//For Windows
#define CRAY_PATH_MAX 4096

char *getFilePath(const char *input) {
	char *dir = NULL;
#ifdef WINDOWS
	dir = calloc(_MAX_DIR, sizeof(*dir));
	_splitpath_s(input, NULL, 0, dir, _MAX_DIR, NULL, 0, NULL, 0);
	return dir;
#else
	char *inputCopy = stringCopy(input);
	dir = stringCopy(dirname(inputCopy));
	free(inputCopy);
	char *final = stringConcat(dir, "/");
	free(dir);
	return final;
#endif
}

#define chunksize 65536
//Get scene data from stdin and return a pointer to it
char *read_stdin(size_t *bytes) {
	wait_for_stdin(2);
	
	char chunk[chunksize];
	
	size_t buf_size = 0;
	char *buf = NULL;
	int stdin_fd = fileno(stdin);
	int read_bytes = 0;
	while ((read_bytes = read(stdin_fd, &chunk, chunksize)) > 0) {
		char *old = buf;
		buf = realloc(buf, buf_size + read_bytes + 1);
		if (!buf) {
			logr(error, "Failed to realloc stdin buffer\n");
			free(old);
			return NULL;
		}
		memcpy(buf + buf_size, chunk, read_bytes);
		buf_size += read_bytes;
	}
	
	if (ferror(stdin)) {
		logr(error, "Failed to read from stdin\n");
		free(buf);
		return NULL;
	}
	
	if (bytes) *bytes = buf_size - 1;
	buf[buf_size ] = 0;
	return buf;
}

char *humanFileSize(unsigned long bytes) {
	float kilobytes, megabytes, gigabytes, terabytes, petabytes, exabytes, zettabytes, yottabytes; // <- Futureproofing?!
	kilobytes  = bytes      / 1000.0f;
	megabytes  = kilobytes  / 1000.0f;
	gigabytes  = megabytes  / 1000.0f;
	terabytes  = gigabytes  / 1000.0f;
	petabytes  = terabytes  / 1000.0f;
	exabytes   = petabytes  / 1000.0f;
	zettabytes = exabytes   / 1000.0f;
	yottabytes = zettabytes / 1000.0f;
	
	// Okay, okay. In reality, this never gets even close to a zettabyte,
	// it'll overflow at around 18 exabytes.
	// I *did* get it to go to yottabytes using __uint128_t, but that's
	// not in C99. Maybe in the future.
	
	char *buf = calloc(64, sizeof(*buf));
	
	if (zettabytes >= 1000) {
		sprintf(buf, "%.02fYB", yottabytes);
	} else if (exabytes >= 1000) {
		sprintf(buf, "%.02fZB", zettabytes);
	} else if (petabytes >= 1000) {
		sprintf(buf, "%.02fEB", exabytes);
	} else if (terabytes >= 1000) {
		sprintf(buf, "%.02fPB", petabytes);
	} else if (gigabytes >= 1000) {
		sprintf(buf, "%.02fTB", terabytes);
	} else if (megabytes >= 1000) {
		sprintf(buf, "%.02fGB", gigabytes);
	} else if (kilobytes >= 1000) {
		sprintf(buf, "%.02fMB", megabytes);
	} else if (bytes >= 1000) {
		sprintf(buf, "%.02fkB", kilobytes);
	} else {
		sprintf(buf, "%ldB", bytes);
	}
	return buf;
}

size_t getFileSize(const char *fileName) {
	FILE *file = fopen(fileName, "r");
	if (!file) return 0;
	fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);
	fclose(file);
	return size;
}
