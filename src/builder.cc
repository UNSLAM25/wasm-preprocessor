#include "builder.h"

ImagePreprocessor * build_client(const int max_num_keypoints, const bool debug) {
    ImagePreprocessor * instance = new ImagePreprocessor(max_num_keypoints, debug);
    return instance;
}

ImagePreprocessor * build_server(const int max_num_keypoints, 
    const std::string SERVER_IP,
    const int slamMode,
    const bool debug = false) {
    ImagePreprocessor * instance = new ImagePreprocessor(max_num_keypoints, SERVER_IP, slamMode, debug);
    instance->set_server_message_callback();
    return instance;
}

ImagePreprocessor * build_server_with_params(int ptr_to_ptr_mask_arr, 
        const int max_num_keypoints,
        const std::string name, 
        const float scale_factor, 
        const unsigned int num_levels,
        const unsigned int ini_fast_thr, 
        const unsigned int min_fast_thr,
        const std::string SERVER_IP,
        const int slamMode,
        const bool debug) {
    ImagePreprocessor * instance = new ImagePreprocessor(ptr_to_ptr_mask_arr, 
        max_num_keypoints, 
        name, 
        scale_factor, 
        num_levels, 
        ini_fast_thr, 
        min_fast_thr, 
        SERVER_IP, 
        slamMode, 
        debug);

    instance->set_server_message_callback();

    return instance;
}
