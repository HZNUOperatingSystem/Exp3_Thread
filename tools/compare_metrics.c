#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  METRICS_FIELD_COUNT = 6,
  LINE_BUFFER_SIZE = 1024,
};

static int split_csv_line(char* line, char* fields[], int field_count) {
  int count = 0;
  char* token = strtok(line, ",\r\n");

  while (token != NULL && count < field_count) {
    fields[count++] = token;
    token = strtok(NULL, ",\r\n");
  }

  return count;
}

static int compare_numeric_field(const char* actual, const char* expected, double tolerance) {
  char* actual_end = NULL;
  char* expected_end = NULL;
  double actual_value;
  double expected_value;

  if (strcmp(actual, "N/A") == 0 || strcmp(expected, "N/A") == 0) {
    return strcmp(actual, expected) == 0 ? 0 : -1;
  }

  actual_value = strtod(actual, &actual_end);
  expected_value = strtod(expected, &expected_end);
  if (actual_end == actual || expected_end == expected || *actual_end != '\0' ||
      *expected_end != '\0') {
    return -1;
  }

  return fabs(actual_value - expected_value) <= tolerance ? 0 : -1;
}

static int compare_metrics_files(const char* actual_path, const char* expected_path,
                                 double tolerance) {
  FILE* actual = fopen(actual_path, "r");
  FILE* expected = fopen(expected_path, "r");
  char actual_line[LINE_BUFFER_SIZE];
  char expected_line[LINE_BUFFER_SIZE];
  int line_number = 0;

  if (actual == NULL || expected == NULL) {
    fclose(actual);
    fclose(expected);
    return -1;
  }

  while (fgets(actual_line, sizeof(actual_line), actual) != NULL &&
         fgets(expected_line, sizeof(expected_line), expected) != NULL) {
    char* actual_fields[METRICS_FIELD_COUNT];
    char* expected_fields[METRICS_FIELD_COUNT];
    int i;

    line_number += 1;

    if (line_number == 1) {
      if (strcmp(actual_line, expected_line) != 0) {
        fclose(actual);
        fclose(expected);
        return -1;
      }
      continue;
    }

    if (split_csv_line(actual_line, actual_fields, METRICS_FIELD_COUNT) != METRICS_FIELD_COUNT ||
        split_csv_line(expected_line, expected_fields, METRICS_FIELD_COUNT) !=
            METRICS_FIELD_COUNT) {
      fclose(actual);
      fclose(expected);
      return -1;
    }

    if (strcmp(actual_fields[0], expected_fields[0]) != 0 ||
        strcmp(actual_fields[5], expected_fields[5]) != 0) {
      fclose(actual);
      fclose(expected);
      return -1;
    }

    for (i = 1; i <= 4; ++i) {
      if (compare_numeric_field(actual_fields[i], expected_fields[i], tolerance) != 0) {
        fclose(actual);
        fclose(expected);
        return -1;
      }
    }
  }

  if ((fgets(actual_line, sizeof(actual_line), actual) != NULL) ||
      (fgets(expected_line, sizeof(expected_line), expected) != NULL)) {
    fclose(actual);
    fclose(expected);
    return -1;
  }

  fclose(actual);
  fclose(expected);
  return 0;
}

int main(int argc, char** argv) {
  double tolerance;
  char* tolerance_end = NULL;

  if (argc != 4) {
    fprintf(stderr, "usage: %s ACTUAL_CSV EXPECTED_CSV TOLERANCE\n", argv[0]);
    return 1;
  }

  tolerance = strtod(argv[3], &tolerance_end);
  if (tolerance_end == argv[3] || *tolerance_end != '\0' || tolerance < 0.0) {
    fprintf(stderr, "invalid tolerance: %s\n", argv[3]);
    return 1;
  }

  return compare_metrics_files(argv[1], argv[2], tolerance) == 0 ? 0 : 1;
}
