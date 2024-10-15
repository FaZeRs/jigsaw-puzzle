add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${OpenCV_LIBS})

file(COPY assets DESTINATION ${EXECUTABLE_OUTPUT_PATH})
