if (__SOCE_CORT_ENGINE__)
#nothing
else(__SOCE_CORT_ENGINE__)
	set(__SOCE_CORT_ENGINE__ 1)
	set(SOCE_CORT_ENGINE_ROOT ${SOCE_ROOT}/cort-engine)

	INCLUDE(${SOCE_ROOT}/utils/CMakeLists.txt)
	INCLUDE_DIRECTORIES( ${SOCE_CORT_ENGINE_ROOT} )
	AUX_SOURCE_DIRECTORY(${SOCE_CORT_ENGINE_ROOT}/src SRC_LIST)
	LIST (APPEND LIBS dl)

endif(__SOCE_CORT_ENGINE__)
