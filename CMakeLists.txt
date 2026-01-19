cmake_minimum_required(VERSION 3.10)
project(Picam)

# C++の標準を設定 (C++17を使用)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OpenCVのインストールを見つける
# REQUIRED: 必須であることを示す
find_package(OpenCV REQUIRED)

# OpenSSLを見つける
find_package(OpenSSL REQUIRED)

# pigpioを見つける
find_library(PIGPIO_LIBRARY NAMES pigpio REQUIRED)

# httplibの設定
include_directories(/home/pi/projects/Pi4-SecurityCamera)

# main.cppから実行ファイルを作成（名前は'main_app')
add_executable(main_app main.cpp)

# 実行ファイルにOpenCVとOpenSSLのライブラリをリンク
target_link_libraries(main_app 
    ${OpenCV_LIBS}
    ${OPENSSL_LIBRARIES}
    ${PIGPIO_LIBRARY}
)

