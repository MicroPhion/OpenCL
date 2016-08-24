include_directories(
	.\\
)

ADD_EXECUTABLE(test main.cpp)

TARGET_LINK_LIBRARIES(
    test "${CMAKE_CURRENT_SOURCE_DIR}\\CL\\OpenCL.lib"
)