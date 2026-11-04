create table tbl as select * from 'US_Accidents_March23.csv';
PRAGMA enable_profiling = 'json';
PRAGMA profiling_output = 'profile.json';

SELECT *
FROM tbl
WHERE Zipcode = '60614'
  AND Start_Time BETWEEN '2022-01-01' AND '2022-02-01';

PRAGMA disable_profiling;