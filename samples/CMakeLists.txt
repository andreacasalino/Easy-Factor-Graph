add_subdirectory(Helpers)

function(SetUpSample NAME_)
	target_link_libraries(${NAME_} PUBLIC EFG)

	target_link_libraries(${NAME_} PUBLIC Samples-Helpers)

	set_target_properties(${NAME_} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${CMAKE_INSTALL_PREFIX}/bin/")
	install(TARGETS ${NAME_})	
endfunction()

function(AddSample NAME_)
    add_executable(${NAME_} ${NAME_}.cpp )

	target_compile_definitions(${NAME_} PUBLIC
		-D SAMPLE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/"
	)
		
	SetUpSample(${NAME_})
endfunction()

function(AddSamplePath NAME_)
    add_executable(${NAME_} ${CMAKE_CURRENT_SOURCE_DIR}/${NAME_}/${NAME_}.cpp )
	
	target_compile_definitions(${NAME_} 
	PUBLIC
		-D SAMPLE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/${NAME_}/"
	)
	
	SetUpSample(${NAME_})
endfunction()

AddSample(Sample01-1-Variables)

AddSample(Sample01-2-Factors)

AddSample(Sample02-BeliefPropagation-A)

AddSamplePath(Sample03-BeliefPropagation-B)

AddSample(Sample04-HMM-chain)

AddSample(Sample05-Matricial)

AddSamplePath(Sample06-Learning-A)

AddSamplePath(Sample07-Learning-B)

AddSamplePath(Sample08-Subgraphing)
