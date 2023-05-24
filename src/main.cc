#include <emscripten/bind.h>
#include "image-preprocessor.h"

using namespace emscripten;

// Factory
ImagePreprocessor* imagePreprocessorFactory(const int max_num_keypoints, const bool debug) {
    return new ImagePreprocessor(max_num_keypoints);
}

EMSCRIPTEN_BINDINGS(my_module) {
    class_<ImagePreprocessor>("ImagePreprocessor")
    
    // Factory methods for constructing image preprocessor
    .constructor(&imagePreprocessorFactory, allow_raw_pointers())

    .function("preprocess_image", &ImagePreprocessor::preprocess_image)
    .function("get_output_image", &ImagePreprocessor::get_output_image)
    .function("load_image", &ImagePreprocessor::load_image);
    
    // Bindings for std::vector<std::vector<float>>
    register_vector<std::vector<float>>("vector<vector<float>>");
    register_vector<float>("vector<float>");
}