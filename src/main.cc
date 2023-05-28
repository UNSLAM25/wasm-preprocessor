#include <emscripten/bind.h>
#include "preprocessor.h"

using namespace emscripten;

// Factory
Preprocessor* preprocessorFactory(const int max_num_keypoints) {
    return new Preprocessor(max_num_keypoints);
}

EMSCRIPTEN_BINDINGS(my_module) {
    class_<Preprocessor>("Preprocessor")    
        .constructor(&preprocessorFactory, allow_raw_pointers())
        .function("preprocess", &Preprocessor::preprocess)
        .function("getAnnotations", &Preprocessor::getAnnotations);
}