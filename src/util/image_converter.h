#ifndef STELLA_VSLAM_UTIL_IMAGE_CONVERTER_H
#define STELLA_VSLAM_UTIL_IMAGE_CONVERTER_H

enum class color_order_t {
    Gray = 0,
    RGB = 1,
    BGR = 2
};

#include <opencv2/core/mat.hpp>

void convert_to_grayscale(cv::Mat& img, const color_order_t in_color_order);

void convert_to_true_depth(cv::Mat& img, const double depthmap_factor);

void equalize_histogram(cv::Mat& img);

#endif // STELLA_VSLAM_UTIL_IMAGE_CONVERTER_H
