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

  if(WIN32)
    set(WSOCK32_LIBRARY "wsock32")
    set(WS2_LIBRARY "ws2_32")
    set(CRYPTO32_LIBRARY "crypt32")
  else()
    set(WSOCK32_LIBRARY "")
    set(WS2_LIBRARY "")
    set(CRYPTO32_LIBRARY "")
  endif()


# Create imported target  gRPC::absl_bad_optional_access
find_library(GPRC_ABSL_BAD_OPTIONAL_ACCESS_LIBRARY
  NAMES
   absl_bad_optional_access
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_BAD_OPTIONAL_ACCESS_LIBRARY)
if(GPRC_ABSL_BAD_OPTIONAL_ACCESS_LIBRARY AND NOT TARGET  gRPC::absl_bad_optional_access)
  add_library( gRPC::absl_bad_optional_access ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_bad_optional_access APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_bad_optional_access PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_BAD_OPTIONAL_ACCESS_LIBRARY}
    INTERFACE_LINK_LIBRARIES "gRPC::absl_raw_logging_internal"
  )
endif()

# Create imported target  gRPC::absl_base
find_library(GPRC_ABSL_BASE_LIBRARY
  NAMES
   absl_base
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_BASE_LIBRARY)
if(GPRC_ABSL_BASE_LIBRARY AND NOT TARGET  gRPC::absl_base)
  add_library( gRPC::absl_base ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_base APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_base PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_BASE_LIBRARY}
    INTERFACE_LINK_LIBRARIES "gRPC::absl_dynamic_annotations;gRPC::absl_log_severity;gRPC::absl_raw_logging_internal;gRPC::absl_spinlock_wait;Threads::Threads"
  )
endif()

# Create imported target  gRPC::absl_dynamic_annotations
find_library(GPRC_ABSL_DYNAMIC_ANNOTATIONS_LIBRARY
  NAMES
   absl_dynamic_annotations
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_DYNAMIC_ANNOTATIONS_LIBRARY)
if(GPRC_ABSL_DYNAMIC_ANNOTATIONS_LIBRARY AND NOT TARGET  gRPC::absl_dynamic_annotations)
  add_library( gRPC::absl_dynamic_annotations ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_dynamic_annotations APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_dynamic_annotations PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_DYNAMIC_ANNOTATIONS_LIBRARY}
  )
endif()

# Create imported target  gRPC::absl_int128
find_library(GPRC_ABSL_INT_128_LIBRARY
  NAMES
   absl_int128
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_INT_128_LIBRARY)
if(GPRC_ABSL_INT_128_LIBRARY AND NOT TARGET  gRPC::absl_int128)
  add_library( gRPC::absl_int128 ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_int128 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_int128 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_INT_128_LIBRARY}
  )
endif()

# Create imported target  gRPC::absl_log_severity
find_library(GPRC_ABSL_LOG_SEVERITY_LIBRARY
  NAMES
   absl_log_severity
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_LOG_SEVERITY_LIBRARY)
if(GPRC_ABSL_LOG_SEVERITY_LIBRARY AND NOT TARGET  gRPC::absl_log_severity)
  add_library( gRPC::absl_log_severity ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_log_severity APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_log_severity PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_LOG_SEVERITY_LIBRARY}
  )
endif()

# Create imported target  gRPC::absl_raw_logging_internal
find_library(GPRC_ABSL_RAW_LOGGING_INTERNAL_LIBRARY
  NAMES
   absl_raw_logging_internal
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_RAW_LOGGING_INTERNAL_LIBRARY)
if(GPRC_ABSL_RAW_LOGGING_INTERNAL_LIBRARY AND NOT TARGET  gRPC::absl_raw_logging_internal)
  add_library( gRPC::absl_raw_logging_internal ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_raw_logging_internal APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_raw_logging_internal PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_RAW_LOGGING_INTERNAL_LIBRARY}
    INTERFACE_LINK_LIBRARIES "gRPC::absl_log_severity"

  )
endif()

# Create imported target  gRPC::absl_spinlock_wait
find_library(GPRC_ABSL_SPINLOCK_WAIT_LIBRARY
  NAMES
   absl_spinlock_wait
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_SPINLOCK_WAIT_LIBRARY)
if(GPRC_ABSL_SPINLOCK_WAIT_LIBRARY AND NOT TARGET  gRPC::absl_spinlock_wait)
  add_library( gRPC::absl_spinlock_wait ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_spinlock_wait APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_spinlock_wait PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_SPINLOCK_WAIT_LIBRARY}

  )
endif()

# Create imported target  gRPC::absl_strings
find_library(GPRC_ABSL_STRINGS_LIBRARY
  NAMES
   absl_strings
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_STRINGS_LIBRARY)
if(GPRC_ABSL_STRINGS_LIBRARY AND NOT TARGET  gRPC::absl_strings)
  add_library( gRPC::absl_strings ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_strings APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_strings PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_STRINGS_LIBRARY}
    INTERFACE_LINK_LIBRARIES "gRPC::absl_strings_internal;gRPC::absl_base;gRPC::absl_int128;gRPC::absl_raw_logging_internal;gRPC::absl_throw_delegate"

  )
endif()

# Create imported target  gRPC::absl_strings_internal
find_library(GPRC_ABSL_STRINGS_INTERNAL_LIBRARY
  NAMES
   absl_strings_internal
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_STRINGS_INTERNAL_LIBRARY)
if(GPRC_ABSL_STRINGS_INTERNAL_LIBRARY AND NOT TARGET  gRPC::absl_strings_internal)
  add_library( gRPC::absl_strings_internal ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_strings_internal APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_strings_internal PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_STRINGS_INTERNAL_LIBRARY}
    INTERFACE_LINK_LIBRARIES "gRPC::absl_raw_logging_internal"

  )
endif()

# Create imported target  gRPC::absl_throw_delegate
find_library(GPRC_ABSL_THROW_DELEGATE_LIBRARY
  NAMES
   absl_throw_delegate
  PATH_SUFFIXES
   lib
  HINTS
   ${gRPC_ROOT_DIR}
)
mark_as_advanced(GPRC_ABSL_TROW_DELEGATE_LIBRARY)
if(GPRC_ABSL_THROW_DELEGATE_LIBRARY AND NOT TARGET  gRPC::absl_throw_delegate)
  add_library( gRPC::absl_throw_delegate ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET  gRPC::absl_throw_delegate APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties( gRPC::absl_throw_delegate PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPRC_ABSL_THROW_DELEGATE_LIBRARY}
    INTERFACE_LINK_LIBRARIES "gRPC::absl_raw_logging_internal"

  )
endif()

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

  set_target_properties(gRPC::address_sorting PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GPR_ADDRESS_SORTING_LIBRARY}
    INTERFACE_LINK_LIBRARIES "${WSOCK32_LIBRARY};${WS2_LIBRARY};${CRYPTO32_LIBRARY}"
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
    INTERFACE_LINK_LIBRARIES "gRPC::absl_strings"
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
    INTERFACE_LINK_LIBRARIES "${WSOCK32_LIBRARY};${WS2_LIBRARY};${CRYPTO32_LIBRARY};OpenSSL::SSL;OpenSSL::Crypto;ZLIB::ZLIB;c-ares::cares;gRPC::address_sorting;gRPC::gpr;gRPC::upb"
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
    INTERFACE_LINK_LIBRARIES "${WSOCK32_LIBRARY};${WS2_LIBRARY};${CRYPTO32_LIBRARY};OpenSSL::SSL;OpenSSL::Crypto;ZLIB::ZLIB;c-ares::cares;gRPC::address_sorting;gRPC::gpr;gRPC::upb"
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
    INTERFACE_LINK_LIBRARIES "${WSOCK32_LIBRARY};${WS2_LIBRARY};${CRYPTO32_LIBRARY};ZLIB::ZLIB;c-ares::cares;gRPC::address_sorting;gRPC::gpr;gRPC::upb"
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
    INTERFACE_LINK_LIBRARIES "${WSOCK32_LIBRARY};${WS2_LIBRARY};${CRYPTO32_LIBRARY};OpenSSL::SSL;OpenSSL::Crypto;protobuf::libprotobuf;gRPC::grpc;gRPC::gpr;gRPC::upb"
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
    INTERFACE_LINK_LIBRARIES "OpenSSL::SSL;OpenSSL::Crypto;protobuf::libprotobuf;gRPC::gpr;gRPC::grpc_cronet;gRPC::grpc"
    IMPORTED_LOCATION ${GRPC_GRPC++_CRONET_LIBRARY}
  )
endif()

# Create imported target gRPC::grpc++_alts
find_library(GRPC_GRPC++_ALTS_LIBRARY
NAMES
  grpc++_alts
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_GRPC++_ALTS_LIBRARY)
if(GRPC_GRPC++_ALTS_LIBRARY AND NOT TARGET gRPC::grpc++_alts)
  add_library(gRPC::grpc++_alts ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpc++_alts APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpc++_alts PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "${WSOCK32_LIBRARY};${WS2_LIBRARY};${CRYPTO32_LIBRARY};OpenSSL::SSL;OpenSSL::Crypto;protobuf::libprotobuf;gRPC::gpr;gRPC::grpc_cronet;gRPC::grpc;gRPC::upb"
    IMPORTED_LOCATION ${GRPC_GRPC++_ALTS_LIBRARY}
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
    INTERFACE_LINK_LIBRARIES "${WSOCK32_LIBRARY};${WS2_LIBRARY};${CRYPTO32_LIBRARY};protobuf::libprotobuf;gRPC::grpc++"
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
    INTERFACE_LINK_LIBRARIES "protobuf::libprotobuf;gRPC::grpc++;gRPC::grpc"
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
    INTERFACE_LINK_LIBRARIES "${WSOCK32_LIBRARY};${WS2_LIBRARY};${CRYPTO32_LIBRARY};protobuf::libprotobuf;gRPC::gpr;gRPC::grpc_unsecure;gRPC::upb"
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
    INTERFACE_LINK_LIBRARIES "gRPC::grpc;gRPC::gpr"
    IMPORTED_LOCATION ${GRPC_PLUGIN_SUPPORT_LIBRARY}
  )
endif()

# Create imported target gRPC::grpcpp_channelz
find_library(GRPC_CHANNELZ_LIBRARY
NAMES
  grpcpp_channelz
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_CHANNELZ_LIBRARY)
if(GRPC_CHANNELZ_LIBRARY AND NOT TARGET gRPC::grpcpp_channelz)
  add_library(gRPC::grpcpp_channelz ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpcpp_channelz APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpcpp_channelz PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "protobuf::libprotobuf;gRPC::grpc++;gRPC::grpc"
    IMPORTED_LOCATION ${GRPC_CHANNELZ_LIBRARY}
  )
endif()

# Create imported target gRPC::grpcpp_channelz
find_library(GRPC_CHANNELZ_LIBRARY
NAMES
  grpcpp_channelz
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_CHANNELZ_LIBRARY)
if(GRPC_CHANNELZ_LIBRARY AND NOT TARGET gRPC::grpcpp_channelz)
  add_library(gRPC::grpcpp_channelz ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::grpcpp_channelz APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::grpcpp_channelz PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "protobuf::libprotobuf;gRPC::grpc++;gRPC::grpc"
    IMPORTED_LOCATION ${GRPC_CHANNELZ_LIBRARY}
  )
endif()

# Create imported target gRPC::upb
find_library(GRPC_UPB_LIBRARY
NAMES
  upb
PATH_SUFFIXES
 lib
HINTS
 ${gRPC_ROOT_DIR}
)
mark_as_advanced(GRPC_UPB_LIBRARY)
if(GRPC_UPB_LIBRARY AND NOT TARGET gRPC::upb)
  add_library(gRPC::upb ${GRPC_LIBRARY_TYPE} IMPORTED)
  set_property(TARGET gRPC::upb APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(gRPC::upb PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
    IMPORTED_LOCATION ${GRPC_UPB_LIBRARY}
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
  GRPC_INCLUDE_DIR
  GPRC_ABSL_BAD_OPTIONAL_ACCESS_LIBRARY
  GPRC_ABSL_BASE_LIBRARY
  GPRC_ABSL_DYNAMIC_ANNOTATIONS_LIBRARY
  GPRC_ABSL_INT_128_LIBRARY
  GPRC_ABSL_LOG_SEVERITY_LIBRARY
  GPRC_ABSL_RAW_LOGGING_INTERNAL_LIBRARY
  GPRC_ABSL_SPINLOCK_WAIT_LIBRARY
  GPRC_ABSL_STRINGS_LIBRARY
  GPRC_ABSL_STRINGS_INTERNAL_LIBRARY
  GPRC_ABSL_THROW_DELEGATE_LIBRARY
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
