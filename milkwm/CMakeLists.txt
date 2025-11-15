project(
	milkwm
)

file(
	GLOB
	milkwm_SRC
	*.c
)

add_executable(
	milkwm
	${milkwm_SRC}
)

target_link_libraries(
	mdm
	PRIVATE
	MDELib
)

include(GNUInstallDirs)
install(
	TARGETS milkwm
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
