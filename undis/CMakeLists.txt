add_library(undis_lib
    command.cpp command.h
    commandtypes.h
    connectionhandler.cpp connectionhandler.h
    kvstore.cpp kvstore.h
    serializer.h
    server.cpp server.h
    threadpool.cpp threadpool.h
)
add_executable(main main.cpp)
target_link_libraries(main undis_lib)
