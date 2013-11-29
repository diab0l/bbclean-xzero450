set(iTunesCOMWindowsSDK_SOURCES
	iTunesCOMInterface_i.c
)

add_library(iTunesCOMWindowsSDK STATIC ${iTunesCOMWindowsSDK_SOURCES})

set_property(TARGET iTunesCOMWindowsSDK PROPERTY FOLDER "3rd/iTunesCOMWindowsSDK")
