#ifndef BUILDER_H
#define BUILDER_H
#include "image-preprocessor.h"

ImagePreprocessor *build_client(const int max_num_keypoints, const bool debug);

ImagePreprocessor *build_server(const int max_num_keypoints,
                                const std::string SERVER_IP,
                                const int slamMode,
                                const bool debug);

ImagePreprocessor *build_server_with_params(int ptr_to_ptr_mask_arr,
                                            const int max_num_keypoints,
                                            const std::string name,
                                            const float scale_factor,
                                            const unsigned int num_levels,
                                            const unsigned int ini_fast_thr,
                                            const unsigned int min_fast_thr,
                                            const std::string SERVER_IP,
                                            const int slamMode,
                                            const bool debug);

#endif // BUILDER_H