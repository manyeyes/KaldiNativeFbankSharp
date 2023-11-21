
include_directories(${PROJECT_SOURCE_DIR})
set(sources
  feature-fbank.cc
  feature-functions.cc
  feature-window.cc
  #fftsg.c
  mel-computations.cc
  online-feature.cc
  rfft.cc
  KNFWrapper.cpp
)

if(KALDI_NATIVE_FBANK_ENABLE_CHECK)
  list(APPEND sources log.cc)
endif()

add_library(kaldi-native-fbank-core SHARED ${sources})
if(KALDI_NATIVE_FBANK_ENABLE_CHECK)
  target_compile_definitions(kaldi-native-fbank-core PUBLIC KNF_ENABLE_CHECK=1)

  if(KNF_HAVE_EXECINFO_H)
    target_compile_definitions(kaldi-native-fbank-core PRIVATE KNF_HAVE_EXECINFO_H=1)
  endif()

  if(KNF_HAVE_CXXABI_H)
    target_compile_definitions(kaldi-native-fbank-core PRIVATE KNF_HAVE_CXXABI_H=1)
  endif()
endif()

# We are using std::call_once() in log.h,which requires us to link with -pthread
if(NOT WIN32 AND KALDI_NATIVE_FBANK_ENABLE_CHECK)
  target_link_libraries(kaldi-native-fbank-core -pthread)
endif()

install(TARGETS kaldi-native-fbank-core
  DESTINATION lib
)

file(MAKE_DIRECTORY
  DESTINATION
    ${PROJECT_BINARY_DIR}/include/kaldi-native-fbank/csrc
)
file(GLOB_RECURSE all_headers *.h)

file(COPY
  ${all_headers}
  DESTINATION
    ${PROJECT_BINARY_DIR}/include/kaldi-native-fbank/csrc
)

install(FILES ${all_headers}
  DESTINATION include/kaldi-native-fbank/csrc
)
