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

enum Command { 
    COMMAND_NONE = -1,
    COMMAND_STOP = 1, 
    COMMAND_STOP_AND_DISCARD = 2, 
    COMMAND_SAVE_POINT_INITIAL = 10,    
    COMMAND_SAVE_POINT_FINAL = 20,
    COMMAND_GRAVITY_VECTOR = 30,    
    COMMAND_PROCESS_FRAME = 100,
    COMMAND_PROCESS_DEBUG_FRAME = 101,
    COMMAND_NEW_TRAJECTORY = 300,
    COMMAND_NEW_TRAJECTORY_POINT = 301,
    COMMAND_FINISH_TRAJECTORY = 302,
    COMMAND_FOLLOW_TRAJECTORY_BETEWEN_POINTS = 303,
    COMMAND_TRAJECTORY_INDICATION = 304,
    COMMAND_TRAJECTORY_FINISHED = 305,
    COMMAND_TRAJECTORY_DISCARD_LAST_POINT = 306,
    COMMAND_FATAL_ERROR = 999
};

enum SlamMode { 
    MODE_MAPPING = 1,
    MODE_LOCALIZE = 2, 
    MODE_TRAJECTORY = 3, 
};

enum OperationMode { 
    SERVER = 1,
    CLIENT = 2,
};

struct ArrayPointer {
    unsigned int arrPointer;
    unsigned int size;
};

class ImagePreprocessor {
public:
    //! Constructor
    ImagePreprocessor(const int max_num_keypoints,
        const bool debug = false);

    //! Destructor
    // ~ImagePreprocessor();

    void load_image(int buffer, int width, int height);
    void preprocess_image();
    emscripten::val get_output_image();

private:
    std::vector<float> serializeKeypoints();
    void serialize_results(const cv::_InputArray& in_descriptors, const cv::_OutputArray& cFrame);
    std::vector<std::vector<float>> read_mask_rectangles(int ptr_to_ptr_mask_arr);
   
    // ORB extractors
    //! ORB extractor for left/monocular image
    orb_extractor* extractor_left_ = nullptr;

    //! Temporary variables for visualization
    std::vector<cv::KeyPoint> keypts_;
    cv::Mat descriptors_;
    cv::Mat mask;
    cv::Mat img;
    int image_width;
    int image_height;

    //! set debug mode
    bool DEBUG = false;

    //! current operation mode
    OperationMode operationMode;

    //! counter for frames
    unsigned int frameCount = 0;

    //! stores latest extracted frame
    cv::Mat currentFrame;
};

#endif // IMAGE_PREPROCESSOR_H