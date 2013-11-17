function(convert_fonts)
	foreach(f ${ARGN})
		message("command ${BDF2FNT} ${FONTDIR}/${f}.bdf ${FONTOUTDIR}/${f}.fnt")
		execute_process(
			COMMAND ${BDF2FNT} ${FONTDIR}/${f}.bdf ${FONTOUTDIR}/${f}.fnt
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			RESULT_VARIABLE BDF2FNT_DESCRIBE_RESULT
			OUTPUT_VARIABLE BDF2FNT_DESCRIBE_OUTPUT
			ERROR_VARIABLE BDF2FNT_DESCRIBE_ERROR
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)
		message("command ${FNT2FON} ${FONTOUTDIR}/${f}.fnt ${FONTOUTDIR}/${f}.fon")
		execute_process(
			COMMAND ${FNT2FON} ${FONTOUTDIR}/${f}.fnt ${FONTOUTDIR}/${f}.fon
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			RESULT_VARIABLE FNT2FON_DESCRIBE_RESULT
			OUTPUT_VARIABLE FNT2FON_DESCRIBE_OUTPUT
			ERROR_VARIABLE FNT2FON_DESCRIBE_ERROR
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)

	endforeach()
endfunction()

set( ARTWIZ_FONTS
    anorexia
    aqui
    cure
    drift
    edges
    gelly
    glisp
    lime
    mints-mild
    mints-strong
    nu
    snap
    kates
    fkp
)
convert_fonts(${ARTWIZ_FONTS})
