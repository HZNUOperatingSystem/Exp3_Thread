#include "common/filter.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

static FilterCnnImpl g_filter_cnn_impl = NULL;

static int clamp_int(int value, int low, int high) {
  if (value < low) {
    return low;
  }
  if (value > high) {
    return high;
  }
  return value;
}

static void sort_window(unsigned char* values, int count) {
  int i;
  int j;

  for (i = 1; i < count; ++i) {
    unsigned char current = values[i];
    j = i - 1;
    while (j >= 0 && values[j] > current) {
      values[j + 1] = values[j];
      --j;
    }
    values[j + 1] = current;
  }
}

static double intensity_value(const unsigned char* pixel, int channels) {
  if (channels == 1 || channels == 2) {
    return (double)pixel[0];
  }

  return 0.299 * (double)pixel[0] + 0.587 * (double)pixel[1] + 0.114 * (double)pixel[2];
}

void filter_default_config(FilterConfig* config) {
  if (config == NULL) {
    return;
  }

  config->kind = FILTER_KIND_MEDIAN;
  config->median_radius = 1;
}

void filter_set_cnn_impl(FilterCnnImpl impl) {
  g_filter_cnn_impl = impl;
}

int filter_apply(const ImageBuffer* input, ImageBuffer* output, const FilterConfig* config) {
  FilterConfig local_config;

  if (config == NULL) {
    filter_default_config(&local_config);
    config = &local_config;
  }

  if (config->kind == FILTER_KIND_MEDIAN) {
    return filter_apply_median(input, output, config->median_radius);
  }
  if (config->kind == FILTER_KIND_BILATERAL) {
    return filter_apply_bilateral(input, output);
  }
  if (config->kind == FILTER_KIND_CNN) {
    return filter_apply_cnn(input, output);
  }

  return -1;
}

int filter_apply_median(const ImageBuffer* input, ImageBuffer* output, int radius) {
  int x;
  int y;
  int channel;
  int window_width;
  int window_capacity;
  unsigned char* window_values;

  if (input == NULL || output == NULL || input->data == NULL || output->data == NULL ||
      input->width != output->width || input->height != output->height ||
      input->channels != output->channels) {
    return -1;
  }

  if (radius < 1) {
    radius = 1;
  }

  window_width = radius * 2 + 1;
  window_capacity = window_width * window_width;
  window_values = (unsigned char*)malloc((size_t)window_capacity);
  if (window_values == NULL) {
    return -1;
  }

  for (y = 0; y < input->height; ++y) {
    for (x = 0; x < input->width; ++x) {
      for (channel = 0; channel < input->channels; ++channel) {
        int dy;
        int count = 0;
        size_t output_offset =
            ((size_t)y * (size_t)input->width + (size_t)x) * (size_t)input->channels +
            (size_t)channel;

        for (dy = -radius; dy <= radius; ++dy) {
          int dx;
          int ny = clamp_int(y + dy, 0, input->height - 1);
          for (dx = -radius; dx <= radius; ++dx) {
            int nx = clamp_int(x + dx, 0, input->width - 1);
            size_t input_offset =
                ((size_t)ny * (size_t)input->width + (size_t)nx) * (size_t)input->channels +
                (size_t)channel;
            window_values[count++] = input->data[input_offset];
          }
        }

        sort_window(window_values, count);
        output->data[output_offset] = window_values[count / 2];
      }
    }
  }

  free(window_values);
  return 0;
}

int filter_apply_bilateral(const ImageBuffer* input, ImageBuffer* output) {
  const int radius = 2;
  const int window_width = radius * 2 + 1;
  const double sigma_space = 1.5;
  const double sigma_range = 30.0;
  double spatial_weights[25];
  int dy;
  int dx;

  if (input == NULL || output == NULL || input->data == NULL || output->data == NULL ||
      input->width != output->width || input->height != output->height ||
      input->channels != output->channels || input->channels <= 0 || input->channels > 4) {
    return -1;
  }

  for (dy = -radius; dy <= radius; ++dy) {
    for (dx = -radius; dx <= radius; ++dx) {
      double distance2 = (double)(dx * dx + dy * dy);
      size_t weight_index = (size_t)(dy + radius) * (size_t)window_width + (size_t)(dx + radius);
      spatial_weights[weight_index] = exp(-distance2 / (2.0 * sigma_space * sigma_space));
    }
  }

  for (int y = 0; y < input->height; ++y) {
    for (int x = 0; x < input->width; ++x) {
      size_t center_offset =
          ((size_t)y * (size_t)input->width + (size_t)x) * (size_t)input->channels;
      double accum[4] = {0.0, 0.0, 0.0, 0.0};
      double total_weight = 0.0;
      double center_intensity = intensity_value(input->data + center_offset, input->channels);

      for (dy = -radius; dy <= radius; ++dy) {
        int ny = clamp_int(y + dy, 0, input->height - 1);

        for (dx = -radius; dx <= radius; ++dx) {
          int nx = clamp_int(x + dx, 0, input->width - 1);
          size_t neighbor_offset =
              ((size_t)ny * (size_t)input->width + (size_t)nx) * (size_t)input->channels;
          size_t weight_index =
              (size_t)(dy + radius) * (size_t)window_width + (size_t)(dx + radius);
          double neighbor_intensity = intensity_value(input->data + neighbor_offset, input->channels);
          double range_diff = neighbor_intensity - center_intensity;
          double range_weight = exp(-(range_diff * range_diff) / (2.0 * sigma_range * sigma_range));
          double weight = spatial_weights[weight_index] * range_weight;

          total_weight += weight;
          for (int channel = 0; channel < input->channels; ++channel) {
            accum[channel] += weight * (double)input->data[neighbor_offset + (size_t)channel];
          }
        }
      }

      if (total_weight == 0.0) {
        for (int channel = 0; channel < input->channels; ++channel) {
          output->data[center_offset + (size_t)channel] = input->data[center_offset + (size_t)channel];
        }
        continue;
      }

      for (int channel = 0; channel < input->channels; ++channel) {
        double filtered = accum[channel] / total_weight;
        if (filtered < 0.0) {
          filtered = 0.0;
        } else if (filtered > 255.0) {
          filtered = 255.0;
        }
        output->data[center_offset + (size_t)channel] = (unsigned char)(filtered + 0.5);
      }
    }
  }

  return 0;
}

int filter_apply_cnn(const ImageBuffer* input, ImageBuffer* output) {
  if (g_filter_cnn_impl == NULL) {
    return -1;
  }

  return g_filter_cnn_impl(input, output);
}
