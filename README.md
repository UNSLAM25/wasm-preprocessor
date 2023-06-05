# WASM preprocessor

The preprocessor extracts features (keypoints and descriptors) from image on the phone, as part of a bigger project.
It is meant to run in a browser, so it is compiled with emscripten to web assembly.
This repository deals with the generation of these webassembly files to be used on a web page.  The web-preprocessor repository has a demo of this preprocessor running on web.

The preprocessor is written in C++, with some parts from stella_vslam open source non restrictive project.  It uses OpenCV, so this repository hosts some compiled OpenCV libraries.

# Compiling instructions

- Install [emsdk](https://emscripten.org/docs/getting_started/downloads.html)
- Activate emsdk properly following the instructions on the emscripten site
- Run source /path/to/emsdk/emsdk_env.sh
- Run emmake make in this project's directory

If all works fine, build/wasm/ directory will appear, holding the 3 files you need to use in you web page.  These files are the preprocessor itself and the initialization binding process.  You can see web-preprocessor repository for a sample of use.

makefile file has the recipe to build de whole module.  It takes care of enabling multithreading.

# Wasm object

This files contains the Preprocessor class, with these methods (see perprocessor.h for more details):

- constructor(max num of keypoints)
- features preprocess(buffer, widht, heidht)
    - images are passed from javascript as a data buffer pointer, along with pixel resolution; images are RGBA
    - returns features, as an array object described below
- image getAnnotations()
    - returns de annotated image from the last preprocessing, as an array object described below

The private method toArray() produces the javascript classless array object has this properties:

- array: Uint8Array
- width and height of a matrix
- type, OpenCV CvMat type; 24 for RGBA, 0 for monochromatic, 5 for float
- elementSize in bytes; 4 for RGBA

The features returned by preprocess():

- Mat of 3Nx4 float, containing N features (returned to javascript as UInt8Array of length 48N)
- the first 2N rows have N 256 bits descriptors, 2 rows each
- the last N rows have N keypoints, with the following parameters:
    - col 0: pt.x
    - col 1: pt.y
    - col 2: angle
    - col 3: octave

As octave is a 32 bits int, it was parsed into float without modification, you can get it back in C++ by:

    int octave = *(int*)&col3;

In the consuming end (C++ visual slam), you need to parse this array in this way:

- N = lenght in bytes / 48
- 66% (first 32N bytes) are descriptors as Mat CV_8UC1 Nx32
- 33% (last 16N bytes) are N keypoints with only the abovementioned 4 properties set (other properties are unused by stella_vslam)

# Project structure

- src
    - main.cc emscripten javascript binding code; here so can see what will be visible from javascript
    - preprocessor.h and preprocessor.cc defines the Preprocessor class, including code from feature folder
    - feature and util folders, host files copied from stella_vslam
- libs/opencv2 has the three OpenCV static libraries already compiled for this project
- include/opencv2 has a bunch of headers from OPenCV to include those libraries

# OpenCV libraries

This project uses these OpenCV libraries:

- core, for Mat
- improc, for cvtColor
- feature2d, for FAST

Including a whole OpenCV module only to use one function may sound overkill, but keep in mind that the final build will strip out the unused WASM code.

These OpenCV modules are compiled as static libraries in librs/opencv:

- libopencv_core.a
- libopencv_improc.a
- libopencv_feature2d.a

This repository provides these already compiled static libraries ready to use in the building of the webassmebly preprocessor module.  These libraries where compiled with multithreading enabled.

# How to compile OpenCV into WASM

Follow this steps to compile OpenCV into WASM.

- Download your preferred OpenCV version (3 or 4)
- Install [emsdk](https://emscripten.org/docs/getting_started/downloads.html). 
- Run the following commands:
    
        source /home/squiro/Desktop/emsdk/emsdk_env.sh
        export EMSCRIPTEN=/home/squiro/Desktop/emsdk/upstream/emscripten

- Run:

        emcmake python ./platforms/js/build_js.py build_wasm --build_wasm --threads --simd --cmake_option="-DCMAKE_INSTALL_PREFIX=/home/squiro/Desktop/install" --cmake_option="-DENABLE_CXX11=ON" --cmake_option="-DWITH_EIGEN=ON" --cmake_option="-DBUILD_JASPER=OFF" --cmake_option="-DBUILD_OPENEXR=OFF"

Note from experience: if it fails with unreconigzed arguments, execute it without emcmake wrapper (from python on).
This command will build opencv.js with threading (web-workers) and simd enable. 

- Copy the three desired .a libraries and place them inside the ./libs folder in this project libs/opencv
