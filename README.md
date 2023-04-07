# emscripten-bindings

The following repository contains the "feature extraction" part of Stella VSLAM, i.e. the process which allows you to obtain keypoints and descriptors from an image.

The C++ code present here can be transpiled/compiled into WASM files, which allows you to execute this code in any browser.

Camera capture isn't a part of this repository.

## What are 'bindings'

Bindings serve as a interface between Javascript and WASM code. Think it like a sort of API.

With these bindings, you can make calls to WASM code through Javascript. For example, you can instantiate classes and call instance methods.

## Compiling instructions

- Install [emsdk](https://emscripten.org/docs/getting_started/downloads.html).
- Activate emsdk properly following the instructions on the emscripten site.
- Run source /path/to/emsdk/emsdk_env.sh.
- Run emmake make in project's root directory.
- main.js and main.wasm should appear inside build/wasm/ directory.

## OpenCV: libraries we use

OpenCV modules are compiled as static libraries.

These are the reasons for each library, the functions and classes from each module:

- core: Mat
- improc: cvtColor ([two times](https://github.com/UNSLAM/emscripten-bindings/blob/5a4afcf9539b54b9e8c22db6d74f360511e33441/util/image_converter.cc#L12))
- feature2d: FAST ([two times](https://github.com/UNSLAM/emscripten-bindings/blob/53e0443a0b423d71ce00dad3bd3da9796c3cea89/feature/orb_extractor.cc#L276))
- flann: not used
- calib3d: not used, but on slam is used to undirstort [cv::undistortPoint three times in perspective.cc](https://github.com/stella-cv/stella_vslam/blob/2c61d3434c31ff32ed99666bc3699c6845d6301b/src/stella_vslam/camera/perspective.cc)
- photo: not used
- video: not used
