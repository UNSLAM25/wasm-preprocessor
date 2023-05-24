#include "image-preprocessor.h"
#include "util/image_converter.h"

/* 
    Instantiates an instance of ImagePreprocessor with operation mode set as SERVER.

    Also allow several paramters such as a mask array, scale factors, etc.
*/

ImagePreprocessor::ImagePreprocessor(const int max_num_keypoints){    
    orb_params * orb_params_ = new orb_params("default");        
    std::vector<std::vector<float>> mask_rectangles = {};
    extractor_left_ = new orb_extractor(orb_params_, max_num_keypoints, mask_rectangles);
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
    extractor_left_->extract(img_gray, cv::Mat(), keypts_, descriptors_);

    if (keypts_.empty()) {
        std::cout << "preprocess: cannot extract any keypoints" << "\n";
        return;
    }

    auto results = serialize_results(descriptors_);
    // At this point results are not sent to anywhere.
}

emscripten::val ImagePreprocessor::get_output_image() {
    if (img.empty())
        return emscripten::val(0);

    cv::Mat outImage = img.clone();
    cv::drawKeypoints(img, keypts_, outImage, cv::Scalar(0,255,0));

    // Data is already contigous because we cloned earlier
    return emscripten::val(emscripten::typed_memory_view(outImage.total()*outImage.channels(), outImage.data));
}

cv::Mat ImagePreprocessor::serialize_results(const cv::_InputArray& in_descriptors) {
    std::vector<float> keypoints = serializeKeypoints();

    unsigned int howManyFeatures = in_descriptors.rows();

    cv::Mat descr = in_descriptors.getMat();
    descr.convertTo(descr, CV_32F);
    cv::Mat matKeypoints = cv::Mat(keypoints).reshape(1, howManyFeatures);
    cv::Mat results;
    cv::hconcat(descr, matKeypoints, results);
    frameCount += 1;
    return results;
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