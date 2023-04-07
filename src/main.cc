#include <emscripten/bind.h>
#include "image-preprocessor.h"
#include "builder.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(my_module) {
    class_<ImagePreprocessor>("ImagePreprocessor")
    // ImagePreprocessor constructors
    // .constructor<int, const int, const std::string, const float, const unsigned int, const unsigned int, const unsigned int, const std::string, const int, const bool>()
    // .constructor<const int, const std::string, const int, const bool>()
    // .constructor<const int, const bool>()
    
    // Factory methods for constructing image preprocessor
    .constructor(&build_client, allow_raw_pointers())
    .constructor(&build_server, allow_raw_pointers())
    .constructor(&build_server_with_params, allow_raw_pointers())

    .function("preprocess_image", &ImagePreprocessor::preprocess_image)
    .function("get_output_image", &ImagePreprocessor::get_output_image)
    .function("load_image", &ImagePreprocessor::load_image)
    .function("set_server_message_callback", &ImagePreprocessor::set_server_message_callback)
    .function("saveInitialCameraPose", &ImagePreprocessor::saveInitialCameraPose)
    .function("saveFinalCameraPose", &ImagePreprocessor::saveFinalCameraPose)
    .function("sendGravityVector", &ImagePreprocessor::sendGravityVector)
    .function("stopSLAM", &ImagePreprocessor::stopSLAM)
    .function("stopAndDiscard", &ImagePreprocessor::stopAndDiscard)
    .function("startNewTrajectory", &ImagePreprocessor::startNewTrajectory)
    .function("addNewPointToTrajectory", &ImagePreprocessor::addNewPointToTrajectory)
    .function("discardTrajectoryLastPoint", &ImagePreprocessor::discardTrajectoryLastPoint)
    .function("finishTrajectory", &ImagePreprocessor::finishTrajectory)
    .function("followTrajectory", &ImagePreprocessor::followTrajectory)
    .function("sendMapName", &ImagePreprocessor::sendMapName);
    
    // Binding for "ArrayPointer" structure defined in image-preprocessor.h
    value_object<ArrayPointer>("ArrayPointer")
        .field("arrPointer", &ArrayPointer::arrPointer)
        .field("size", &ArrayPointer::size)
        ;

    // Binding for std:vector<Keypoint>
    // register_vector<Keypoint>("vector<Keypoint>");

    // Bindings for std::vector<std::vector<float>>
    register_vector<std::vector<float>>("vector<vector<float>>");
    register_vector<float>("vector<float>");

    // Bindings for enums
    enum_<Command>("Command")
        .value("COMMAND_GRAVITY_VECTOR", COMMAND_GRAVITY_VECTOR);
}