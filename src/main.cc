#include <emscripten/bind.h>
#include "image-preprocessor.h"

using namespace emscripten;

// Factory
ImagePreprocessor * build_client(const int max_num_keypoints, const bool debug) {
    ImagePreprocessor * instance = new ImagePreprocessor(max_num_keypoints, debug);
    return instance;
}

EMSCRIPTEN_BINDINGS(my_module) {
    class_<ImagePreprocessor>("ImagePreprocessor")
    // ImagePreprocessor constructors
    // .constructor<int, const int, const std::string, const float, const unsigned int, const unsigned int, const unsigned int, const std::string, const int, const bool>()
    // .constructor<const int, const std::string, const int, const bool>()
    // .constructor<const int, const bool>()
    
    // Factory methods for constructing image preprocessor
    .constructor(&build_client, allow_raw_pointers())

    .function("preprocess_image", &ImagePreprocessor::preprocess_image)
    .function("get_output_image", &ImagePreprocessor::get_output_image)
    .function("load_image", &ImagePreprocessor::load_image);
    
    // Binding for "ArrayPointer" structure defined in image-preprocessor.h
    value_object<ArrayPointer>("ArrayPointer")
        .field("arrPointer", &ArrayPointer::arrPointer)
        .field("size", &ArrayPointer::size)
        ;

    // Bindings for std::vector<std::vector<float>>
    register_vector<std::vector<float>>("vector<vector<float>>");
    register_vector<float>("vector<float>");

    // Bindings for enums
    enum_<Command>("Command")
        .value("COMMAND_GRAVITY_VECTOR", COMMAND_GRAVITY_VECTOR);
}