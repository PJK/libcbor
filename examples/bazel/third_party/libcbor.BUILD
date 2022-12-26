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
      "cbor.h",
    ],
    includes = [
      ".", 
      "internal",
    ],
    visibility = ["//visibility:public"],
)

