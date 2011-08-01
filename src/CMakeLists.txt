rock_library(dummyproject
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(dummyproject_bin Main.cpp
    DEPS dummyproject)

