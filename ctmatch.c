#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "match.h"

// this struct will hold match score for each line
typedef struct {
  char   *line;
  double  score;
} matchresult_t;

// match results comparing func
int matchresult_comp_func(const void *a, const void *b) {
  double score_a = ((matchresult_t*)a)->score;
  double score_b = ((matchresult_t*)b)->score;

  if (score_a == score_b)
    return 0;
  else
    return score_a < score_b ? +1 : -1; /* sort in reverse order */
}

int main(int argc, char ** argv) {
  char   *line             = 0;
  size_t  read             = 0;
  size_t  max_line_len     = 0;
  size_t  current_line_len = 0;
  char   *abbrev           = argv[1];

  size_t         results_buf_len = 2000;
  matchresult_t *results_buf     = malloc(results_buf_len * sizeof(matchresult_t));
  size_t         results_count   = 0;

  printf("Search pattern: %s\n", abbrev);

  while ((read = getline(&line, &current_line_len, stdin)) != -1) {
    matchinfo_t matchinfo = {
      line,
      strlen(line),
      abbrev,
      strlen(abbrev),
      1.0, 0, 0, 1
    };
    double  score     = 0;
    char   *line_copy = 0;

    // chomp newline at the end of filename
    line[read - 1] = 0;

    // if getline call expanded line buffer, remember new buffer size
    if (current_line_len > max_line_len)
      max_line_len = current_line_len;

    // next iteration of while will reuse line buffer
    current_line_len = max_line_len;

    score = recursive_match(&matchinfo, 0, 0, 0, 0.0);

    // realloc results_buf if needed
    if (results_buf_len == results_count)
      results_buf = realloc(results_buf, (results_buf_len *= 2));

    // make copy of line
    line_copy = malloc(read + 1);
    memcpy(line_copy, line, read + 1);

    // add result to results_buf
    results_buf[results_count].line = line_copy;
    results_buf[results_count].score = score;
    results_count++;
  }

  // Sorting results
  qsort(results_buf, results_count, sizeof(matchresult_t), matchresult_comp_func);

  // Print sorted results
  for (size_t i = 0; i < results_count; i++)
    printf("%f: %s\n", results_buf[i].score, results_buf[i].line);

  // Cleanup
  free(line);

  for (size_t i = 0; i < results_count; i++)
    free(results_buf[i].line);

  free(results_buf);

  exit(EXIT_SUCCESS);
}
