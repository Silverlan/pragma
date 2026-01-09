import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

third_party_libs_dir = curDir +"/third_party_libs"
os.chdir(third_party_libs_dir)
get_gh_submodule("7zpp",                "Silverlan/7zip-cpp",               "afed5bccd4faf4f825a640aa94a7ed78f54c178c")
get_gh_submodule("bvh",                 "madmann91/bvh",                    "ac41ab8")
get_gh_submodule("bzip2",               "Silverlan/bzip2",                  "6f08ffc2409d34a8f0f06ab3b0291bced4e9d08c")
get_gh_submodule("clip2tri",            "raptor/clip2tri",                  "f62a734d22733814b8a970ed8a68a4d94c24fa5f")
get_gh_submodule("efsw",                "SpartanJ/efsw",                    "1bb814c")
get_gl_submodule("eigen",               "libeigen/eigen",                   "b66188b5") # v3.4.1
get_gh_submodule("exprtk",              "ArashPartow/exprtk",               "f46bffcd6966d38a09023fb37ba9335214c9b959")
get_gh_submodule("freetype",            "aseprite/freetype2",               "9a2d6d97b2d8a5d22d02948b783df12b764afa2d")
get_gh_submodule("glfw",                "Silverlan/glfw",                   "8d6f5d9bc8477057dccc1190d46b0945219e2f33", "feat/drag-callback")
get_gh_submodule("gli",                 "Silverlan/gli",                    "2938076944739166235750f3ccdce37c6cac2d31")
get_gh_submodule("libnoise",            "qknight/libnoise",                 "9ce0737b55812f7de907e86dc633724524e3a8e8")
get_gh_submodule("libsamplerate",       "libsndfile/libsamplerate",         "15c392d")
if platform == "linux":
    get_gh_submodule("linenoise.cpp",   "Silverlan/linenoise.cpp",          "74ae32ebf22d18a7e22afe1eeecf04171311ebcf")
get_gh_submodule("luabind",             "Silverlan/luabind-deboostified",   "38b223d4a8538d08f7ddded7989f4297aa74ee6d")
get_gh_submodule("luajit",              "Silverlan/LuaJIT",                 "cb42653","v2.1")
get_gh_submodule("lunasvg",             "sammycage/lunasvg",                "7b6970f") # v3.4.0
get_gh_submodule("miniball",            "Silverlan/miniball",               "609fbf16e7a9cc3dc8f88e4d1c7a1d8ead842bb1")
get_gh_submodule("mpaheader",           "Silverlan/mpaheader",              "fab7aeeeae4034e42fdb975b580d014514f460d5")
get_gh_submodule("muparserx",           "Silverlan/muparserx",              "910f6128494e174c3de81bd34c3d938b47873c08")
if platform == "win32":
    get_gh_submodule("nvtt",            "castano/nvidia-texture-tools",     "aeddd65f81d36d8cb7b169b469ef25156666077e")
get_gh_submodule("ogg",                 "xiph/ogg",                         "be05b13") # v1.3.6
get_gh_submodule("openfbx",             "nem0/OpenFBX",                     "82a43d9191f2250145fddc219b4083667c33f2a5")
get_gh_submodule("rapidxml",            "discord/rapidxml",                 "2ae4b2888165a393dfb6382168825fddf00c27b9")
get_gh_submodule("recastnavigation",    "recastnavigation/recastnavigation","6dc1667") # v1.6.0
get_gh_submodule("rectangle_bin_pack",  "juj/RectangleBinPack",             "83e7e11")
get_gh_submodule("spdlog",              "Silverlan/spdlog",                 "d1ebfb9","v1.x")
get_gh_submodule("squish",              "ivan-kulikov-dev/squish",          "aeb01b8f88f97c994baff8453870f3e8a347afd7")
get_gh_submodule("tinygltf",            "syoyo/tinygltf",                   "37250b3","release") # v2.9.6
get_gh_submodule("vorbis",              "xiph/vorbis",                      "43bbff0141028e58d476c1d5fd45dd5573db576d")
get_gh_submodule("vkvparser",           "Silverlan/VKVParser",              "380f5527ec57ac7ce540da67a86a23dbfdb593d9")
get_gh_submodule("vtflib",              "Silverlan/VTFLib",                 "3e6b80bcd6dacf6bd4a35666869464fac55051cc")

os.chdir(curDir)
