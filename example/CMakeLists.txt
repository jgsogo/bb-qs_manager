cmake_minimum_required(VERSION 2.8)

#------------------------------------------
#   List source files
file(GLOB _headers "*.h")
file(GLOB _src "*.cpp")
set(SRC_FILES
	${_headers}
	${_src}	
	)
	
if(WIN32)
	# remove warning C4275: se ha utilizado una interfaz no de DLL class
	# remove warning C4996: 'standard function': This function or variable may be unsafe...
	# remove warning C4251: 'atribute' : class 'name' necesita tener una interfaz DLL...
	add_definitions(-D_CRT_SECURE_NO_WARNINGS /wd4275 /wd4251)
endif(WIN32)

add_executable(qs_manager ${SRC_FILES})
 
 set_target_properties(qs_manager PROPERTIES
  LINKER_LANGUAGE CXX
  FOLDER "examples/core")