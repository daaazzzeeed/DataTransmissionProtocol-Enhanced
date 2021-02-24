cmake_minimum_required(VERSION 3.17)
project(DataTransmissionProtocol_Enhanced)

set(CMAKE_CXX_STANDARD 17)

add_executable(DataTransmissionProtocol_Enhanced main.cpp Router.cpp Router.h Device.cpp Device.h utilities.h)