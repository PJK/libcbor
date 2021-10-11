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

