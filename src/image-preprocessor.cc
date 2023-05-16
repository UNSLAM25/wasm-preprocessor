#include "image-preprocessor.h"
#include "util/image_converter.h"
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#include <emscripten/val.h>

/* 
    Instantiates an instance of ImagePreprocessor with operation mode set as SERVER.

    Also allow several paramters such as a mask array, scale factors, etc.
*/
ImagePreprocessor::ImagePreprocessor(
    int ptr_to_ptr_mask_arr, 
    const int max_num_keypoints,
    const std::string name, 
    const float scale_factor, 
    const unsigned int num_levels,
    const unsigned int ini_fast_thr, 
    const unsigned int min_fast_thr,
    // own parameters
    const std::string SERVER_IP,
    const int slamMode,
    const bool debug) {
    
    orb_params * orb_params_ = new orb_params(name, scale_factor, num_levels, ini_fast_thr, min_fast_thr);
        
    //auto mask_rectangles = preprocessing_params["mask_rectangles"].as<std::vector<std::vector<float>>>(std::vector<std::vector<float>>());
    std::vector<std::vector<float>> mask_rectangles = read_mask_rectangles(ptr_to_ptr_mask_arr);

    for (const auto& v : mask_rectangles) {
        if (v.size() != 4) {
            throw std::runtime_error("mask rectangle must contain four parameters");
        }
        if (v.at(0) >= v.at(1)) {
            throw std::runtime_error("x_max must be greater than x_min");
        }
        if (v.at(2) >= v.at(3)) {
            throw std::runtime_error("y_max must be greater than x_min");
        }
    }

    // const auto max_num_keypoints = preprocessing_params["max_num_keypoints"].as<unsigned int>(2000);
    extractor_left_ = new orb_extractor(orb_params_, max_num_keypoints, mask_rectangles);
    descriptors_ = cv::Mat();
    // empty mask
    mask = cv::Mat();
    operationMode = SERVER;
    DEBUG = debug;

    // Create websocket instance
    if (!emscripten_websocket_is_supported()) {
        std::cout << "Emscripten websocket is not supported" << "\n";
        return;
    }
    
    ws = create_websocket_instance(SERVER_IP, slamMode);

    std::cout << "Created instance of ImagePreprocessor" << "\n";
}

/* 
    Instantiates an instance of ImagePreprocessor with operation mode set as SERVER
*/

ImagePreprocessor::ImagePreprocessor(const int max_num_keypoints, 
    // own params
    const std::string SERVER_IP,
    const int slamMode, 
    const bool debug) 
    : DEBUG(debug) {    
    orb_params * orb_params_ = new orb_params("default");        
    std::vector<std::vector<float>> mask_rectangles = {};
    extractor_left_ = new orb_extractor(orb_params_, max_num_keypoints, mask_rectangles);
    descriptors_ = cv::Mat();
    // empty mask
    mask = cv::Mat();
    operationMode = SERVER;

    // Create websocket instance
    if (!emscripten_websocket_is_supported()) {
        std::cout << "Emscripten websocket is not supported" << "\n";
        return;
    }

    ws = create_websocket_instance(SERVER_IP, slamMode);
    

    std::cout << "Created instance of ImagePreprocessor" << "\n";
}

/*
    Instantiates an instance of ImagePreprocessor with operation mode set as CLIENT.
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


/*
*   Websocket related methods
*/

EMSCRIPTEN_WEBSOCKET_T ImagePreprocessor::create_websocket_instance(const std::string SERVER_IP, const int mode) {
    std::string websocketURL = "ws://" + SERVER_IP;
    switch (mode)
    {
    case MODE_MAPPING:
        websocketURL += "/api/ws/map";
        break;
    case MODE_LOCALIZE:
        websocketURL += "/api/ws/localize";
        break;

    case MODE_TRAJECTORY:
        websocketURL += "/api/ws/trajectory";
        break;
    
    default:
        websocketURL += "/api/ws/map";
        break;
    }

    const char* wsURL = websocketURL.c_str();
    EmscriptenWebSocketCreateAttributes ws_attrs = {
        wsURL,
        NULL,
        EM_TRUE
    };
    EMSCRIPTEN_WEBSOCKET_T ws = emscripten_websocket_new(&ws_attrs); 
    return ws;
}

EM_BOOL ImagePreprocessor::process_server_message(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) {
    ImagePreprocessor * instance = (ImagePreprocessor *) userData;
    
    auto data = reinterpret_cast<float*>(websocketEvent->data);
    // data[0] = represents a command
    
    if (data[0] == COMMAND_TRAJECTORY_INDICATION)
    {
        instance->draw_trajectory_marker = true;
        instance->direction = data[1];
        instance->marker_x = data[2];
        instance->marker_distance = data[3];
    }

    if (data[0] == COMMAND_TRAJECTORY_FINISHED)
    {
        instance->draw_trajectory_marker = false;
        instance->draw_trajectory_finish = true;
    }

    if (data[0] == COMMAND_FATAL_ERROR)
    {        
        instance->currentFrame.release();
        EM_ASM(
            stopCapture();
            showErrorModal();
        );
    }
    
    instance->send_data(reinterpret_cast<float*>(instance->currentFrame.data), instance->currentFrame.total()*instance->currentFrame.channels());

    return EM_TRUE;
}

bool ImagePreprocessor::check_if_connection_is_alive()
{
    unsigned short readyState = 0;
    emscripten_websocket_get_ready_state(ws, &readyState);
    if (readyState == 3 || readyState == 2)
    { 
        EM_ASM(throw new Error("Cant reach server"););
        return false;
    }
    return true;
}

void ImagePreprocessor::send_data(float * results, uint32_t size) {
    if (!check_if_connection_is_alive())
        return;
    void* data = reinterpret_cast<float*>(results); 
    // avoid sending empty data
    if (size)
    {
        emscripten_websocket_send_binary(ws, data, size * sizeof(float));
    }
}

void ImagePreprocessor::saveInitialCameraPose() {
    float cmd = COMMAND_SAVE_POINT_INITIAL;
    send_data(reinterpret_cast<float*>(&cmd), 1);
}

void ImagePreprocessor::saveFinalCameraPose() {
    float cmd = COMMAND_SAVE_POINT_FINAL;   
    send_data(reinterpret_cast<float*>(&cmd), 1);
}

void ImagePreprocessor::stopSLAM() {
    float cmd = COMMAND_STOP;
    send_data(reinterpret_cast<float*>(&cmd), 1);
}

void ImagePreprocessor::stopAndDiscard() {
    float cmd = COMMAND_STOP_AND_DISCARD;
    send_data(reinterpret_cast<float*>(&cmd), 1);
}

void ImagePreprocessor::sendGravityVector(int pointer) {
    send_data(reinterpret_cast<float*>(pointer), 4);
}

void ImagePreprocessor::startNewTrajectory() {
    float cmd = COMMAND_NEW_TRAJECTORY;
    send_data(reinterpret_cast<float*>(&cmd), 1);
}

void ImagePreprocessor::addNewPointToTrajectory() {
    float cmd = COMMAND_NEW_TRAJECTORY_POINT;
    send_data(reinterpret_cast<float*>(&cmd), 1);
}

void ImagePreprocessor::discardTrajectoryLastPoint() {
    float cmd = COMMAND_TRAJECTORY_DISCARD_LAST_POINT;
    send_data(reinterpret_cast<float*>(&cmd), 1);
}

void ImagePreprocessor::finishTrajectory() {
    float cmd = COMMAND_FINISH_TRAJECTORY;
    send_data(reinterpret_cast<float*>(&cmd), 1);
}

void ImagePreprocessor::followTrajectory(float tId, float ini, float fin) {    
    float arr[4] = {COMMAND_FOLLOW_TRAJECTORY_BETEWEN_POINTS, tId, ini, fin};
    send_data(reinterpret_cast<float*>(&arr), 4);
}

void ImagePreprocessor::sendMapName(std::string mapName) { 
    char *cstr = new char[mapName.length() + 1];
    strcpy(cstr, mapName.c_str());    
    void* data = reinterpret_cast<char*>(cstr); 
    emscripten_websocket_send_binary(ws, data, mapName.length() * sizeof(char));    
    std::cout << "Sent map name: " << mapName << "\n";    
    delete [] cstr;
}

void ImagePreprocessor::set_server_message_callback() {
    emscripten_websocket_set_onmessage_callback(ws, this, ImagePreprocessor::process_server_message);
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
    if (operationMode == SERVER && sendFirstFrame)
    {
        send_data(reinterpret_cast<float*>(currentFrame.data), currentFrame.total()*currentFrame.channels());
        sendFirstFrame = false;
    }
}

emscripten::val ImagePreprocessor::get_output_image() {
    if (img.empty())
        return emscripten::val(0);

    cv::Mat outImage = img.clone();
    
    // TODO: draw keypoints configurable option?
    cv::drawKeypoints(img, keypts_, outImage, cv::Scalar(0,255,0));

    if (draw_trajectory_marker)
        draw_marker(outImage);    

    if (draw_trajectory_finish)    
        draw_finished_message(outImage);

    // Data is already contigous because we cloned earlier
    // return emscripten::val(emscripten::typed_memory_view(outImage.total()*outImage.channels(), outImage.clone().data));
    return emscripten::val(emscripten::typed_memory_view(outImage.total()*outImage.channels(), outImage.data));
}

// Draws text over the image indicating we arrived at the destination
void ImagePreprocessor::draw_finished_message(cv::Mat& outImage) {
    cv::putText(outImage, "Llegaste al destino", cv::Point(image_width/2-300, image_height/2), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255,255,255), 4, cv::LINE_AA);
}

void ImagePreprocessor::draw_marker(cv::Mat& outImage) {
    std::string direction = get_direction_text();
    cv::putText(outImage, direction, cv::Point(0,40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,0,0), 4, cv::LINE_AA);
    cv::putText(outImage, direction, cv::Point(0,40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255,255,255), 2, cv::LINE_AA);

    if (marker_x > 0 && marker_x < image_width)
    {
        cv::Rect rect(marker_x, image_height/2, 50, get_marker_size());
        cv::rectangle(outImage, rect, cv::Scalar(0, 255, 0), 3);            
        // This gives a black border to the text, helps with visualization
        cv::putText(outImage, "Objetivo", cv::Point(marker_x-100, image_height/2 - 30), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0,0,0), 4, cv::LINE_AA);
        cv::putText(outImage, "Objetivo", cv::Point(marker_x-100, image_height/2 - 30), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255,255,255), 2, cv::LINE_AA);
    }    
}

float ImagePreprocessor::get_marker_size() {
    float size = 150 * (1/marker_distance);

    return size > 10 ? size : 10;
}

std::string ImagePreprocessor::get_direction_text() {
    // FORWARD = 0
    // RIGHT = 1
    // LEFT = 2
    switch((int) direction)
    {
        case 0:
            return std::string("Direccion: Adelante");
        case 1: 
            return std::string("Direccion: Derecha");
        case 2: 
            return std::string("Direccion: Izquierda");
        default:
            return std::string("");
    }
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
