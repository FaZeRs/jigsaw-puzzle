# Take all cpp files in
file(GLOB FOLDER_SOURCE
    "*.cpp"
)

list(REMOVE_ITEM FOLDER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

set(SOURCE
  ${SOURCE}
  ${FOLDER_SOURCE}
)

include_directories(${CMAKE_CURRENT_LIST_DIR})

# Compile into a static library
list(LENGTH FOLDER_SOURCE RES_LEN)
if(NOT RES_LEN EQUAL 0)
  add_library(${CMAKE_PROJECT_NAME}-lib ${SOURCE})
endif()

# Set compilation target
add_executable(${CMAKE_PROJECT_NAME} main.cpp)

if(NOT RES_LEN EQUAL 0)
  target_link_libraries(${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_NAME}-lib)
endif()

# Link with OpenCV
target_link_libraries(${CMAKE_PROJECT_NAME} ${OpenCV_LIBS})