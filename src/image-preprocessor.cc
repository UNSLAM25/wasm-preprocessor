#include "image-preprocessor.h"
#include "util/image_converter.h"

/* 
    Instantiates an instance of ImagePreprocessor with operation mode set as SERVER.

    Also allow several paramters such as a mask array, scale factors, etc.
*/

ImagePreprocessor::ImagePreprocessor(const int max_num_keypoints, const bool debug) : DEBUG(debug) {    
    orb_params * orb_params_ = new orb_params("default");        
    std::vector<std::vector<float>> mask_rectangles = {};
    extractor_left_ = new orb_extractor(orb_params_, max_num_keypoints, mask_rectangles);
    descriptors_ = cv::Mat();
    // empty mask
    mask = cv::Mat();
    operationMode = CLIENT;
    std::cout << "Created instance of ImagePreprocessor" << "\n";
}

// TODO: width and height are going to be the same (unless user changes to portrait mode)
// but the user shouldnt change screen orientation eitherway...
void ImagePreprocessor::load_image(int buffer, int width, int height) 
{
    // Read the image
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer);    
    // Release previous image
    img.release();
    image_width = width;
    image_height = height;
    // Fill new image
    img = cv::Mat(height, width, CV_8UC4, ptr);
}

/*
*   Process method
*/

void ImagePreprocessor::preprocess_image() {    
    // color conversion    
    cv::Mat img_gray = img.clone();
       
    // All the images we receive from Javascript come in RGBA or RGB format (depending on if we are using alpha on canvas or not)
    // so the color order is color_order_t::RGB
    convert_to_grayscale(img_gray, color_order_t::RGB);

    // Clear descriptors cv::Mat on every loop
    descriptors_.release();

    keypts_.clear();
    // auto start = std::chrono::steady_clock::now();
    extractor_left_->extract(img_gray, mask, keypts_, descriptors_);
    // auto finish = std::chrono::steady_clock::now();
    // double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count();
    // std::cout << "Total EXTRACT time: " << elapsed_seconds << "\n";

    if (keypts_.empty()) {
        std::cout << "preprocess: cannot extract any keypoints" << "\n";
        return;
    }

    serialize_results(descriptors_, currentFrame);

    // Once we've got it, send first frame
    // If we don't do this, the server will get stuck waiting for data
}

emscripten::val ImagePreprocessor::get_output_image() {
    if (img.empty())
        return emscripten::val(0);

    cv::Mat outImage = img.clone();
    
    // TODO: draw keypoints configurable option?
    cv::drawKeypoints(img, keypts_, outImage, cv::Scalar(0,255,0));

    // Data is already contigous because we cloned earlier
    // return emscripten::val(emscripten::typed_memory_view(outImage.total()*outImage.channels(), outImage.clone().data));
    return emscripten::val(emscripten::typed_memory_view(outImage.total()*outImage.channels(), outImage.data));
}

void ImagePreprocessor::serialize_results(const cv::_InputArray& in_descriptors, const cv::_OutputArray& cFrame) {
    std::vector<float> keypoints = serializeKeypoints();

    unsigned int howManyFeatures = in_descriptors.rows();

    cv::Mat descr = in_descriptors.getMat();
    descr.convertTo(descr, CV_32F);
    cv::Mat matKeypoints = cv::Mat(keypoints).reshape(1, howManyFeatures);
    cv::Mat temp;
    cv::hconcat(descr, matKeypoints, temp);

    if (DEBUG)
    {
        cv::Mat extraInfo(temp.rows, 2, 0);
        extraInfo.convertTo(extraInfo, CV_32F);
        extraInfo.at<float>(0,0) = COMMAND_PROCESS_DEBUG_FRAME;
        extraInfo.at<float>(0,1) = frameCount;   
        cv::hconcat(extraInfo, temp, cFrame);
    }
    else 
    {
        cv::Mat extraInfo(temp.rows, 1, 0);
        extraInfo.convertTo(extraInfo, CV_32F);
        extraInfo.at<float>(0,0) = COMMAND_PROCESS_FRAME;        
        cv::hconcat(extraInfo, temp, cFrame); 
    }

    frameCount += 1;
}

std::vector<float> ImagePreprocessor::serializeKeypoints() {
    std::vector<float> keypoints = {};

    for (const auto& v : keypts_) {
        keypoints.push_back(v.pt.x);
        keypoints.push_back(v.pt.y);
        keypoints.push_back(v.angle);
        keypoints.push_back((float) v.octave);
    }   
    
    return keypoints;
}

/*
*   Read mask rectangles from WebAssembly's heap.
*   int ptr_to_ptr_mask_arr is a pointer to an array of pointers
*/
std::vector<std::vector<float>> ImagePreprocessor::read_mask_rectangles(int ptr_to_ptr_mask_arr) {
    const int arrSize = 4;
    std::vector<std::vector<float>> mask_rectangles = {};    

    uint32_t * arrPointer = reinterpret_cast<uint32_t *>(ptr_to_ptr_mask_arr);  

    for (int i = 0; i < arrSize; i++)
    {
        std::vector<float> temp = {};
        float* pointer = reinterpret_cast<float*>(arrPointer[i]);    

        for (int i = 0; i < arrSize; i++)
        {
            temp.push_back(pointer[i]);
        } 

        mask_rectangles.push_back(temp);
    } 

    return mask_rectangles;   
}