cmake_minimum_required(VERSION 3.16)
project(AsdevLabCore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASDEVLAB_LIBNOVA_SOURCES
    ../../external/libnova/src/julian_day.c
    ../../external/libnova/src/dynamical_time.c
    ../../external/libnova/src/sidereal_time.c
    ../../external/libnova/src/transform.c
    ../../external/libnova/src/nutation.c
    ../../external/libnova/src/aberration.c
    ../../external/libnova/src/apparent_position.c
    ../../external/libnova/src/solar.c
    ../../external/libnova/src/precession.c
    ../../external/libnova/src/proper_motion.c
    ../../external/libnova/src/mercury.c
    ../../external/libnova/src/venus.c
    ../../external/libnova/src/earth.c
    ../../external/libnova/src/mars.c
    ../../external/libnova/src/jupiter.c
    ../../external/libnova/src/saturn.c
    ../../external/libnova/src/uranus.c
    ../../external/libnova/src/neptune.c
    ../../external/libnova/src/pluto.c
    ../../external/libnova/src/vsop87.c
    ../../external/libnova/src/lunar.c
    ../../external/libnova/src/elliptic_motion.c
    ../../external/libnova/src/asteroid.c
    ../../external/libnova/src/comet.c
    ../../external/libnova/src/parabolic_motion.c
    ../../external/libnova/src/utility.c
    ../../external/libnova/src/refraction.c
    ../../external/libnova/src/rise_set.c
    ../../external/libnova/src/angular_separation.c
    ../../external/libnova/src/hyperbolic_motion.c
    ../../external/libnova/src/parallax.c
    ../../external/libnova/src/airmass.c
    ../../external/libnova/src/heliocentric_time.c
    ../../external/libnova/src/elp/elp1.c
    ../../external/libnova/src/elp/elp2.c
    ../../external/libnova/src/elp/elp3.c
    ../../external/libnova/src/elp/elp4.c
    ../../external/libnova/src/elp/elp5.c
    ../../external/libnova/src/elp/elp6.c
    ../../external/libnova/src/elp/elp7.c
    ../../external/libnova/src/elp/elp8.c
    ../../external/libnova/src/elp/elp9.c
    ../../external/libnova/src/elp/elp10.c
    ../../external/libnova/src/elp/elp11.c
    ../../external/libnova/src/elp/elp12.c
    ../../external/libnova/src/elp/elp13.c
    ../../external/libnova/src/elp/elp14.c
    ../../external/libnova/src/elp/elp15.c
    ../../external/libnova/src/elp/elp16.c
    ../../external/libnova/src/elp/elp17.c
    ../../external/libnova/src/elp/elp18.c
    ../../external/libnova/src/elp/elp19.c
    ../../external/libnova/src/elp/elp20.c
    ../../external/libnova/src/elp/elp21.c
    ../../external/libnova/src/elp/elp22.c
    ../../external/libnova/src/elp/elp23.c
    ../../external/libnova/src/elp/elp24.c
    ../../external/libnova/src/elp/elp25.c
    ../../external/libnova/src/elp/elp26.c
    ../../external/libnova/src/elp/elp27.c
    ../../external/libnova/src/elp/elp28.c
    ../../external/libnova/src/elp/elp29.c
    ../../external/libnova/src/elp/elp30.c
    ../../external/libnova/src/elp/elp31.c
    ../../external/libnova/src/elp/elp32.c
    ../../external/libnova/src/elp/elp33.c
    ../../external/libnova/src/elp/elp34.c
    ../../external/libnova/src/elp/elp35.c
    ../../external/libnova/src/elp/elp36.c
)

add_library(asdevlab_core
    src/hardware/mount/onstep_mount_client.cpp
    src/hardware/mount/lx200_parser.cpp
    src/hardware/mount/lx200_socket_transport.cpp
    src/hardware/mount/lx200_connection_manager.cpp
    src/services/motion_service.cpp
    src/services/tracking_service.cpp
    src/services/guide_service.cpp
    src/services/alignment_service.cpp
    src/services/camera_service.cpp
    src/services/image_service.cpp
    src/services/plate_solver_service.cpp
    src/services/scheduler_service.cpp
    src/services/focus_service.cpp
    src/state_machine.cpp
    src/safety_service.cpp
    src/telescope_core.cpp
    src/catalog/catalog_engine.cpp
    src/catalog/coordinate_resolver.cpp
    src/astronomy/astronomy_service.cpp
    src/astronomy/providers/libnova_ephemeris_provider.cpp
    src/astronomy/astronomy_utils.cpp
    src/services/target_service.cpp
    src/services/observation_service.cpp
    src/web/catalog_api.cpp
    src/web/mount_api.cpp
    src/web/target_api.cpp
    src/web/api_utils.cpp
    ${ASDEVLAB_LIBNOVA_SOURCES}
)

target_include_directories(asdevlab_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../LiveStacker/include)
target_include_directories(asdevlab_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../external/libnova/src)
target_compile_definitions(asdevlab_core PRIVATE LIBNOVA_STATIC)
set_property(TARGET asdevlab_core PROPERTY POSITION_INDEPENDENT_CODE ON)

find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(asdevlab_core PRIVATE CURL::libcurl)

add_executable(asdevlab_core_app src/main.cpp)
target_link_libraries(asdevlab_core_app PRIVATE asdevlab_core)

add_executable(asdevlab_lx200_terminal src/lx200_terminal.cpp)
target_link_libraries(asdevlab_lx200_terminal PRIVATE asdevlab_core)

add_executable(asdevlab_web_app src/web_server.cpp)
target_link_libraries(asdevlab_web_app PRIVATE asdevlab_core)

enable_testing()
add_executable(asdevlab_service_test test/test_core_services.cpp)
target_link_libraries(asdevlab_service_test PRIVATE asdevlab_core)
add_executable(asdevlab_motion_service_test test/test_motion_service.cpp)
target_link_libraries(asdevlab_motion_service_test PRIVATE asdevlab_core)
add_executable(asdevlab_goto_solve_test test/test_goto_solve_loop.cpp)
target_link_libraries(asdevlab_goto_solve_test PRIVATE asdevlab_core)
add_executable(asdevlab_proxy_injection_test test/test_proxy_injection.cpp)
target_link_libraries(asdevlab_proxy_injection_test PRIVATE asdevlab_core)
add_executable(asdevlab_onstep_adapter_test test/test_onstep_adapter.cpp)
target_link_libraries(asdevlab_onstep_adapter_test PRIVATE asdevlab_core)
add_executable(asdevlab_lx200_socket_transport_test test/test_lx200_socket_transport.cpp)
target_link_libraries(asdevlab_lx200_socket_transport_test PRIVATE asdevlab_core Threads::Threads)
add_executable(asdevlab_lx200_parser_test test/test_lx200_parser.cpp)
target_link_libraries(asdevlab_lx200_parser_test PRIVATE asdevlab_core)
add_executable(asdevlab_catalog_engine_test test/test_catalog_engine.cpp)
target_link_libraries(asdevlab_catalog_engine_test PRIVATE asdevlab_core)
add_executable(asdevlab_target_service_test test/test_target_service.cpp)
target_link_libraries(asdevlab_target_service_test PRIVATE asdevlab_core)
add_executable(asdevlab_astronomy_service_test test/test_astronomy_service.cpp)
target_link_libraries(asdevlab_astronomy_service_test PRIVATE asdevlab_core)
add_executable(asdevlab_mount_integration_test test/test_mount_integration.cpp)
target_link_libraries(asdevlab_mount_integration_test PRIVATE asdevlab_core)
add_executable(asdevlab_api_integration_test test/test_api_integration.cpp)
target_link_libraries(asdevlab_api_integration_test PRIVATE asdevlab_core)
add_executable(asdevlab_mount_api_test test/test_mount_api.cpp)
target_link_libraries(asdevlab_mount_api_test PRIVATE asdevlab_core)
add_executable(asdevlab_mount_rotator_api_test test/test_mount_rotator_api.cpp)
target_link_libraries(asdevlab_mount_rotator_api_test PRIVATE asdevlab_core)
add_test(NAME asdevlab_mount_rotator_api_test COMMAND asdevlab_mount_rotator_api_test)
add_test(NAME asdevlab_mount_api_test COMMAND asdevlab_mount_api_test)
add_executable(asdevlab_mount_flipmirror_api_test test/test_mount_flipmirror_rest.cpp)
target_link_libraries(asdevlab_mount_flipmirror_api_test PRIVATE asdevlab_core)
add_test(NAME asdevlab_mount_flipmirror_api_test COMMAND asdevlab_mount_flipmirror_api_test)
add_executable(asdevlab_mount_api_altaz_test test/test_mount_api_altaz.cpp)
target_link_libraries(asdevlab_mount_api_altaz_test PRIVATE asdevlab_core)
add_test(NAME asdevlab_mount_api_altaz_test COMMAND asdevlab_mount_api_altaz_test)
add_executable(asdevlab_mount_focuser_api_test test/test_mount_focuser_api.cpp)
target_link_libraries(asdevlab_mount_focuser_api_test PRIVATE asdevlab_core)
add_test(NAME asdevlab_mount_focuser_api_test COMMAND asdevlab_mount_focuser_api_test)
add_executable(asdevlab_terminal_test tools/asdevlab_terminal_test.cpp)
target_link_libraries(asdevlab_terminal_test PRIVATE asdevlab_core)
add_test(NAME asdevlab_smoke COMMAND asdevlab_core_app)
add_test(NAME asdevlab_service_test COMMAND asdevlab_service_test)
add_test(NAME asdevlab_motion_service_test COMMAND asdevlab_motion_service_test)
add_test(NAME asdevlab_goto_solve_test COMMAND asdevlab_goto_solve_test)
add_test(NAME asdevlab_proxy_injection_test COMMAND asdevlab_proxy_injection_test)
add_test(NAME asdevlab_onstep_adapter_test COMMAND asdevlab_onstep_adapter_test)
add_test(NAME asdevlab_lx200_socket_transport_test COMMAND asdevlab_lx200_socket_transport_test)
add_test(NAME asdevlab_lx200_parser_test COMMAND asdevlab_lx200_parser_test)
add_test(NAME asdevlab_catalog_engine_test COMMAND asdevlab_catalog_engine_test)
add_test(NAME asdevlab_astronomy_service_test COMMAND asdevlab_astronomy_service_test)
add_test(NAME asdevlab_mount_integration_test COMMAND asdevlab_mount_integration_test)
