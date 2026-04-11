#include "common/pipeline.h"

#include <stdlib.h>
#include <stdio.h>

#include "common/image_io.h"
#include "common/metrics.h"

enum {
  PIPELINE_STATUS_OK = 0,
  PIPELINE_STATUS_LOAD = 1,
  PIPELINE_STATUS_SHAPE = 2,
  PIPELINE_STATUS_FILTER = 3,
  PIPELINE_STATUS_SAVE = 4,
  PIPELINE_STATUS_METRICS = 5,
};

static int pipeline_has_failures(const ImageResult results[], int count) {
  int i;

  for (i = 0; i < count; ++i) {
    if (results[i].status_code != 0) {
      return 1;
    }
  }

  return 0;
}

static const char* pipeline_current_chapter(void) {
  const char* chapter = getenv("LAB_CHAPTER");

  if (chapter == NULL || chapter[0] == '\0') {
    fprintf(stderr, "LAB_CHAPTER is not set\n");
    return NULL;
  }

  return chapter;
}

static void pipeline_reset_result(ImageResult* result) {
  result->psnr_before = 0.0;
  result->psnr_after = 0.0;
  result->ssim_before = 0.0;
  result->ssim_after = 0.0;
  result->ssim_available = 0;
  result->status_code = PIPELINE_STATUS_OK;
}

int pipeline_process_one_image(const ImageJob* job, const FilterConfig* config, int compute_ssim,
                               ImageResult* out_result) {
  ImageBuffer input = {0, 0, 0, NULL};
  ImageBuffer gt = {0, 0, 0, NULL};
  ImageBuffer output = {0, 0, 0, NULL};

  if (job == NULL || config == NULL || out_result == NULL) {
    return -1;
  }

  pipeline_reset_result(out_result);

  if (image_load_png(job->input_path, &input) != 0 || image_load_png(job->gt_path, &gt) != 0) {
    out_result->status_code = PIPELINE_STATUS_LOAD;
    image_free(&input);
    image_free(&gt);
    return -1;
  }

  if (input.width != gt.width || input.height != gt.height || input.channels != gt.channels) {
    out_result->status_code = PIPELINE_STATUS_SHAPE;
    image_free(&input);
    image_free(&gt);
    return -1;
  }

  if (image_allocate_like(&input, &output) != 0) {
    out_result->status_code = PIPELINE_STATUS_LOAD;
    image_free(&input);
    image_free(&gt);
    return -1;
  }

  metrics_compute_psnr(&input, &gt, &out_result->psnr_before);
  if (compute_ssim) {
    if (metrics_compute_ssim(&input, &gt, &out_result->ssim_before) != 0) {
      out_result->status_code = PIPELINE_STATUS_METRICS;
      image_free(&input);
      image_free(&gt);
      image_free(&output);
      return -1;
    }
  }

  if (filter_apply(&input, &output, config) != 0) {
    out_result->status_code = PIPELINE_STATUS_FILTER;
    image_free(&input);
    image_free(&gt);
    image_free(&output);
    return -1;
  }

  if (image_save_png(job->output_path, &output) != 0) {
    out_result->status_code = PIPELINE_STATUS_SAVE;
    image_free(&input);
    image_free(&gt);
    image_free(&output);
    return -1;
  }

  metrics_compute_psnr(&output, &gt, &out_result->psnr_after);
  if (compute_ssim) {
    if (metrics_compute_ssim(&output, &gt, &out_result->ssim_after) != 0) {
      out_result->status_code = PIPELINE_STATUS_METRICS;
      image_free(&input);
      image_free(&gt);
      image_free(&output);
      return -1;
    }
    out_result->ssim_available = 1;
  }

  image_free(&input);
  image_free(&gt);
  image_free(&output);
  return 0;
}

int pipeline_run_image_batch(ImageBatchExecutor executor) {
  static const char* list_path = "image/list.txt";
  static const char* input_dir = "image/input";
  static const char* gt_dir = "image/gt";
  static const char* output_root = "output";
  const char* chapter;
  char output_dir[96];
  char metrics_path[128];
  ImageJob jobs[MAX_IMAGE_JOBS];
  ImageResult results[MAX_IMAGE_JOBS] = {0};
  FilterConfig config;
  int job_count;

  if (executor == NULL) {
    return 1;
  }

  chapter = pipeline_current_chapter();
  if (chapter == NULL) {
    return 1;
  }

  snprintf(output_dir, sizeof(output_dir), "%s/%s", output_root, chapter);
  snprintf(metrics_path, sizeof(metrics_path), "%s/%s/metrics.csv", output_root, chapter);

  filter_default_config(&config);

  job_count = dataset_load_jobs(list_path, input_dir, gt_dir, output_dir, jobs, MAX_IMAGE_JOBS);
  if (job_count <= 0) {
    fprintf(stderr, "failed to load jobs from %s\n", list_path);
    return 1;
  }

  if (dataset_ensure_directory(output_root) != 0 || dataset_ensure_directory(output_dir) != 0) {
    fprintf(stderr, "failed to create output directories\n");
    return 1;
  }

  if (executor(jobs, &config, results, job_count) != 0) {
    return 1;
  }

  if (pipeline_write_metrics_csv(metrics_path, jobs, results, job_count) != 0) {
    fprintf(stderr, "failed to write %s\n", metrics_path);
    return 1;
  }

  return pipeline_has_failures(results, job_count);
}

int pipeline_write_metrics_csv(const char* path, const ImageJob jobs[], const ImageResult results[],
                               int count) {
  FILE* file;
  int i;

  if (path == NULL || jobs == NULL || results == NULL || count < 0) {
    return -1;
  }

  file = fopen(path, "w");
  if (file == NULL) {
    return -1;
  }

  fprintf(file, "name,psnr_before,psnr_after,ssim_before,ssim_after,status_code\n");
  for (i = 0; i < count; ++i) {
    if (results[i].ssim_available) {
      fprintf(file, "%s,%.6f,%.6f,%.6f,%.6f,%d\n", jobs[i].name, results[i].psnr_before,
              results[i].psnr_after, results[i].ssim_before, results[i].ssim_after,
              results[i].status_code);
    } else {
      fprintf(file, "%s,%.6f,%.6f,N/A,N/A,%d\n", jobs[i].name, results[i].psnr_before,
              results[i].psnr_after, results[i].status_code);
    }
  }

  fclose(file);
  return 0;
}
