#ifndef COMMON_FILTER_H_
#define COMMON_FILTER_H_

#include "common/image_io.h"

typedef enum {
  FILTER_KIND_MEDIAN = 0,
  FILTER_KIND_BILATERAL = 1,
  FILTER_KIND_CNN = 2,
} FilterKind;

typedef struct {
  FilterKind kind;
  int median_radius;
} FilterConfig;

void filter_default_config(FilterConfig* config);
int filter_apply(const ImageBuffer* input, ImageBuffer* output, const FilterConfig* config);
int filter_apply_median(const ImageBuffer* input, ImageBuffer* output, int radius);
int filter_apply_bilateral(const ImageBuffer* input, ImageBuffer* output);
int filter_apply_cnn(const ImageBuffer* input, ImageBuffer* output);

#endif
