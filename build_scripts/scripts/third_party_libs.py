import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

third_party_libs_dir = curDir +"/third_party_libs"
os.chdir(third_party_libs_dir)
get_submodule("7zpp","https://github.com/Silverlan/7zip-cpp.git","df81447")
get_submodule("bvh","https://github.com/madmann91/bvh.git","ac41ab8")
get_submodule("bzip2","https://github.com/Silverlan/bzip2.git","6f08ffc2409d34a8f0f06ab3b0291bced4e9d08c")
get_submodule("clip2tri","https://github.com/raptor/clip2tri.git","f62a734d22733814b8a970ed8a68a4d94c24fa5f")
get_submodule("efsw","https://github.com/SpartanJ/efsw.git","1bb814c")
get_submodule("eigen","https://gitlab.com/libeigen/eigen.git","b66188b5") # v3.4.1
get_submodule("exprtk","https://github.com/ArashPartow/exprtk.git","f46bffcd6966d38a09023fb37ba9335214c9b959")
get_submodule("freetype","https://github.com/aseprite/freetype2.git","9a2d6d97b2d8a5d22d02948b783df12b764afa2d")
get_submodule("glfw","https://github.com/Silverlan/glfw.git","8d6f5d9bc8477057dccc1190d46b0945219e2f33", "feat/drag-callback")
get_submodule("gli","https://github.com/Silverlan/gli.git","54351c86ea95e35b2f506ab33da57381d5c4828f", "feat/cxx_module")
get_submodule("libnoise","https://github.com/qknight/libnoise.git","9ce0737b55812f7de907e86dc633724524e3a8e8")
get_submodule("libsamplerate","https://github.com/libsndfile/libsamplerate.git","15c392d")
if platform == "linux":
    get_submodule("linenoise.cpp","https://github.com/Silverlan/linenoise.cpp.git","74ae32ebf22d18a7e22afe1eeecf04171311ebcf")
get_submodule("luabind","https://github.com/Silverlan/luabind-deboostified.git","28823c6be1ad2169e42bd3bad2c01ce450d35dc9", "feat/cxx_module")
get_submodule("luajit","https://github.com/Silverlan/LuaJIT.git","cb42653","v2.1")
get_submodule("lunasvg","https://github.com/sammycage/lunasvg.git","7b6970f") # v3.4.0
get_submodule("miniball","https://github.com/Silverlan/miniball.git","609fbf16e7a9cc3dc8f88e4d1c7a1d8ead842bb1")
get_submodule("mpaheader","https://github.com/Silverlan/mpaheader.git","fab7aeeeae4034e42fdb975b580d014514f460d5")
get_submodule("muparserx","https://github.com/Silverlan/muparserx.git","910f6128494e174c3de81bd34c3d938b47873c08")
if platform == "win32":
    get_submodule("nvtt","https://github.com/castano/nvidia-texture-tools.git","aeddd65f81d36d8cb7b169b469ef25156666077e")
get_submodule("ogg","https://github.com/xiph/ogg.git","be05b13") # v1.3.6
get_submodule("openfbx","https://github.com/nem0/OpenFBX.git","82a43d9191f2250145fddc219b4083667c33f2a5")
get_submodule("rapidxml","https://github.com/discord/rapidxml.git","2ae4b2888165a393dfb6382168825fddf00c27b9")
get_submodule("recastnavigation","https://github.com/recastnavigation/recastnavigation.git","6dc1667") # v1.6.0
get_submodule("rectangle_bin_pack","https://github.com/juj/RectangleBinPack.git","83e7e11")
get_submodule("spdlog","https://github.com/Silverlan/spdlog.git","d1ebfb9","v1.x")
get_submodule("squish","https://github.com/ivan-kulikov-dev/squish.git","aeb01b8f88f97c994baff8453870f3e8a347afd7")
get_submodule("tinygltf","https://github.com/syoyo/tinygltf.git","37250b3","release") # v2.9.6
get_submodule("vorbis","https://github.com/xiph/vorbis.git","43bbff0141028e58d476c1d5fd45dd5573db576d")
get_submodule("vkvparser","https://github.com/Silverlan/VKVParser.git","5ffec9da2fc51288d0d77a99fcf173d6e9b91cdb")
get_submodule("vtflib","https://github.com/Silverlan/VTFLib.git","3e6b80bcd6dacf6bd4a35666869464fac55051cc")

os.chdir(curDir)
