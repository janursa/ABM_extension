


add_executable(${APP} main.cpp )
target_compile_features(${APP} PRIVATE cxx_std_17)
target_link_libraries(${APP}  pybind11::module)


