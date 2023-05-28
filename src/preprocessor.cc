#include "preprocessor.h"
#include "opencv2/imgproc.hpp"
#include <vector>
//#include <cassert>

Preprocessor::Preprocessor(const int max_num_keypoints){    
    orb_params * orb_params_ = new orb_params("default");        
    vector<vector<float>> mask_rectangles = {};
    extractor = new orb_extractor(orb_params_, max_num_keypoints, mask_rectangles);
}

cv::Mat Preprocessor::to8UC4Mat(int ptr, int width, int height){
    //assert(array.size() == width * height * 4);
    return Mat(height, width, CV_8UC4, reinterpret_cast<void*>(ptr));
}

val Preprocessor::toArray(cv::Mat mat){
    val result = val::object();
    result.set("width", mat.cols);
    result.set("height", mat.rows);
    result.set("type", mat.type()); // CvMat type
    result.set("elementSize", mat.elemSize());  // Size in bytes
    result.set("array", typed_memory_view(mat.total()*mat.elemSize(), mat.data));   // elemSize is 4 for RGBA
    return result;
}

val Preprocessor::preprocess(int ptr, int width, int height){
    Mat img = to8UC4Mat(ptr, width, height);
    cvtColor(img, imGray, cv::COLOR_RGBA2GRAY); // RGBA is the format used in javascript
    Mat descriptors;
    extractor->extract(img, Mat(), keypoints, descriptors);

    // serialize descriptors and keypoints on a Mat
    auto howManyFeatures = descriptors.rows;  // Same as keypoints.size()
    Mat serializedFeatures(howManyFeatures*2, 4, CV_32FC1, descriptors.data);
    serializedFeatures.reserve(howManyFeatures*3);  // Possible copy of descriptors
    for(const auto& keypoint : keypoints){
        serializedFeatures.push_back(vector<float>{
            keypoint.pt.x, 
            keypoint.pt.y, 
            keypoint.angle, 
            *(float*)&keypoint.octave
        });
    }
    return toArray(serializedFeatures);
}

val Preprocessor::getAnnotations(){
    if (imGray.empty())
        return val(0);

    Mat annotatedImage;
    // drawkeypoints works on RGBA too
    drawKeypoints(annotatedImage, keypoints, annotatedImage, Scalar(0,255,0,0));
    return toArray(annotatedImage);
}