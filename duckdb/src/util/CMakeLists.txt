add_library_unity(
  duckdb_util
  OBJECT
  util.cpp)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_util>
    PARENT_SCOPE)
