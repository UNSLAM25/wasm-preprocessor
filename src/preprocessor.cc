#include "preprocessor.h"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <iostream>
#include <cstring>

#define MAT(mat) cout << #mat ": rows " << mat.rows << ", cols " << mat.cols << ", type " << mat.type() << ", bytes " <<  mat.total()*mat.elemSize() << endl;

Preprocessor::Preprocessor(const int min_size){    
    orb_params * orb_params_ = new orb_params("default");
    extractor = new orb_extractor(orb_params_, min_size, {});
}

cv::Mat Preprocessor::to8UC4Mat(int ptr, int width, int height){
    return Mat(height, width, CV_8UC4, reinterpret_cast<void*>(ptr));
}

val Preprocessor::toArray(cv::Mat mat){
    //cout << "toArray" << endl;
    //MAT(mat)
    toArrayMatBuffer = mat;
    val result = val::object();
    result.set("width", mat.cols);
    result.set("height", mat.rows);
    result.set("type", mat.type()); // CvMat type
    result.set("elementSize", mat.elemSize());  // Size in bytes
    result.set("array", typed_memory_view(mat.total()*mat.elemSize(), mat.data));   // elemSize is 4 for RGBA
    //cout << "toArray ending " << endl;
    return result;
}

val Preprocessor::preprocess(int ptr, int width, int height, int debug){
    //cout << "preprocess " << ptr << ", " << width << ", " << height << endl;
    Mat img = to8UC4Mat(ptr, width, height);
    //MAT(img)
    cvtColor(img, imGray, cv::COLOR_RGBA2GRAY); // RGBA is the format used in javascript
    //MAT(imGray)
    Mat descriptors;
    extractor->extract(imGray, Mat(), keypoints, descriptors);
    //cout << "extract " << keypoints.size() << endl;
    //MAT(descriptors)

    // serialize descriptors and keypoints on a Mat
    int howManyFeatures = descriptors.rows;  // Same as keypoints.size()
    //cout << "howManyFeatures " << howManyFeatures << endl;

    cv::Mat imageDescriptor(howManyFeatures + (debug==1), 38, CV_8UC1);  // +1 for debug data
    for(int i=0; i<howManyFeatures; i++){
        unsigned char *row = imageDescriptor.ptr(i);
        std::memcpy(row, descriptors.ptr(i), 32);  // dst, src, byteCount
        *((unsigned short*) (row+32)) = (unsigned short) keypoints[i].pt.x;
        *((unsigned short*) (row+34)) = (unsigned short) keypoints[i].pt.y;
        row[36] = (unsigned char)(keypoints[i].angle * 255.0/360.0);
        row[37] = (unsigned char) keypoints[i].octave;
    }

    if(debug == 1){
        /* Debug
        * Last row has octave = 255, meaning the whole row isn't a descriptor but a debug info.
        * This info consists in 5 floats with 4 first keypoint properties and the sum of the first descriptor elements.
        */
        float *debugRow = (float*) imageDescriptor.ptr(howManyFeatures);
        ((unsigned char*) debugRow)[37] = 255; // Marcador de debug

        auto &kp = keypoints[0];
        debugRow[0] = kp.pt.x;
        debugRow[1] = kp.pt.y;
        debugRow[2] = kp.angle;
        debugRow[3] = kp.octave;

        int descriptorChecksum = 0;
        for(int i=0; i<32; i++){
            descriptorChecksum += descriptors.at<unsigned char>(0,i);
        }
        debugRow[4] = descriptorChecksum;  // int in the range 0 to 8191, implicit conversion to float.
    }

    //MAT(imageDescriptor)
    return toArray(imageDescriptor);
}

val Preprocessor::getAnnotations(){
    //cout << "getAnnotations ";
    MAT(imGray)

    if (imGray.empty())
        return val(0);

    Mat annotatedImage;
    cvtColor(imGray, annotatedImage, cv::COLOR_GRAY2RGBA); // RGBA is the format used in javascript
    MAT(annotatedImage)

    // drawkeypoints works on RGBA too
    drawKeypoints(annotatedImage, keypoints, annotatedImage, Scalar(0,255,0,0));
    //cout << "drawKeypoints ";
    return toArray(annotatedImage);
}