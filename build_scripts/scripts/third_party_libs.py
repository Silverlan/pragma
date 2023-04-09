import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

third_party_libs_dir = curDir +"/third_party_libs"
os.chdir(third_party_libs_dir)
get_submodule("7zpp","https://github.com/Silverlan/7zip-cpp.git")
get_submodule("bvh","https://github.com/madmann91/bvh.git","2fd0db62022993963a7343669275647cb073e19a","v1")
get_submodule("bzip2","https://github.com/sergiud/bzip2.git","c4a14bb87ee395fb2c69ef5dbb50762fe862517e","cmake")
get_submodule("clip2tri","https://github.com/raptor/clip2tri.git","f62a734d22733814b8a970ed8a68a4d94c24fa5f")
get_submodule("cppbepuik","https://github.com/Silverlan/cppbepuik.git","b5ac10cb57dab8d8d1e012a6cef31faf8eb6df10")
get_submodule("eigen","https://gitlab.com/libeigen/eigen.git","12e8d57108c50d8a63605c6eb0144c838c128337")
get_submodule("exprtk","https://github.com/ArashPartow/exprtk.git","f46bffcd6966d38a09023fb37ba9335214c9b959")
get_submodule("freetype","https://github.com/aseprite/freetype2.git","fbbcf50367403a6316a013b51690071198962920")
get_submodule("glfw","https://github.com/glfw/glfw.git","814b7929c5add4b0541ccad26fb81f28b71dc4d8","3.3-stable")
get_submodule("gli","https://github.com/g-truc/gli.git","dd17acf9cc7fc6e6abe9f9ec69949eeeee1ccd82")
get_submodule("glm","https://github.com/g-truc/glm.git","7590260cf81f3e49f492e992f60dd88cd3265d14")
get_submodule("libnoise","https://github.com/qknight/libnoise.git","2fb16f638aac6868d550c735898f217cdefa3559")
get_submodule("libpng","https://github.com/glennrp/libpng.git","5a0b7e9c29ec23f87c601622cb2db01781a6cbba","libpng16")
get_submodule("libsamplerate","https://github.com/libsndfile/libsamplerate.git","a2eb2814150a4ada0b49ea3cc5e51170572fc606")
get_submodule("libzip","https://github.com/nih-at/libzip.git","26ba5523db09213f532821875542dba7afa04b65")
get_submodule("luabind","https://github.com/Silverlan/luabind-deboostified.git","a93762a3b58b8d34b1e252e62fe21539f8b8061f")
get_submodule("luajit","https://github.com/Silverlan/LuaJIT.git","355d87d570cf424e46756af4a1871af134fdfc76","v2.1")
get_submodule("miniball","https://github.com/Silverlan/miniball.git","609fbf16e7a9cc3dc8f88e4d1c7a1d8ead842bb1")
get_submodule("muparserx","https://github.com/Silverlan/muparserx.git","1d4656bd671ab856a79f767c483a78c0a036918b")
get_submodule("ogg","https://github.com/xiph/ogg.git","db5c7a49ce7ebda47b15b78471e78fb7f2483e22")
get_submodule("rapidxml","https://github.com/discord/rapidxml.git","2ae4b2888165a393dfb6382168825fddf00c27b9")
get_submodule("recastnavigation","https://github.com/recastnavigation/recastnavigation.git","14b2631527c4792e95b2c78ebfa8ac4cd3413363")
get_submodule("rectangle_bin_pack","https://github.com/juj/RectangleBinPack.git","f159bbeb55e54b510cb12b1011c53dadbf35ed8a")
get_submodule("spdlog","https://github.com/Silverlan/spdlog.git","31b93be2e1829ff945f03b1754ea9ad1f32794be","v1.x")
get_submodule("squish","https://github.com/ivan-kulikov-dev/squish.git","aeb01b8f88f97c994baff8453870f3e8a347afd7")
get_submodule("tinygltf","https://github.com/syoyo/tinygltf.git","967c98dd90406cb9d25df0d95909deae7771e997","release")
get_submodule("vorbis","https://github.com/xiph/vorbis.git","9eadeccdc4247127d91ac70555074239f5ce3529")
get_submodule("vtflib","https://github.com/Silverlan/VTFLib.git","3e6b80bcd6dacf6bd4a35666869464fac55051cc")
get_submodule("zlib","https://github.com/Silverlan/zlib.git","0dcc7471231d72376d592f80f06c2e2246c2b3c4")

os.chdir(curDir)
