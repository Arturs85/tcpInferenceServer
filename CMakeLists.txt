cmake_minimum_required(VERSION 2.8)
project(tcpinferenceserver)
#set (CMAKE_CXX_STANDARD 11)


find_package(Torch REQUIRED HINTS "/home/arturs/libtorch")
FIND_PACKAGE( OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIRS} )

add_executable( ${PROJECT_NAME} main.cpp tcpserver.cpp inference.cpp uicom.cpp )
#target_link_libraries( resChanger ${OpenCV_LIBS} )


target_link_libraries(${PROJECT_NAME} ${TORCH_LIBRARIES} ${OpenCV_LIBS} pthread )

