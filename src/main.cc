#include <emscripten/bind.h>
#include "preprocessor.h"

using namespace emscripten;

Preprocessor* preprocessorFactory(const int max_num_keypoints) {
    Preprocessor * pre = new Preprocessor(max_num_keypoints);
    return pre;
}

Preprocessor* preprocessorFactoryWebsocket(const int max_num_keypoints, std::string ip_port ) {
    Preprocessor * pre = new Preprocessor(max_num_keypoints);
    pre->initWebsocket(ip_port);   
    return pre;
}

EMSCRIPTEN_BINDINGS(my_module) {
    class_<Preprocessor>("Preprocessor")    
        .constructor(&preprocessorFactory, allow_raw_pointers())
        .constructor(&preprocessorFactoryWebsocket, allow_raw_pointers())
        .function("preprocess", &Preprocessor::preprocess)
        .function("getAnnotations", &Preprocessor::getAnnotations);
}