project(DeepBeliefSDK)
cmake_minimum_required(VERSION 2.8)

if ( WIN32 ) 
  add_definitions(-DUSE_EIGEN_GEMM=1 -D_WIN32_WINNT=0x0501 -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
  # for <vs2015:
  add_definitions(-Dsnprintf=_snprintf -Dstrcasecmp=lstrcmpiA)
  include_directories(${EIGEN3_INCLUDE_DIR})
else ()
  add_definitions(-Ofast -g)
endif()
message(STATUS "Current list dir: ${CMAKE_CURRENT_LIST_DIR}")

include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/graph")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/math")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/third_party")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/svm")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/utility")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/opengl")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/windows")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib")

option(USE_GEMM "Use GEMM BLAS" OFF)

set(LIBSRCS 
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/libjpcnn.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/basenode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/buffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/convnode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/dropoutnode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/flatnode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/gconvnode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/graph.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/maxnode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/neuronnode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/nodefactory.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/normalizenode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/poolnode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/prepareinput.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/graph/relunode.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_add.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_channels.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_correlate.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_dot.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_gemm.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_local_response.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_margin.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_max.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_scale.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/math/matrix_softmax.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/opengl/glbuffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/opengl/glcontext.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/opengl/glgemm.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/opengl/glprogram.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/pi/mailbox.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/pi/qpu_gemm.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/svm/svm.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/svm/svmutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/third_party/stb_image.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/utility/binary_format.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/utility/cstring_helpers.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/utility/os_image_load.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/utility/os_image_save.cpp
)

if ( WIN32 ) 
  set(LIBSRCS ${LIBSRCS} 
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/windows/mmap.cpp
  )
endif ()

#message(STATUS "lib sources: ${LIBSRCS})
add_library(libjpcnn ${LIBSRCS})

set(TOOLSRCS 
  ${CMAKE_CURRENT_LIST_DIR}/src/tool/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/tool/timer.cpp
)
if ( WIN32 ) 
  set(TOOLSRCS ${TOOLSRCS} 
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/windows/getopt.cpp
  )
endif ()

add_executable(jpcnn ${TOOLSRCS})
target_link_libraries(jpcnn libjpcnn)

set(simple_example_src
  ${CMAKE_CURRENT_LIST_DIR}/../examples/SimpleLinux/main.c
)
add_executable(simple_example ${simple_example_src})
target_link_libraries(simple_example libjpcnn)

if ( OpenCV_DIR ) 
find_package( OpenCV REQUIRED ${OPENCV_REQUIRED_MODULES})
endif ()

if ( OpenCV_FOUND ) 
  include_directories(${OpenCV_INCLUDE_DIRS})
  set(opencv_example_src
    ${CMAKE_CURRENT_LIST_DIR}/../examples/SimpleOpenCV/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../examples/SimpleOpenCV/deepbeliefopencv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../examples/SimpleOpenCV/deepbeliefopencv.h
  )
  add_executable(opencv_example ${opencv_example_src})
  target_link_libraries(opencv_example libjpcnn ${OpenCV_LIBS})
endif ()