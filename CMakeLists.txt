# CMake'in minimum sürümünü belirt
cmake_minimum_required(VERSION 3.10)

# Proje adını tanımla
project(GridDetector_CPP)

# C++ 11 standardını kullan (veya 14, 17)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OpenCV'yi bul
# Bu komut, sisteminize kurulu OpenCV'yi bulmaya çalışır
find_package(OpenCV REQUIRED)

# OpenCV'nin include dizinlerini ekle
include_directories(${OpenCV_INCLUDE_DIRS})

# Derlenecek programı (executable) ve kaynak dosyalarını belirt
add_executable(GridDetector main.cpp)

# Programı OpenCV kütüphanelerine bağla
target_link_libraries(GridDetector ${OpenCV_LIBS})