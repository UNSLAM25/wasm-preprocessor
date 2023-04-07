# How to compile OpenCV into WASM

Follow this steps to compile OpenCV into WASM.

- Download your preferred OpenCV version (3 or 4)
- Install [emsdk](https://emscripten.org/docs/getting_started/downloads.html). 
- Run the following commands:
    
        source /home/squiro/Desktop/emsdk/emsdk_env.sh
        export EMSCRIPTEN=/home/squiro/Desktop/emsdk/upstream/emscripten

- Run:

        emcmake python ./platforms/js/build_js.py build_wasm --build_wasm --threads --simd --cmake_option="-DCMAKE_INSTALL_PREFIX=/home/squiro/Desktop/install" --cmake_option="-DENABLE_CXX11=ON" --cmake_option="-DWITH_EIGEN=ON" --cmake_option="-DBUILD_JASPER=OFF" --cmake_option="-DBUILD_OPENEXR=OFF"

        (if it fails with unreconigzed arguments, execute it without emcmake wrapper)

- This command will build opencv.js with threading (web-workers) and simd enable. Extract (copy and paste) the .a libraries and place them inside the ./libs folder in this project.

- Then:

        cd build_wasm 
        emcmake install

---

## Relevant stuff that I left here a while ago and I don't remember why

https://github.com/opencv/opencv/issues/19493 

--> https://github.com/opencv/opencv/issues/19493#issuecomment-857167996 <--

https://github.com/opencv/opencv/issues/20313