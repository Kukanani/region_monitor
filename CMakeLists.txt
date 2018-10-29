cmake_minimum_required(VERSION 2.8.3)
project(region_monitor)

# Use C++11
add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
    message_generation
    message_runtime
    pcl_conversions
    pcl_ros
    roscpp
    sensor_msgs
    std_msgs
)

########################################################
## Declare ROS messages, services, and configurations ##
########################################################

add_message_files(
    FILES
    Region.msg
)

add_service_files(
    FILES
    Monitor.srv
)

generate_messages(
    DEPENDENCIES
    sensor_msgs std_msgs
)

###################################
## catkin specific configuration ##
###################################

catkin_package(
    INCLUDE_DIRS include
    # LIBRARIES
    CATKIN_DEPENDS pcl_ros sensor_msgs std_msgs
    # DEPENDS
)

include_directories(
    include
    ${catkin_INCLUDE_DIRS}
)

add_executable(region_monitor src/region_monitor.cpp)
add_dependencies(region_monitor ${orp_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(region_monitor ${catkin_LIBRARIES})