include_directories(${PROJECT_SOURCE_DIR})
add_executable(vm_test test.c)
add_dependencies(vm_test vm_test)
add_test(vm_test vm_test)
add_test(vm_test_valgrind valgrind
    --error-exitcode=1 --read-var-info=yes
    --leak-check=full ./vm_test)
target_link_libraries(vm_test vml cmocka)

