# locate the additional necessary dependencies, if any

# extra source files
SET(sources
    SO3.cpp
    SE3.cpp
    SE3cov.cpp
)

# extra header files
SET(headers
    mrob/SO3.hpp
    mrob/SE3.hpp
    mrob/SE3cov.hpp
)

# create the shared library
ADD_LIBRARY(SE3 SHARED ${sources})

# build tests if enabled
IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)
