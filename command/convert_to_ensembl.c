/*
 * convert_to_ensembl.c
 *
 *  Created on: May 18, 2017
 *      Author: fafa
 *
 *  The convert_to_ensembl function add "gene" and "transcript" rows if not
 *  present to conform to Ensembl GTF format
 */

#include "libgtftk.h"

/*
 * external functions declaration
 */
extern INDEX_ID *index_gtf(GTF_DATA *gtf_data, char *key);
extern char *get_attribute_value(GTF_ROW *row, char *attr);
extern int update_row_table(GTF_DATA *gtf_data);
extern void add_attribute(GTF_ROW *row, char *key, char *value);
extern GTF_DATA *clone(GTF_DATA *gtf_data);

/*
 * global variables declaration
 */
extern COLUMN **column;
INDEX_ID *tid_index, *gid_index;
int n, nbrow;
GTF_DATA *gtf_d;

static void action_transcript(const void *nodep, const VISIT which, const int depth) {
	int i, ok, start, end, k;
	char *feature;
	GTF_ROW *row, *tr_row;

	// the row list of the current transcript
	ROW_LIST *datap = *((ROW_LIST **)nodep);

	switch (which) {
		case preorder:
			break;
		/*
		 * The operations are made on internal nodes and leaves of the tree.
		 */
		case leaf :
		case postorder:
			ok = 0;
			row = NULL;
			for (i = 0; i < datap->nb_row; i++) {
				// the current row
				row = gtf_d->data[datap->row[i]];

				// the current feature value
				feature = row->field[2];
				if (!strcmp(feature, "transcript")) {
					ok = 1;
					break;
				}
			}
			if (!ok) {
				tr_row = (GTF_ROW *)calloc(1, sizeof(GTF_ROW));
				tr_row->rank = -1;
				tr_row->field = (char **)calloc(8, sizeof(char *));
				start = INT_MAX;
				end = 0;
				for (i = 0; i < datap->nb_row; i++) {
					// the current row
					row = gtf_d->data[datap->row[i]];

					// the current feature value
					feature = row->field[2];

					k = atoi(row->field[3]);
					if (k < start) start = k;
					k = atoi(row->field[4]);
					if (k > end) end = k;

					if (!ok)
						if (!strcmp(feature, "exon")) {
							for (k = 0; k < row->attributes.nb; k++)
								if (strncmp(row->attributes.attr[k]->key, "exon", 4))
									add_attribute(tr_row, row->attributes.attr[k]->key, row->attributes.attr[k]->value);
							tr_row->field[0] = strdup(row->field[0]);
							tr_row->field[1] = get_attribute_value(row, "transcript_source");
							if (tr_row->field[1] == NULL) tr_row->field[1] = strdup(row->field[1]);
							tr_row->field[2] = strdup("transcript");
							tr_row->field[5] = strdup(row->field[5]);
							tr_row->field[6] = strdup(row->field[6]);
							tr_row->field[7] = strdup(row->field[7]);
							ok = 1;
							nbrow++;
						}
				}
				asprintf(&(tr_row->field[3]), "%d", start);
				asprintf(&(tr_row->field[4]), "%d", end);

				if (!strcmp(gtf_d->data[datap->row[0]]->field[2], "gene")) {
					tr_row->next = gtf_d->data[datap->row[1]];
					gtf_d->data[datap->row[0]]->next = tr_row;
				}
				else {
					tr_row->next = gtf_d->data[datap->row[0]];
					if (datap->row[0] != 0)
						gtf_d->data[datap->row[0] - 1]->next = tr_row;
					else
						gtf_d->data[0] = tr_row;
				}
			}
			n++;
			break;
		case endorder:
			break;
	}
}

static void action_gene(const void *nodep, const VISIT which, const int depth) {
	int i, ok, start, end, k;
	char *feature;
	GTF_ROW *row, *g_row;

	// the row list of the current gene
	ROW_LIST *datap = *((ROW_LIST **)nodep);

	switch (which) {
		case preorder:
			break;
		/*
		 * The operations are made on internal nodes and leaves of the tree.
		 */
		case leaf :
		case postorder:
			ok = 0;
			row = NULL;
			for (i = 0; i < datap->nb_row; i++) {
				// the current row
				row = gtf_d->data[datap->row[i]];

				// the current feature value
				feature = row->field[2];
				if (!strcmp(feature, "gene")) {
					ok = 1;
					break;
				}
			}
			if (!ok) {
				g_row = (GTF_ROW *)calloc(1, sizeof(GTF_ROW));
				g_row->rank = -1;
				g_row->field = (char **)calloc(8, sizeof(char *));
				start = INT_MAX;
				end = 0;
				for (i = 0; i < datap->nb_row; i++) {
					// the current row
					row = gtf_d->data[datap->row[i]];

					// the current feature value
					feature = row->field[2];

					k = atoi(row->field[3]);
					if (k < start) start = k;
					k = atoi(row->field[4]);
					if (k > end) end = k;

					if (!strcmp(feature, "exon") || !strcmp(feature, "transcript")) {
						if (!ok) {
							for (k = 0; k < row->attributes.nb; k++)
								if (!strncmp(row->attributes.attr[k]->key, "gene", 4) ||
										strstr(row->attributes.attr[k]->key, "_gene_") ||
										!strncmp(row->attributes.attr[k]->key + strlen(row->attributes.attr[k]->key) - 5, "_gene", 5))
									add_attribute(g_row, row->attributes.attr[k]->key, row->attributes.attr[k]->value);
							g_row->field[0] = strdup(row->field[0]);
							g_row->field[1] = get_attribute_value(row, "gene_source");
							if (g_row->field[1] == NULL) g_row->field[1] = strdup(row->field[1]);
							g_row->field[2] = strdup("gene");
							g_row->field[5] = strdup(row->field[5]);
							g_row->field[6] = strdup(row->field[6]);
							g_row->field[7] = strdup(row->field[7]);
							ok = 1;
							nbrow++;
						}
					}
				}
				asprintf(&(g_row->field[3]), "%d", start);
				asprintf(&(g_row->field[4]), "%d", end);

				if (datap->row[0] != 0)	gtf_d->data[datap->row[0] - 1]->next = g_row;
				g_row->next = gtf_d->data[datap->row[0]];
				if (datap->row[0] == 0)	gtf_d->data[0] = g_row;
			}
			n++;
			break;
		case endorder:
			break;
	}
}

__attribute__ ((visibility ("default")))
GTF_DATA *convert_to_ensembl(GTF_DATA *gtf_data) {
	/*
	 * reserve memory for the GTF_DATA structure to return
	 */
	GTF_DATA *ret = clone(gtf_data);

	/*
	 * indexing the gtf with transcript_id
	 */
	tid_index = index_gtf(ret, "transcript_id");

	/*
	 * tree browsing of the transcript_id index (rank 0)
	 */
	gtf_d = ret;
	n = nbrow = 0;
	twalk(column[tid_index->column]->index[tid_index->index_rank].data, action_transcript);

	ret->size = nbrow + ret->size;
	update_row_table(ret);

	/*
	 * indexing the gtf with gene_id
	 */
	gid_index = index_gtf(ret, "gene_id");

	/*
	 * tree browsing of the gene_id index (rank 1)
	 */
	n = nbrow = 0;
	twalk(column[gid_index->column]->index[gid_index->index_rank].data, action_gene);

	ret->size += nbrow;
	update_row_table(ret);

	return ret;
}
