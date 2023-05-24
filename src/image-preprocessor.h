#ifndef IMAGE_PREPROCESSOR_H
#define IMAGE_PREPROCESSOR_H

#include "feature/orb_extractor.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/core/types.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#include <iostream>

class ImagePreprocessor {
public:
    //! Constructor
    ImagePreprocessor(const int max_num_keypoints);

    void load_image(int buffer, int width, int height);
    void preprocess_image();
    emscripten::val get_output_image();

private:
    std::vector<float> serializeKeypoints();
    cv::Mat serialize_results(const cv::_InputArray& in_descriptors);
   
    // ORB extractors
    //! ORB extractor for left/monocular image
    orb_extractor* extractor_left_ = nullptr;

    //! Temporary variables for visualization
    std::vector<cv::KeyPoint> keypts_;
    cv::Mat descriptors_;
    cv::Mat img;

    //! counter for frames
    unsigned int frameCount = 0;
};

#endif // IMAGE_PREPROCESSOR_H