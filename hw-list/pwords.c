/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

/*
 * main - handle command line, spawning one thread per file.
 */

typedef struct{
  char* file_path;
  word_count_t* word_counts;
} thread_data_t; 

void* count_file_words(void* arg){
    thread_data_t* data = (thread_data_t*)arg; 
    FILE* fp = fopen(data->file_path, "r");
    count_words(data->word_counts, fp);
    fclose(fp);
    free(data);
    pthread_exit(NULL);
} 


int main(int argc, char* argv[]) {
  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else {
    int num_files = argc - 1;
    pthread_t* threads = malloc(num_files * sizeof(pthread_t));
    if (threads == NULL) {
        fprintf(stderr, "Malloc failed for threads array\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_files; i++) {
      thread_data_t* data = malloc(sizeof(thread_data_t));
      if (data == NULL) {
        fprintf(stderr, "Malloc failed for thread %d\n", i);
        free(threads);
        exit(EXIT_FAILURE);
        }
      data->file_path = argv[i + 1];
      data->word_counts = &word_counts;
      int rc = pthread_create(&threads[i], NULL, count_file_words, (void*)data);
      if (rc != 0) {
          fprintf(stderr, "Failed to create thread %d: %d\n", i, rc);
          free(data);
          free(threads);
          exit(EXIT_FAILURE);
      }
    }

    for (int i = 0; i < num_files; i++) {
      pthread_join(threads[i], NULL);
    }
    free(threads);
  }

  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  return 0;
}
