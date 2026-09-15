-- include subprojects
includes("lib/commonlibsse-ng")

-- project constants
set_project("SimpleDualWieldBlock")
set_version("0.0.0")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- enable Xbyak support
set_config("skse_xbyak", true)

-- common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- plugin target
target("SimpleDualWieldBlock")
    add_rules("commonlibsse-ng.plugin", {
        name = "Simple Dual Wield Block",
        author = "",
        description = "Simple Dual Wield Block"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")