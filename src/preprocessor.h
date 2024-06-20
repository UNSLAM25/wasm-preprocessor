#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "feature/orb_extractor.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/core/types.hpp"
#include "opencv2/features2d.hpp"
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#include <emscripten/websocket.h>

using namespace emscripten;
using namespace cv;
using namespace std;

class Preprocessor {
public:
    /**
     * @brief Construct a new Preprocessor object
     * This consctructor also initialize the ORB extractor
     * 
     * @param max_num_keypoints Limits the number of keypoints, extra keypoints detected will be eliminated by quadtree
     */
    Preprocessor(const int min_size);

    Preprocessor():Preprocessor(800){};

    /**
     * @brief Bound method to process a supplied image
     * 
     * Input image:
     * In javascript the image buffer is allocated on heap with malloc, and should be free() after this method.
     * The image is supplied as an array with width and height metadata.
     * The raw array is type uint8_t, the image is RGBA.
     * All three arguments are passed to toMat(), see its reference.
     * Serialized features are in the form of all of 32 bytes (256 bits) descriptors followed by all of 16 bytes (3 float and 1 int) keypoints.
     * Number of features is array byte size over 48 bytes per feature.
     * 
     * @param ptr int to be cast to pointer, with the address of image buffer
     * @param width of the image
     * @param height of the image
     * @param debug 0 (default) means no debug, 1 adds a debug row to the returned matrix.
     * @return val Serialized features, aka "image descriptor".  See toArray for object description.
     * 
     * emscripten isn't binding the optional parameter "debug", this method only works if all parameters are explicit given.
     */
    val preprocess(int ptr, int width, int height, int debug = 0);

    /**
     * @brief Gets the annotated image
     * A monochromatic version of the last preprocessed image is kept in the object, along with the keypoints detected.
     * This method annotate those keypoints on that image, and returns it.
     * 
     * @return val Annotated RGBA image.  See toArray for object description.
     */
    val getAnnotations();

    /**
     * @brief Initializes the websocket connection
     * Constructs a websocket instance in the given IP and Port. 
     * Also setups a callback to process received messages from the server.     
     *      
     * Should be called after Preprocessor class instantiation.
     */
    void initWebsocket(const std::string SERVER_IP);

    static EM_BOOL onServerMessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData);
private:
    /**
     * @brief Grabs a memory allocated uint8 array image, and return a Mat CV_8UC4
     * Javascript images are RGBA, toMat delivers only CV_8UC4 Mat.
     * Javascript pass the pointer to heap as an int.
     * to8UC4Mat assumes it is CV_8UC4.  No size check is done.
     * Failing to deliver the expected format could cause a memory error.
     * 
     * @param ptr int to be cast to pointer, with the address of image buffer
     * @param width of the image 
     * @param height of the image 
     * @return Mat 
     */
    Mat to8UC4Mat(int ptr, int width, int height);

    /**
     * @brief Converts a Mat to a val object ready to return to javascript
     * The val object includes these properties:
     * - width
     * - height
     * - type: CvMat type, including number of channels
     * - elementSize, in bytes (covers all channels).  For CV_8UC4 this value is 4.
     * - array, a memory_view delivered to Javascript as Uint8Array
     * 
     * @param mat , the Mat to be converted
     * @return val , the javacript object wrapping the array
     */
    val toArray(Mat mat);
    
    /**
     * @brief Preserves Mat data after destruction of local Mat containers
     * This Mat stores a copy of toArray() Mat argument, 
     * so at least one pointer to its data survives after returning to javascript.
     * This buffer persists until toArray() is invoked again.
     */
    Mat toArrayMatBuffer;

    // Properties
    /** Extractor, initialized on construction */
    orb_extractor* extractor = nullptr;

    /** 
     * Keypoints got in preprocess, kept for possible annotation
     * Only consumed by getAnnotations()
     */
    vector<KeyPoint> keypoints;

    /** 
     * Monochromatic version of preprocess input image, kept for possible annotation
     * Only consumed by getAnnotations()
     */
    Mat imGray;

    /** 
     * Will house a websokect instance, used to send data to the server
     */
    EMSCRIPTEN_WEBSOCKET_T websocket;

    /**
     * @brief Checks if the websocket connection is still alive
     */
    bool isConnected();

    /**
     * @brief Sends data to the server through the websocket
     */
    void sendData(float * imageData, uint32_t size);
};
#endif // PREPROCESSOR_H