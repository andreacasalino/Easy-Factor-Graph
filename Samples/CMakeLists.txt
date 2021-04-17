add_subdirectory(Utils)

function(AddSample NAME_)
    add_executable(${NAME_} ${NAME_}.cpp )

    target_link_libraries(${NAME_}
		PUBLIC
			Sample-Utils
    )
	
	target_compile_definitions(${NAME_} 
	PUBLIC
		-D SAMPLE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/"
	)
endfunction()

function(AddSamplePath NAME_)
    add_executable(${NAME_} ${CMAKE_CURRENT_SOURCE_DIR}/${NAME_}/${NAME_}.cpp )

    target_link_libraries(${NAME_}
		PUBLIC
			Sample-Utils
    )
	
	target_compile_definitions(${NAME_} 
	PUBLIC
		-D SAMPLE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/${NAME_}/"
	)
endfunction()

AddSample(Sample01-1-Variables)

AddSample(Sample01-2-Factors)

AddSample(Sample02-BeliefPropagation-A)

AddSamplePath(Sample03-BeliefPropagation-B)



# option(BUILD_04 "Build Sample_04" ON)
# if(BUILD_04)
    # add_executable(
        # Sample04
        # ./Sample_04_HMM_chain/Main_04.cpp
    # )

    # target_link_libraries( Sample04
		# PUBLIC
			# EFG::Easy-Factor-Graph
    # )	
	
	# add_custom_command(TARGET Sample04 PRE_BUILD
		# COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Sample_04_graphs
	# )
# endif()

# option(BUILD_05 "Build Sample_05" ON)
# if(BUILD_05)
    # add_executable(
        # Sample05
        # ./Sample_05_Graph_Matrix/Main_05.cpp
    # )

    # target_link_libraries( Sample05
		# PUBLIC
			# EFG::Easy-Factor-Graph
    # )	
	
	# add_custom_command(TARGET Sample04 PRE_BUILD
		# COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Sample_05_graphs
	# )
# endif()

# option(BUILD_06 "Build Sample_06" ON)
# if(BUILD_06)
    # add_executable(
        # Sample06
        # ./Sample_06_Learning_A/Main_06.cpp
    # )

    # target_link_libraries( Sample06
		# PUBLIC
			# EFG::Easy-Factor-Graph
    # )	
	
	# add_custom_command(TARGET Sample06 PRE_BUILD
	    # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Sample_06_Learning_A/Sample_06_graphs ${CMAKE_CURRENT_BINARY_DIR}/Sample_06_graphs
    # )
# endif()

# option(BUILD_07 "Build Sample_06" ON)
# if(BUILD_07)
    # add_executable(
        # Sample07
        # ./Sample_07_Learning_B/Main_07.cpp
    # )

    # target_link_libraries( Sample07
		# PUBLIC
			# EFG::Easy-Factor-Graph
    # )	
	
	# add_custom_command(TARGET Sample07 PRE_BUILD
	    # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Sample_07_Learning_B/Sample_07_graphs ${CMAKE_CURRENT_BINARY_DIR}/Sample_07_graphs
    # )
# endif()

# option(BUILD_08 "Build Sample_06" ON)
# if(BUILD_08)
    # add_executable(
        # Sample08
        # ./Sample_08_subgraph_operation/Main_08.cpp
    # )

    # target_link_libraries( Sample08
		# PUBLIC
			# EFG::Easy-Factor-Graph
    # )	
	
	# add_custom_command(TARGET Sample08 PRE_BUILD
	    # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Sample_08_subgraph_operation/Sample_08_graphs ${CMAKE_CURRENT_BINARY_DIR}/Sample_08_graphs
    # )
# endif()