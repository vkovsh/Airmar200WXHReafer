import qbs

//Library {
//    type: project.libraryType
CppApplication {
//    Depends { name: "BuildConfig" }
    Depends { name: "Qt.core" }

//    Group {
//        qbs.install: true
//        fileTagsFilter: product.type
//    }

    Depends { name: "cpp" }

    cpp.includePaths: ['./..']

    files: [
        "**/*.h",
        "**/*.cpp",
        "**/*.rem",
    ]
}
