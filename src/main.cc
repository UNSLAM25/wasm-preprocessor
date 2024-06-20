#include <emscripten/bind.h>
#include "preprocessor.h"

using namespace emscripten;

// Factory
Preprocessor* preprocessorFactory(const int max_num_keypoints, const std::string ip_port) {
    Preprocessor * pre = new Preprocessor(max_num_keypoints);
    pre->initWebsocket(ip_port);
    return pre;
}

EMSCRIPTEN_BINDINGS(my_module) {
    class_<Preprocessor>("Preprocessor")    
        .constructor(&preprocessorFactory, allow_raw_pointers())
        .function("preprocess", &Preprocessor::preprocess)
        .function("getAnnotations", &Preprocessor::getAnnotations);
}