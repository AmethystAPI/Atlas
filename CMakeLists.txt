# Set Minimum CMAKE Version
cmake_minimum_required(VERSION 3.27.0)

# Set Project Name
project(Bedrock-Insights)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 23)

# Add AdvancedF3 subfolder as a subdirectory
add_subdirectory(AdvancedF3)

# Add BlockNav subfolder as a subdirectory
add_subdirectory(BlockNav)
