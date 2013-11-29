set(ZLIB_SOURCES
	adler32.c
	compress.c
	crc32.c
	deflate.c
	gzio.c
	infblock.c
	infcodes.c
	inffast.c
	inflate.c
	inftrees.c
	infutil.c
	maketree.c
	minigzip.c
	trees.c
	uncompr.c
	zutil.c
)

add_library(zlib STATIC ${ZLIB_SOURCES})

set_property(TARGET zlib PROPERTY FOLDER "tools/zlib")

