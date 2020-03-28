function(sleip_add_test)
  foreach(test_name ${ARGN})
    add_executable(${test_name} "${test_name}.cpp")

    target_link_libraries(${test_name} PRIVATE ${PROJECT_NAME} Catch2::Catch2)
    set_target_properties(${test_name} PROPERTIES FOLDER "Test")

    if (MSVC)
      target_link_libraries(${test_name} PRIVATE Boost::disable_autolinking)
    endif()

    catch_discover_tests(${test_name})
  endforeach()
endfunction()
