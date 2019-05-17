cmake_minimum_required ( VERSION 3.10.2 )
project ( lognosis )

set ( CMAKE_CXX_STANDARD 17 )

add_library ( lognosis "" )

target_include_directories ( lognosis
    PRIVATE 
        src/
        3rd-party/
)

target_sources ( lognosis
    PRIVATE
        src/lognosis/Logger.cpp
        src/lognosis/loguru.cpp
)

#install ( TARGETS lognosis EXPORT liblognosis ARCHIVE DESTINATION lib)
#install ( DIRECTORY src/lognosis/ DESTINATION include/lognosis FILES_MATCHING PATTERN "*.h" )
#install ( FILES 3rd-party/loguru/loguru.hpp DESTINATION include/loguru )
