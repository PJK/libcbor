#genrule(
#  name = "cbor_cmake",
#  srcs = glob(["**"]),
#  outs = ["libcbor.a", "cbor.h", "cbor/arrays.h", "cbor/bytestrings.h",
#          "cbor/callbacks.h", "cbor/cbor_export.h", "cbor/common.h", "cbor/configuration.h", "cbor/data.h",
#          "cbor/encoding.h", "cbor/floats_ctrls.h", "cbor/ints.h", "cbor/maps.h",
#          "cbor/serialization.h", "cbor/streaming.h", "cbor/strings.h", "cbor/tags.h"],
#  cmd = " && ".join([
#    # Remember where output should go.
#    "INITIAL_WD=`pwd`",
#    # Build libcbor library.
#    "cd `dirname $(location CMakeLists.txt)`",
#    "cmake -DCMAKE_BUILD_TYPE=Release -DCBOR_CUSTOM_ALLOC=ON .",
#    "cmake --build .",
#    # Export the .a and .h files for cbor rule, below.
#    "cp src/libcbor.a src/cbor.h $$INITIAL_WD/$(RULEDIR)",
#    "cp src/cbor/*h cbor/configuration.h $$INITIAL_WD/$(RULEDIR)/cbor"]),
#  visibility = ["//visibility:private"],
#)
#
#cc_import(
#  name = "cbor",
#  hdrs = ["cbor.h", "cbor/arrays.h", "cbor/bytestrings.h",
#          "cbor/callbacks.h", "cbor/cbor_export.h", "cbor/common.h", "cbor/configuration.h", "cbor/data.h",
#          "cbor/encoding.h", "cbor/floats_ctrls.h", "cbor/ints.h", "cbor/maps.h",
#          "cbor/serialization.h", "cbor/streaming.h", "cbor/strings.h", "cbor/tags.h"],
#  static_library = "libcbor.a",
#  visibility = ["//visibility:public"],
#)

cc_library(
    name = "cbor",
    srcs = glob([
      "src/**/*.h",
      "src/**/*.c",

    ]),
    deps = [
      "@libcbor_bazel_example//third_party/libcbor:config",
    ],
    hdrs = [
      "src/cbor.h",
    ],
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
)

