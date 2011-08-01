rock_library(dummyproject
    SOURCES Dummy.cpp)

rock_executable(dummyproject_bin Main.cpp
    DEPS dummyproject)

