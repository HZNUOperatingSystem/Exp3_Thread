#include "common/dataset.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void trim_line(char* line) {
  size_t length;
  size_t start = 0;

  length = strlen(line);
  while (length > 0 && isspace((unsigned char)line[length - 1])) {
    line[--length] = '\0';
  }

  while (line[start] != '\0' && isspace((unsigned char)line[start])) {
    ++start;
  }

  if (start > 0) {
    memmove(line, line + start, length - start + 1);
  }
}

int dataset_load_jobs(const char* list_path,
                      const char* input_dir,
                      const char* gt_dir,
                      const char* output_dir,
                      ImageJob jobs[],
                      int max_jobs) {
  FILE* file;
  char line[256];
  int count = 0;

  if (list_path == NULL || input_dir == NULL || gt_dir == NULL || output_dir == NULL ||
      jobs == NULL || max_jobs <= 0) {
    return -1;
  }

  file = fopen(list_path, "r");
  if (file == NULL) {
    return -1;
  }

  while (fgets(line, sizeof(line), file) != NULL) {
    trim_line(line);
    if (line[0] == '\0' || line[0] == '#') {
      continue;
    }
    if (count >= max_jobs) {
      fclose(file);
      return -1;
    }

    snprintf(jobs[count].name, sizeof(jobs[count].name), "%s", line);
    snprintf(jobs[count].input_path, sizeof(jobs[count].input_path), "%s/%s", input_dir, line);
    snprintf(jobs[count].gt_path, sizeof(jobs[count].gt_path), "%s/%s", gt_dir, line);
    snprintf(jobs[count].output_path, sizeof(jobs[count].output_path), "%s/%s", output_dir,
             line);
    ++count;
  }

  fclose(file);
  return count;
}

int dataset_ensure_directory(const char* path) {
  struct stat st;

  if (path == NULL) {
    return -1;
  }

  if (stat(path, &st) == 0) {
    return S_ISDIR(st.st_mode) ? 0 : -1;
  }

  return mkdir(path, 0755);
}
