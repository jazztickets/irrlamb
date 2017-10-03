execute_process(COMMAND git log --oneline COMMAND wc -l OUTPUT_VARIABLE BUILD_NUMBER OUTPUT_STRIP_TRAILING_WHITESPACE)
if(BUILD_NUMBER STREQUAL "")
	set(BUILD_NUMBER 0)
endif()

configure_file(
	"${SRC}"
	"${DST}"
	@ONLY
)
