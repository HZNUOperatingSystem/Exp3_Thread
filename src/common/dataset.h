#ifndef COMMON_DATASET_H_
#define COMMON_DATASET_H_

#define MAX_IMAGE_JOBS 64

typedef struct {
  char name[128];
  char input_path[256];
  char gt_path[256];
  char output_path[256];
} ImageJob;

int dataset_load_jobs(const char* list_path, const char* input_dir, const char* gt_dir,
                      const char* output_dir, ImageJob jobs[], int max_jobs);
int dataset_ensure_directory(const char* path);

#endif
