ADD_EXECUTABLE( iconvpp_sample sample.cpp )
install( TARGETS iconvpp_sample
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
