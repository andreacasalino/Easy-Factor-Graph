function(AddSample NAME_ SOURCE_)
    add_executable(${NAME_} ${SOURCE_} )

    target_link_libraries( ${NAME_}
		PUBLIC
			EFG
    )	
endfunction()

AddSample(Sample01-Distributions ./Sample_01_Potential_handling/Main_01.cpp)

# option(BUILD_02 "Build Sample_02" ON)
# if(BUILD_02)
    # add_executable(
        # Sample02
        # ./Sample_02_Belief_propagation_A/Main_02.cpp
    # )

    # target_link_libraries( Sample02
		# PUBLIC
			# EFG::Easy-Factor-Graph
    # )	
# endif()

# option(BUILD_03 "Build Sample_03" ON)
# if(BUILD_03)
    # add_executable(
        # Sample03
        # ./Sample_03_Belief_propagation_B/Main_03.cpp
    # )

    # target_link_libraries( Sample03
		# PUBLIC
			# EFG::Easy-Factor-Graph
    # )	
	
	# add_custom_command(TARGET Sample03 PRE_BUILD
	    # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Sample_03_Belief_propagation_B/Sample_03_graphs ${CMAKE_CURRENT_BINARY_DIR}/Sample_03_graphs
    # )
# endif()

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