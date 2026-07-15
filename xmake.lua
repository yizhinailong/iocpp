add_rules("mode.debug", "mode.release")

set_languages("c++23")

target("iocpp", function()
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("include")
end)
