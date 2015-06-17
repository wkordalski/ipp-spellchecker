if(${BUILD_SHARED_LIBS})

    find_package (PythonLibs)
    find_package (Boost COMPONENTS python3)

    if(PYTHONLIBS_FOUND AND Boost_PYTHON3_FOUND)
        include_directories(${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
        
        add_library(pyippdict SHARED wrapper.cpp)
        target_link_libraries(pyippdict dictionary ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
    endif()

endif()