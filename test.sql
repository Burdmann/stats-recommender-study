ATTACH 'test.db' (ROW_GROUP_SIZE 8192);
USE test;
create table tbl as select * from 'US_Accidents_March23.csv';
PRAGMA enable_profiling = 'json';
PRAGMA profiling_output = 'profile.json';

SELECT *
FROM tbl
WHERE Zipcode = '60614';

PRAGMA disable_profiling;
DROP TABLE tbl;