add_library(HEADER_ONLY_TARGET dct.tcc filter.tcc 
                               stat.tcc raftmath.tcc 
                               streamingstat.tcc)
set_target_properties(HEADER_ONLY_TARGET PROPERTIES LINKER_LANGUAGE CXX)
