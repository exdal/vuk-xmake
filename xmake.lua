set_project("vuk")

add_repositories("exdal https://github.com/exdal/xmake-repo.git")
add_requires("vulkan-memory-allocator v3.1.0")
add_requires("concurrentqueue v1.0.4")
add_requires("fmt 11.0.2")
add_requires("plf_colony v7.41")
add_requires("robin-hood-hashing 3.11.5")
add_requires("stb 2024.06.01")
add_requires("function2 4.2.4")
add_requires("spirv-cross 1.3.268+0")
add_requires("small_vector 2024.12.23")

target("vuk")
    set_kind("static")
    add_languages("cxx20")
    add_files("src/**.cpp")
    add_includedirs("include/", { public = true })

    -- public packages
    add_packages(
        "fmt",
        "robin-hood-hashing",
        "plf_colony",
        "function2",
        "small_vector",
        "vulkan-memory-allocator",
        { public = true })

    -- private packages
    add_packages(
        "spirv-cross",
        "concurrentqueue",
        "stb",
        { public = false })

    on_config(function (target)
        if target:has_tool("cc", "cl") then
            target:add("defines", "VUK_COMPILER_MSVC=1", { force = true, public = true })
            target:add("cxflags", "/permissive- /Zc:char8_t- /wd4068", { public = false })
        elseif target:has_tool("cc", "clang_cl") then
            target:add("defines", "VUK_COMPILER_CLANGCL=1", { force = true, public = true })
            target:add("cxflags", "-Wno-nullability-completeness", { public = false })
        elseif target:has_tool("cc", "clang") then
            target:add("defines", "VUK_COMPILER_CLANGPP=1", { force = true, public = true })
            target:add("cxflags", "-fno-char8_t -Wno-nullability-completeness", { public = false })
        elseif target:has_tool("cc", "gcc") then
            target:add("defines", "VUK_COMPILER_CLANGPP=1", { force = true, public = true })
            target:add("cxflags", "-fno-char8_t", { public = false })
        end
    end)
target_end()
