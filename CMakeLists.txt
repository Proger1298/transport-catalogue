cmake_minimum_required(VERSION 3.18)

project(transport_catalogue CXX)
set(CMAKE_CXX_STANDARD 20)

# Исходные файлы проекта
set(SOURCES
    main.cpp
    domain.cpp
    geo.cpp
    json_builder.cpp
    json_reader.cpp
    json.cpp
    map_renderer.cpp
    request_handler.cpp
    svg.cpp
    transport_catalogue.cpp
    transport_router.cpp
)

# Заголовочные файлы
set(HEADERS
    domain.h
    geo.h
    json_builder.h
    json_reader.h
    json.h
    map_renderer.h
    request_handler.h
    svg.h
    transport_catalogue.h
    transport_router.h
)

# Создание исполняемого файла
add_executable(transport_catalogue ${SOURCES} ${HEADERS})
