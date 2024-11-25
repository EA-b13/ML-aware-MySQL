cmake_minimum_required(VERSION 3.10)
project(SentimentIntegration)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find libTorch
set(CMAKE_PREFIX_PATH "/Users/eashanbajaj/libtorch") # Update this path to your libtorch directory
find_package(Torch REQUIRED)

# Find SentencePiece
find_path(SENTENCEPIECE_INCLUDE_DIR sentencepiece_processor.h PATHS /opt/homebrew/include)
find_library(SENTENCEPIECE_LIBRARY sentencepiece PATHS /opt/homebrew/lib)

if(NOT SENTENCEPIECE_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find SentencePiece include directory.")
endif()

if(NOT SENTENCEPIECE_LIBRARY)
    message(FATAL_ERROR "Could not find SentencePiece library.")
endif()

# Find MySQL
find_path(MYSQL_INCLUDE_DIR mysql.h PATHS /opt/homebrew/Cellar/mysql/9.0.1_6/include/mysql)
find_library(MYSQL_LIBRARY mysqlclient PATHS /opt/homebrew/Cellar/mysql/9.0.1_6/lib)

if(NOT MYSQL_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find MySQL include directory.")
endif()

if(NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "Could not find MySQL library.")
endif()

# Include directories
include_directories(${SENTENCEPIECE_INCLUDE_DIR})
include_directories(${MYSQL_INCLUDE_DIR})
include_directories(include)

# Create the example executable
add_executable(example test.cpp sentiment_analyzer.cpp tokenizer.cpp)
target_link_libraries(example "${TORCH_LIBRARIES}" ${SENTENCEPIECE_LIBRARY})
set_property(TARGET example PROPERTY CXX_STANDARD 17)

# Create the UDF shared library
add_library(sentiment_udf SHARED udf_sentiment.cpp sentiment_analyzer.cpp tokenizer.cpp)
target_link_libraries(sentiment_udf "${TORCH_LIBRARIES}" ${SENTENCEPIECE_LIBRARY} ${MYSQL_LIBRARY})
set_target_properties(sentiment_udf PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# Ensure that the library can find the required dynamic libraries at runtime
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
