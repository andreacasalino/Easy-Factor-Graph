if(NOT gRPC_ROOT_DIR)
  message(FATAL_ERROR "gRPC_ROOT_DIR is required for FindGrpc.cmake")
endif()

include(FindPackageHandleStandardArgs)

set(GRPC_LIBRARY_TYPE STATIC)

# Find gRPC include directory
find_path(GRPC_INCLUDE_DIR
  PATH_SUFFIXES
    include
  NAMES
    grpc/grpc.h
  HINTS
    ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_INCLUDE_DIR)

# Create imported target gRPC::address_sorting
find_library(GPR_ADDRESS_SORTING_LIBRARY
  NAMES
   address_sorting
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPR_ADDRESS_SORTING_LIBRARY)
if(GPR_ADDRESS_SORTING_LIBRARY AND NOT TARGET gRPC::address_sorting)
  add_library(gRPC::address_sorting ${GRPC_LIBRARY_TYPE} IMPORTED)

  if(WIN32)
     set(LIBRARIES_M "")
     set(LIBRARIES_PTHREAD "")
     set(LIBRARIES_DL "")
  else()
     set(LIBRARIES_M "m")
     set(LIBRARIES_PTHREAD "pthread")
     set(LIBRARIES_DL "dl")
  endif()

  set_target_properties(gRPC::address_sorting PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPR_ADDRESS_SORTING_LIBRARY}
    INTERFACE_LINK_LIBRARIES "${LIBRARIES_PTHREAD};${LIBRARIES_DL}"
  )
endif()
# Create imported target gRPC::gpr
find_library(GPR_LIBRARY
  NAMES
   gpr
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPR_LIBRARY)
if(GPR_LIBRARY AND NOT TARGET gRPC::gpr)
  add_library(gRPC::gpr ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::gpr APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::gpr PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPR_LIBRARY}
    INTERFACE_LINK_LIBRARIES "${LIBRARIES_PTHREAD};${LIBRARIES_DL}"
  )
endif()

# Create imported target gRPC::grpc
find_library(GRPC_LIBRARY
  NAMES
   grpc
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_LIBRARY)
if(GRPC_LIBRARY AND NOT TARGET gRPC::grpc)
  add_library(gRPC::grpc ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)

  set_target_properties(gRPC::grpc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GRPC_LIBRARY}
    INTERFACE_LINK_LIBRARIES "OpenSSL::SSL;OpenSSL::Crypto;ZLIB::ZLIB;c-ares::cares;${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::address_sorting;gRPC::gpr"
  )
endif()

# Create imported target gRPC::grpc_cronet
find_library(GRPC_CRONET_LIBRARY
  NAMES
   grpc_cronet
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_CRONET_LIBRARY)
if(GRPC_CRONET_LIBRARY AND NOT TARGET gRPC::grpc_cronet)
  add_library(gRPC::grpc_cronet ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc_cronet APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc_cronet PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GRPC_CRONET_LIBRARY}
    INTERFACE_LINK_LIBRARIES "OpenSSL::SSL;OpenSSL::Crypto;ZLIB::ZLIB;c-ares::cares;${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::address_sorting;gRPC::gpr"
  )
endif()

# Create imported target gRPC::grpc_unsecure
find_library(GRPC_GRPC_UNSECURE_LIBRARY
NAMES
  grpc_unsecure
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_GRPC_UNSECURE_LIBRARY)

if(GRPC_GRPC_UNSECURE_LIBRARY AND NOT TARGET gRPC::grpc_unsecure)
  add_library(gRPC::grpc_unsecure ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc_unsecure APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc_unsecure PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "ZLIB::ZLIB;c-ares::cares;gRPC::address_sorting;${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::gpr"
    IMPORTED_LOCATION ${GRPC_GRPC_UNSECURE_LIBRARY}
  )
endif()

# Create imported target gRPC::grpc++
find_library(GRPC_GRPC++_LIBRARY
NAMES
  grpc++
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_GRPC++_LIBRARY)

if(GRPC_GRPC++_LIBRARY AND NOT TARGET gRPC::grpc++)
  add_library(gRPC::grpc++ ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc++ APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc++ PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "OpenSSL::SSL;OpenSSL::Crypto;protobuf::libprotobuf;${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::grpc;gRPC::gpr"
    IMPORTED_LOCATION ${GRPC_GRPC++_LIBRARY}
  )
endif()

# Create imported target gRPC::grpc++_cronet
find_library(GRPC_GRPC++_CRONET_LIBRARY
NAMES
  grpc++_cronet
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_GRPC++_CRONET_LIBRARY)
if(GRPC_GRPC++_CRONET_LIBRARY AND NOT TARGET gRPC::grpc++_cronet)
  add_library(gRPC::grpc++_cronet ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc++_cronet APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc++_cronet PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "OpenSSL::SSL;OpenSSL::Crypto;protobuf::libprotobuf;${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::gpr;gRPC::grpc_cronet;gRPC::grpc"
    IMPORTED_LOCATION ${GRPC_GRPC++_CRONET_LIBRARY}
  )
endif()

# Create imported target gRPC::grpc++_error_details
find_library(GRPC_GRPC++_ERROR_DETAILS_LIBRARY
NAMES
  grpc++_error_details
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_GRPC++_ERROR_DETAILS_LIBRARY)
if(GRPC_GRPC++_ERROR_DETAILS_LIBRARY AND NOT TARGET gRPC::grpc++_error_details)
  add_library(gRPC::grpc++_error_details ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc++_error_details APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc++_error_details PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "protobuf::libprotobuf;${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::grpc++"
    IMPORTED_LOCATION ${GRPC_GRPC++_ERROR_DETAILS_LIBRARY}
  )
endif()

# Create imported target gRPC::grpc++_reflection
find_library(GRPC_GRPC++_REFLECTION_LIBRARY
  NAMES
    grpc++_reflection
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_GRPC++_REFLECTION_LIBRARY)
if(GRPC_GRPC++_REFLECTION_LIBRARY AND NOT TARGET gRPC::grpc++_reflection)
  add_library(gRPC::grpc++_reflection ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc++_error_details APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc++_reflection PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "protobuf::libprotobuf;${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::grpc++;gRPC::grpc"
    IMPORTED_LOCATION ${GRPC_GRPC++_REFLECTION_LIBRARY}
  )
endif()

# Create imported target gRPC::grpc++_unsecure
find_library(GRPC_GRPC++_UNSECURE_LIBRARY
NAMES
  grpc++_unsecure
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_GRPC++_UNSECURE_LIBRARY)
if(GRPC_GRPC++_UNSECURE_LIBRARY AND NOT TARGET gRPC::grpc++_unsecure)
  add_library(gRPC::grpc++_unsecure ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc++_unsecure APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc++_unsecure PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "protobuf::libprotobuf;${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::gpr;gRPC::grpc_unsecure"
    IMPORTED_LOCATION ${GRPC_GRPC++_UNSECURE_LIBRARY}
  )
endif()

# Create imported target gRPC::grpc_plugin_support
find_library(GRPC_PLUGIN_SUPPORT_LIBRARY
NAMES
  grpc_plugin_support
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_PLUGIN_SUPPORT_LIBRARY)
if(GRPC_PLUGIN_SUPPORT_LIBRARY AND NOT TARGET gRPC::grpc_plugin_support)
  add_library(gRPC::grpc_plugin_support ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc_plugin_support APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc_plugin_support PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "${LIBRARIES_M};${LIBRARIES_PTHREAD};gRPC::grpc;gRPC::gpr"
    IMPORTED_LOCATION ${GRPC_PLUGIN_SUPPORT_LIBRARY}
  )
endif()

# Create imported target gRPC::grpc_csharp_ext
find_library(GRPC_CSHARP_LIBRARY
NAMES
  grpc_csharp_ext
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_CSHARP_LIBRARY)
if(GRPC_CSHARP_LIBRARY AND NOT TARGET gRPC::grpc_csharp_ext)
  add_library(gRPC::grpc_csharp_ext ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_target_properties(gRPC::grpc_csharp_ext PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GRPC_CSHARP_LIBRARY}
  )
endif()

# ------------------------------------------------ EXECUTABLES -----------------------------------------------------
# Create imported target gRPC::grpc_cpp_plugin
find_program(GRPC_CPP_PLUGIN
  NAMES
    grpc_cpp_plugin
  PATH_SUFFIXES
   bin
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_CPP_PLUGIN)
if(GRPC_CPP_PLUGIN AND NOT TARGET gRPC::grpc_cpp_plugin)
  add_executable(gRPC::grpc_cpp_plugin IMPORTED)
  set_target_properties(gRPC::grpc_cpp_plugin PROPERTIES
    IMPORTED_LOCATION ${GRPC_CPP_PLUGIN}
  )
endif()

# Create imported target gRPC::grpc_csharp_plugin
find_program(GRPC_CSHARP_PLUGIN
  NAMES
    grpc_csharp_plugin
  PATH_SUFFIXES
   bin
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_CSHARP_PLUGIN)
if(GRPC_CSHARP_PLUGIN AND NOT TARGET gRPC::grpc_csharp_plugin)
  add_executable(gRPC::grpc_csharp_plugin IMPORTED)
  set_target_properties(gRPC::grpc_csharp_plugin PROPERTIES
    IMPORTED_LOCATION ${GRPC_CSHARP_PLUGIN}
  )
endif()

# Create imported target gRPC::grpc_node_plugin
find_program(GRPC_NODE_PLUGIN
  NAMES
    grpc_node_plugin
  PATH_SUFFIXES
   bin
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_NODE_PLUGIN)
if(GRPC_NODE_PLUGIN AND NOT TARGET gRPC::grpc_node_plugin)
  add_executable(gRPC::grpc_node_plugin IMPORTED)
  set_target_properties(gRPC::grpc_node_plugin PROPERTIES
    IMPORTED_LOCATION ${GRPC_NODE_PLUGIN}
  )
endif()

# Create imported target gRPC::grpc_objective_c_plugin
find_program(GRPC_OBJECTIVE_C_PLUGIN
  NAMES
    grpc_objective_c_plugin
  PATH_SUFFIXES
   bin
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_OBJECTIVE_C_PLUGIN)
if(GRPC_OBJECTIVE_C_PLUGIN AND NOT TARGET gRPC::grpc_objective_c_plugin)
  add_executable(gRPC::grpc_objective_c_plugin IMPORTED)
  set_target_properties(gRPC::grpc_objective_c_plugin PROPERTIES
    IMPORTED_LOCATION ${GRPC_OBJECTIVE_C_PLUGIN}
  )
endif()

# Create imported target gRPC::grpc_php_plugin
find_program(GRPC_PHP_PLUGIN
  NAMES
    grpc_php_plugin
  PATH_SUFFIXES
   bin
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_PHP_PLUGIN)
if(GRPC_PHP_PLUGIN AND NOT TARGET gRPC::grpc_php_plugin)
  add_executable(gRPC::grpc_php_plugin IMPORTED)
  set_target_properties(gRPC::grpc_php_plugin PROPERTIES
    IMPORTED_LOCATION ${GRPC_PHP_PLUGIN}
  )
endif()

# Create imported target gRPC::grpc_python_plugin
find_program(GRPC_PYTHON_PLUGIN
  NAMES
    grpc_python_plugin
  PATH_SUFFIXES
   bin
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_PYTHON_PLUGIN)
if(GRPC_PYTHON_PLUGIN AND NOT TARGET gRPC::grpc_python_plugin)
  add_executable(gRPC::grpc_python_plugin IMPORTED)
  set_target_properties(gRPC::grpc_python_plugin PROPERTIES
    IMPORTED_LOCATION ${GRPC_PYTHON_PLUGIN}
  )
endif()

# Create imported target gRPC::grpc_ruby_plugin
find_program(GRPC_RUBY_PLUGIN
  NAMES
    grpc_ruby_plugin
  PATH_SUFFIXES
   bin
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_RUBY_PLUGIN)
if(GRPC_RUBY_PLUGIN AND NOT TARGET gRPC::grpc_ruby_plugin)
  add_executable(gRPC::grpc_ruby_plugin IMPORTED)
  set_target_properties(gRPC::grpc_ruby_plugin PROPERTIES
    IMPORTED_LOCATION ${GRPC_RUBY_PLUGIN}
  )
endif()

find_package_handle_standard_args(gRPC DEFAULT_MSG
  GPR_ADDRESS_SORTING_LIBRARY
  GPR_LIBRARY
  GRPC_LIBRARY
  GRPC_GRPC++_LIBRARY
  GRPC_GRPC_UNSECURE_LIBRARY
  GRPC_INCLUDE_DIR
  GRPC_GRPC++_REFLECTION_LIBRARY
  GRPC_GRPC++_UNSECURE_LIBRARY
  GRPC_CPP_PLUGIN
  GRPC_CSHARP_PLUGIN
  GRPC_NODE_PLUGIN
  GRPC_OBJECTIVE_C_PLUGIN
  GRPC_PHP_PLUGIN
  GRPC_PYTHON_PLUGIN
  GRPC_RUBY_PLUGIN
)
