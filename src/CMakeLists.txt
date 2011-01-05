rock_library(dummyproject
    SOURCES dummy.cpp)

rock_executable(dummyproject_bin main.cpp
    DEPS dummyproject)

