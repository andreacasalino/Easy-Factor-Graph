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

AddSample(Sample04-HMM-chain)

AddSample(Sample05-Matricial)




AddSamplePath(Sample08-Subgraphing)
