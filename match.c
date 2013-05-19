// Matcher code is copied from Vim Command-T plugin
// http://github.com/wincent/Command-T
// Original copyright below


// Copyright 2010 Wincent Colaiuta. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "match.h"

double recursive_match(matchinfo_t *m,  // sharable meta-data
                       long str_idx,    // where in the path string to start
                       long abbrev_idx, // where in the search string to start
                       long last_idx,   // location of last matched character
                       double score)    // cumulative score so far
{
  double seen_score = 0;      // remember best score seen via recursion
  int dot_file_match = 0;     // true if abbrev matches a dot-file
  int dot_search = 0;         // true if searching for a dot

  for (long i = abbrev_idx; i < m->abbrev_len; i++) {
    char c = m->abbrev_p[i];
    if (c == '.')
      dot_search = 1;
    int found = 0;
    for (long j = str_idx; j < m->str_len; j++, str_idx++) {
      char d = m->str_p[j];
      if (d == '.') {
        if (j == 0 || m->str_p[j - 1] == '/') {
          m->dot_file = 1;        // this is a dot-file
          if (dot_search)         // and we are searching for a dot
            dot_file_match = 1; // so this must be a match
        }
      }
      else if (d >= 'A' && d <= 'Z')
        d += 'a' - 'A'; // add 32 to downcase
      if (c == d) {
        found = 1;
        dot_search = 0;

        // calculate score
        double score_for_char = m->max_score_per_char;
        long distance = j - last_idx;
        if (distance > 1) {
          double factor = 1.0;
          char last = m->str_p[j - 1];
          char curr = m->str_p[j]; // case matters, so get again
          if (last == '/')
            factor = 0.9;
          else if (last == '-' ||
                   last == '_' ||
                   last == ' ' ||
                   (last >= '0' && last <= '9'))
            factor = 0.8;
          else if (last >= 'a' && last <= 'z' &&
                   curr >= 'A' && curr <= 'Z')
            factor = 0.8;
          else if (last == '.')
            factor = 0.7;
          else
            // if no "special" chars behind char, factor diminishes
            // as distance from last matched char increases
            factor = (1.0 / distance) * 0.75;
          score_for_char *= factor;
        }

        if (++j < m->str_len) {
          // bump cursor one char to the right and
          // use recursion to try and find a better match
          double sub_score = recursive_match(m, j, i, last_idx, score);
          if (sub_score > seen_score)
            seen_score = sub_score;
        }

        score += score_for_char;
        last_idx = str_idx++;
        break;
      }
    }
    if (!found)
      return 0.0;
  }
  if (m->dot_file) {
    if (m->never_show_dot_files ||
        (!dot_file_match && !m->always_show_dot_files))
      return 0.0;
  }
  return (score > seen_score) ? score : seen_score;
}
