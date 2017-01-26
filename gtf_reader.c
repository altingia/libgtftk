/*
 * gtf_reader.c
 *
 *  Created on: Jan 10, 2017
 *      Author: fafa
 *
 *  Contains the functions for GTF file reading :
 *  	get_next_gtf_line:	reads a line from input (plain or gzipped GTF file, or standard input)
 *  	get_gtf_reader:		create an appropriate GTF reader depending on the query
 */

#include "libgtftk.h"

/*
 * Returns the next row of text from the file pointed by gr (a gzipped file or a flat file),
 * or NULL if the EOF has been reached
 */
char *get_next_gtf_line(GTF_READER *gr, char *buffer) {
	char *ret = NULL;

	if (gr->gz) {
		if (gzgets(gr->gzfile, buffer, 10000) != Z_NULL)
			ret = buffer;
	}
	else {
		if (fgets(buffer, 10000, gr->plainfile) != NULL)
			ret = buffer;
	}
	return ret;
}

/*
 * This function analyzes "query" and constructs a GTF_READER object containing :
 *  - the absolute name of the GTF file (which can be gzipped)
 *  - the kind of file (.gz or .gtf)
 *  - the FILE ou gzFile pointer to read it
 *
 * "query" can be :
 *  - a local .gtf or .gtf.gz file (absolute path)
 *  - a file in ~/.gtftk/
 */
GTF_READER *get_gtf_reader(char *query) {
	GTF_READER *gr = (GTF_READER *)calloc(1, sizeof(GTF_READER));
	char *tmp = (char *)calloc(1000, sizeof(char)), *query_filename;

	if (access(query, F_OK) && strcmp(query, "-")) {
		// query is not a local file
		strcpy(tmp, getenv("HOME"));
		strcat(tmp, "/.gtftk/");
		strcat(tmp, query);
		if (access(tmp, F_OK)) {
			// query is not a file in ~/.gtftk
			query_filename = NULL;

			// query is not valid
			perror(query);
		}
		else
			// query is a file in ~/.gtftk
			query_filename = strdup(tmp);
	}
	else
		// query is a valid local file
		query_filename = strdup(query);

	if (query_filename != NULL) {
		// here we got a valid local file in query_filename
		if (strstr(query_filename, ".gtf.gz")) {
			// gz file
			gr->filename = strdup(query_filename);
			gr->gz = 1;
			gr->gzfile = gzopen(gr->filename, "r");
			gr->plainfile = NULL;
		}
		else if (strstr(query_filename, ".gtf")) {
			// plain text file
			gr->filename = strdup(query_filename);
			gr->plainfile = fopen(gr->filename, "ro");
			gr->gzfile = NULL;
			gr->gz = 0;
		}
		else if (!strcmp(query_filename, "-")) {
			// query is standard input
			gr->plainfile = stdin;
			gr->gzfile = NULL;
			gr->filename = strdup("-");
			gr->gz = 0;
		}
		else {
			// not a usable file
			perror(query_filename);
		}
	}
	else {
		free(gr);
		gr = NULL;
	}
	free(tmp);
	return gr;
}
