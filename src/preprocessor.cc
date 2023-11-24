#include "preprocessor.h"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <iostream>
#include <cstring>
//#include <cassert>
#define MAT(mat) cout << #mat ": rows " << mat.rows << ", cols " << mat.cols << ", type " << mat.type() << ", bytes " <<  mat.total()*mat.elemSize() << endl;

Preprocessor::Preprocessor(const int max_num_keypoints){    
    orb_params * orb_params_ = new orb_params("default");
    extractor = new orb_extractor(orb_params_, max_num_keypoints, {});
    cout << "extractor " << extractor << endl;
}

cv::Mat Preprocessor::to8UC4Mat(int ptr, int width, int height){
    //assert(array.size() == width * height * 4);
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

val Preprocessor::preprocess(int ptr, int width, int height){
    //cout << "preprocess " << ptr << ", " << width << ", " << height << endl;
    Mat img = to8UC4Mat(ptr, width, height);
    //MAT(img)
    cvtColor(img, imGray, cv::COLOR_RGBA2GRAY); // RGBA is the format used in javascript
    //MAT(imGray)
    Mat descriptors;
    extractor->extract(imGray, Mat(), keypoints, descriptors);
    //cout << "extract " << keypoints.size() << endl;
    MAT(descriptors)

    // serialize descriptors and keypoints on a Mat
    int howManyFeatures = descriptors.rows;  // Same as keypoints.size()
    //cout << "howManyFeatures " << howManyFeatures << endl;

    cv::Mat imageDescriptor(howManyFeatures+1, 38, CV_8UC1);  // +1 for debug data
    //imageDescriptor.pop_back(); // howManyFeatures rows, plus one reserved row.
    //assert(descriptors.cols == 32);
    //descriptors.copyTo(imageDescriptor.colRange(0,32).rowRange(0,howManyFeatures));
    for(int i=0; i<howManyFeatures; i++){
        unsigned char *row = imageDescriptor.ptr(i);
        std::memcpy(row, descriptors.ptr(i), 32);  // dst, src, byteCount
        *((unsigned short*) (row+32)) = (unsigned short) keypoints[i].pt.x;
        *((unsigned short*) (row+34)) = (unsigned short) keypoints[i].pt.y;
        row[36] = (unsigned char)(keypoints[i].angle * 255.0/360.0);
        row[37] = (unsigned char) keypoints[i].octave;
    }
    /* Debug
     * Last row has octave = 255, meaning the whole row isn't a descriptor but a debug info.
     * This info consists in 5 floats with 4 first keypoint properties and the sum of the first descriptor elements.
     */
    //float *debugRow = imageDescriptor.ptr<float>(howManyFeatures);    // funciona, pero ptr no es una función plantillada según la documentación.  Sin embargo no produjo error de compilación.
    float *debugRow = (float*) imageDescriptor.ptr(howManyFeatures);

    auto &kp = keypoints[0];
    debugRow[0] = kp.pt.x;
    debugRow[1] = kp.pt.y;
    debugRow[2] = kp.angle;
    debugRow[3] = kp.octave;

    int sumaDescriptor = 0;
    int sumaDescriptorSerializado = 0;
    for(int i=0; i<32; i++){
        sumaDescriptor += descriptors.at<unsigned char>(0,i);
        sumaDescriptorSerializado += imageDescriptor.at<unsigned char>(0,i);
    }
    debugRow[4] = sumaDescriptor;  // conversión implícita a float, no hay problema con el rango de valores de 0 a 8191.
    if(sumaDescriptor != sumaDescriptorSerializado){
        cout << "ERROR: las sumas difieren: " << sumaDescriptor << ", " << sumaDescriptorSerializado << endl;
    }

    ((unsigned char*) debugRow)[37] = 255; // Marcador de debug


    MAT(imageDescriptor)
    return toArray(imageDescriptor);


    /*
    //Mat serializedFeatures(howManyFeatures*2, 4, CV_32FC1, descriptors.data);
    //serializedFeatures.resize(howManyFeatures*3);  // Possible copy of descriptors
    Mat serializedFeatures(howManyFeatures*3, 4, CV_32SC1);
    Mat serializedDescriptors(howManyFeatures*2, 4, CV_32SC1, descriptors.data);
    serializedDescriptors.copyTo(serializedFeatures.rowRange(0,howManyFeatures*2));
    size_t row = howManyFeatures*2;//, end=serializedFeatures.rows;
    cout << "howManyFeatures " << howManyFeatures << endl;
    cout << "Row & Keypoint x,y, angle:" << row << ", " << keypoints[0].pt.x << ", " << keypoints[0].pt.y << ", " << keypoints[0].angle << endl;
    for(const auto& keypoint : keypoints){
        serializedFeatures.at<int>(row, 0) = keypoint.pt.x;
        serializedFeatures.at<int>(row, 1) = keypoint.pt.y;
        serializedFeatures.at<int>(row, 2) = keypoint.angle;
        serializedFeatures.at<int>(row, 3) = keypoint.octave;
        //serializedFeatures.at<float>(row, 3) = *(float*)&keypoint.octave;
        row++;
    }
    cout << "serializedFeatures x:" << serializedFeatures.at<int>(howManyFeatures*2, 0) << endl;
    */

    /*
    serializedFeatures.reserve(howManyFeatures*3);  // Possible copy of descriptors
    // Add keypoints
    for(const auto& keypoint : keypoints){
        serializedFeatures.push_back((Mat_<float>(1,4)<<
            keypoint.pt.x,
            keypoint.pt.y,
            keypoint.angle,
            *(float*)&keypoint.octave
        ));
    }*/
    /*
    vector<float> feature(4);
    cout << " feature.size " << feature.size() << endl;
    for(const auto& keypoint : keypoints){
        cout << "<";
        feature[0] = keypoint.pt.x;
        feature[1] = keypoint.pt.y;
        feature[2] = keypoint.angle;
        feature[3] = *(float*)&keypoint.octave;
        cout << "-";
        serializedFeatures.push_back(Mat(feature));
        cout << ">";
    }
    */
    //cout << endl;
    //cout << "After adding keypoints ";
    /*MAT(serializedFeatures)
    return toArray(serializedFeatures);*/
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