#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

struct BilateralFilterParams {
  int radius = 3;
  double sigma_space = 3.0;
  double sigma_range = 25.0;
};

namespace bilateral_filter_detail {

inline int clamp_int(int value, int low, int high) {
  return std::max(low, std::min(value, high));
}

inline std::size_t pixel_offset(int x, int y, int width, int channels) {
  return static_cast<std::size_t>(y) * static_cast<std::size_t>(width) *
             static_cast<std::size_t>(channels) +
         static_cast<std::size_t>(x) * static_cast<std::size_t>(channels);
}

inline double color_distance_squared(const std::uint8_t* lhs, const std::uint8_t* rhs,
                                     int channels) {
  double sum = 0.0;
  for (int channel = 0; channel < channels; ++channel) {
    const double diff = static_cast<double>(lhs[channel]) - static_cast<double>(rhs[channel]);
    sum += diff * diff;
  }
  return sum;
}

inline std::uint8_t round_to_byte(double value) {
  if (value <= 0.0) {
    return 0;
  }
  if (value >= 255.0) {
    return 255;
  }
  return static_cast<std::uint8_t>(value + 0.5);
}

} // namespace bilateral_filter_detail

inline void bilateral_filter_rows(const std::uint8_t* src, std::uint8_t* dst, int width, int height,
                                  int channels, int y_begin, int y_end,
                                  const BilateralFilterParams& params) {
  if (src == nullptr || dst == nullptr || width <= 0 || height <= 0 || channels <= 0) {
    return;
  }
  if (params.radius < 0 || params.sigma_space <= 0.0 || params.sigma_range <= 0.0) {
    return;
  }

  y_begin = bilateral_filter_detail::clamp_int(y_begin, 0, height);
  y_end = bilateral_filter_detail::clamp_int(y_end, 0, height);
  if (y_begin >= y_end) {
    return;
  }

  const int radius = params.radius;
  const int kernel_width = radius * 2 + 1;
  const double spatial_scale = 2.0 * params.sigma_space * params.sigma_space;
  const double range_scale = 2.0 * params.sigma_range * params.sigma_range;

  std::vector<double> spatial_weights(static_cast<std::size_t>(kernel_width) *
                                      static_cast<std::size_t>(kernel_width));
  for (int dy = -radius; dy <= radius; ++dy) {
    for (int dx = -radius; dx <= radius; ++dx) {
      const double distance_squared = static_cast<double>(dx * dx + dy * dy);
      const std::size_t index =
          static_cast<std::size_t>(dy + radius) * static_cast<std::size_t>(kernel_width) +
          static_cast<std::size_t>(dx + radius);
      spatial_weights[index] = std::exp(-distance_squared / spatial_scale);
    }
  }

  std::vector<double> weighted_sum(static_cast<std::size_t>(channels), 0.0);

  for (int y = y_begin; y < y_end; ++y) {
    for (int x = 0; x < width; ++x) {
      std::fill(weighted_sum.begin(), weighted_sum.end(), 0.0);
      double total_weight = 0.0;

      const std::size_t center_offset =
          bilateral_filter_detail::pixel_offset(x, y, width, channels);
      const std::uint8_t* center_pixel = src + center_offset;

      for (int dy = -radius; dy <= radius; ++dy) {
        const int ny = y + dy;
        if (ny < 0 || ny >= height) {
          continue;
        }

        for (int dx = -radius; dx <= radius; ++dx) {
          const int nx = x + dx;
          if (nx < 0 || nx >= width) {
            continue;
          }

          const std::size_t neighbor_offset =
              bilateral_filter_detail::pixel_offset(nx, ny, width, channels);
          const std::uint8_t* neighbor_pixel = src + neighbor_offset;
          const std::size_t weight_index =
              static_cast<std::size_t>(dy + radius) * static_cast<std::size_t>(kernel_width) +
              static_cast<std::size_t>(dx + radius);
          const double spatial_weight = spatial_weights[weight_index];
          const double range_weight = std::exp(-bilateral_filter_detail::color_distance_squared(
                                                   center_pixel, neighbor_pixel, channels) /
                                               range_scale);
          const double weight = spatial_weight * range_weight;

          for (int channel = 0; channel < channels; ++channel) {
            weighted_sum[static_cast<std::size_t>(channel)] +=
                weight * static_cast<double>(neighbor_pixel[channel]);
          }
          total_weight += weight;
        }
      }

      if (total_weight == 0.0) {
        for (int channel = 0; channel < channels; ++channel) {
          dst[center_offset + static_cast<std::size_t>(channel)] = center_pixel[channel];
        }
        continue;
      }

      for (int channel = 0; channel < channels; ++channel) {
        dst[center_offset + static_cast<std::size_t>(channel)] =
            bilateral_filter_detail::round_to_byte(weighted_sum[static_cast<std::size_t>(channel)] /
                                                   total_weight);
      }
    }
  }
}

inline void bilateral_filter_image(const std::uint8_t* src, std::uint8_t* dst, int width,
                                   int height, int channels, const BilateralFilterParams& params) {
  bilateral_filter_rows(src, dst, width, height, channels, 0, height, params);
}
