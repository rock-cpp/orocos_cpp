rock_library(dummy_project
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(dummy_project_bin Main.cpp
    DEPS dummy_project)

