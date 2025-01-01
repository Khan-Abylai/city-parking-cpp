cmake_minimum_required(VERSION 3.25)

project(city_parking)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(OpenCV REQUIRED)
find_package(PkgConfig)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(ncnn REQUIRED)

pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_RTP REQUIRED gstreamer-rtp-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(LIBONNXRUNTIME REQUIRED IMPORTED_TARGET libonnxruntime)

include_directories(${CURL_INCLUDE_DIR})
include_directories(${GST_INCLUDE_DIRS} ${GST_APP_INCLUDE_DIRS} ${GST_RTP_INCLUDE_DIRS})
include_directories(${LIBONNXRUNTIME_INCLUDE_DIRS})

find_path(ONNX_RUNTIME_SESSION_INCLUDE_DIRS onnxruntime_cxx_api.h HINTS /tmp/onnxruntime/include/onnxruntime/core/session/)
find_library(ONNX_RUNTIME_LIB onnxruntime HINTS /tmp/onnxruntime/build/Linux/MinSizeRel)

if (NOT ONNX_RUNTIME_SESSION_INCLUDE_DIRS)
    set(ONNX_RUNTIME_SESSION_INCLUDE_DIRS /home/orangepi/onnxruntime/include/onnxruntime/core/session/)
endif ()

if (NOT ONNX_RUNTIME_LIB)
    find_library(ONNX_RUNTIME_LIB onnxruntime HINTS /home/orangepi/onnxruntime/build/Linux/MinSizeRel)
endif ()

add_executable(city_parking
        src/SharedQueue.h
        src/Main.cpp
        src/client/FrameData.cpp
        src/client/FrameData.h
        src/app/Constants.h
        src/app/LicensePlate.cpp
        src/app/LicensePlate.h
        src/app/TemplateMatching.cpp
        src/app/TemplateMatching.h
        src/app/Utils.cpp
        src/app/Utils.h
        src/app/PackageSender.cpp
        src/app/PackageSender.h
        src/IThreadLauncher.h
        src/ILogger.cpp
        src/ILogger.h
        src/app/Package.h
        src/app/Package.cpp
        src/Config.h
        src/Config.cpp
        src/client/GstreamerReader.cpp
        src/client/GstreamerReader.h
        src/ITimer.h
        src/RandomStringGenerator.h
        src/RandomStringGenerator.cpp
        src/app/FrameProcessor.cpp
        src/app/FrameProcessor.h
        src/app/Car.cpp
        src/app/Car.h
        src/app/Counter.h
        src/app/CalibParams.h
        src/app/CalibParams.cpp
        src/app/CalibParamsUpdater.h
        src/app/CalibParamsUpdater.cpp
        src/client/CameraClientLauncher.cpp
        src/client/CameraClientLauncher.h
        src/Profile.h
        src/app/CarTracker.cpp
        src/app/CarTracker.h
        src/anpr/DetectionNCNN.cpp
        src/anpr/DetectionNCNN.h
        src/anpr/DetectionNCNN.h
        src/anpr/DetectionNCNN.cpp
        src/anpr/LPRecognizerNCNN.cpp
        src/anpr/LPRecognizerNCNN.h)

target_include_directories(city_parking PRIVATE ${ONNX_RUNTIME_SESSION_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${ncnn_INCLUDE_DIRS})
target_link_libraries(city_parking PRIVATE ${ONNX_RUNTIME_LIB} ${OpenCV_LIBS} ${GST_LIBRARIES} ${GST_APP_LIBRARIES} ${GST_RTP_LIBRARIES} nlohmann_json::nlohmann_json ${CURL_LIBRARIES} /usr/local/lib/libcpr.so ${ALL_INTERFACE_TARGET} ncnn)
target_compile_definitions(city_parking PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING>)

file(COPY ./models DESTINATION ./)