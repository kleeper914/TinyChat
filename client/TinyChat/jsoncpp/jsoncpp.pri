INCLUDEPATH+=$$PWD/json \
            $$PWD/lib_json \
DEPENDPATH+=$$PWD/lib_json
HEADERS += \
        $$PWD/json/json.h \
        $$PWD/json/allocator.h \
        $$PWD/json/assertions.h \
        $$PWD/json/config.h \
        $$PWD/json/forwards.h \
        $$PWD/json/json_features.h \
        $$PWD/json/reader.h \
        $$PWD/json/value.h \
        $$PWD/json/version.h \
        $$PWD/json/writer.h \
        $$PWD/lib_json/json_tool.h \

SOURCES += $$PWD/lib_json/json_reader.cpp \
        $$PWD/lib_json/json_value.cpp \
        $$PWD/lib_json/json_writer.cpp \
