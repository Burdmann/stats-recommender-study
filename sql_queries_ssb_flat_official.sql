SET threads TO 1;
ATTACH 'test.db' (ROW_GROUP_SIZE 8192);
USE test;
CREATE TABLE denormalized(
    lo_orderkey        BIGINT,
    lo_linenumber      BIGINT,
    lo_custkey         BIGINT,
    lo_partkey         BIGINT,
    lo_suppkey         BIGINT,
    lo_orderdate       DATE,
    lo_orderpriority   VARCHAR,
    lo_shippriority    BIGINT,
    lo_quantity        BIGINT,
    lo_extendedprice   BIGINT,
    lo_ordertotalprice BIGINT,
    lo_discount        BIGINT,
    lo_revenue         BIGINT,
    lo_supplycost      BIGINT,
    lo_tax             BIGINT,
    lo_commitdate      DATE,
    lo_shipmode        VARCHAR,
    c_custkey          BIGINT,
    c_name             VARCHAR,
    c_address          VARCHAR,
    c_city             VARCHAR,
    c_nation           VARCHAR,
    c_region           VARCHAR,
    c_phone            VARCHAR,
    c_mktsegment       VARCHAR,
    s_suppkey          BIGINT,
    s_name             VARCHAR,
    s_address          VARCHAR,
    s_city             VARCHAR,
    s_nation           VARCHAR,
    s_region           VARCHAR,
    s_phone            VARCHAR,
    p_partkey          BIGINT,
    p_name             VARCHAR,
    p_mfgr             VARCHAR,
    p_category         VARCHAR,
    p_brand            VARCHAR,
    p_color            VARCHAR,
    p_type             VARCHAR,
    p_size             BIGINT,
    p_container        VARCHAR,
    d_datekey          BIGINT,
    d_date             VARCHAR,
    d_dayofweek        VARCHAR,
    d_month            VARCHAR,
    d_year             BIGINT,
    d_yearmonthnum     BIGINT,
    d_yearmonth        VARCHAR,
    d_daynuminweek     BIGINT,
    d_daynuminmonth    BIGINT,
    d_daynuminyear     BIGINT,
    d_monthnuminyear   BIGINT,
    d_weeknuminyear    BIGINT,
    d_sellingseason    VARCHAR,
    d_lastdayinweekfl  BIGINT,
    d_lastdayinmonthfl BIGINT,
    d_holidayfl        BIGINT,
    d_weekdayfl        BIGINT
);;

COPY denormalized FROM 'ssb_flat/ssb_flat.csv' (FORMAT 'csv', delimiter ',', header 1);

-- Query 1.1
select sum(lo_extendedprice*lo_discount) as revenue from denormalized where d_year = 1993 and lo_discount between 1 and 3 and lo_quantity < 25;
-- Query 1.2
select sum(lo_extendedprice*lo_discount) as lo_revenue from denormalized where d_yearmonthnum = 199401 and lo_discount between 4 and 6 and lo_quantity between 26 and 35;
-- Query 1.3
select sum(lo_extendedprice*lo_discount) as lo_revenue from denormalized where d_weeknuminyear = 6 and d_year = 1994 and lo_discount between 5 and 7 and lo_quantity between 26 and 35;
-- Query 2.1
select sum(lo_revenue), d_year, p_brand from denormalized where p_category = 'MFGR#12' and s_region = 'AMERICA' group by d_year, p_brand order by d_year, p_brand;
-- Query 2.2
select sum(lo_revenue), d_year, p_brand from denormalized where p_brand between 'MFGR#2221' and 'MFGR#2228' and s_region = 'ASIA' group by d_year, p_brand order by d_year, p_brand;
-- Query 2.3
select sum(lo_revenue), d_year, p_brand from denormalized where p_brand= 'MFGR#2239' and s_region = 'EUROPE' group by d_year, p_brand order by d_year, p_brand;
-- Query 3.1
select c_nation, s_nation, d_year, sum(lo_revenue) as lo_revenue from denormalized where c_region = 'ASIA' and s_region = 'ASIA' and d_year >= 1992 and d_year <= 1997 group by c_nation, s_nation, d_year order by d_year asc, lo_revenue desc;
-- Query 3.2
select c_city, s_city, d_year, sum(lo_revenue) as lo_revenue from denormalized where c_nation = 'UNITED STATES' and s_nation = 'UNITED STATES' and d_year >= 1992 and d_year <= 1997 group by c_city, s_city, d_year order by d_year asc, lo_revenue desc;
-- Query 3.3
select c_city, s_city, d_year, sum(lo_revenue) as lo_revenue from denormalized where (c_city='UNITED KI1' or c_city='UNITED KI5') and (s_city='UNITED KI1' or s_city='UNITED KI5') and d_year >= 1992 and d_year <= 1997 group by c_city, s_city, d_year order by d_year asc, lo_revenue desc;
-- Query 3.4
select c_city, s_city, d_year, sum(lo_revenue) as lo_revenue from denormalized where (c_city='UNITED KI1' or c_city='UNITED KI5') and (s_city='UNITED KI1' or s_city='UNITED KI5') and d_yearmonth = 'Dec1997' group by c_city, s_city, d_year order by d_year asc, lo_revenue desc;
-- Query 4.1
select d_year, c_nation, sum(lo_revenue - lo_supplycost) as profit from denormalized where c_region = 'AMERICA' and s_region = 'AMERICA' and (p_mfgr = 'MFGR#1' or p_mfgr = 'MFGR#2') group by d_year, c_nation order by d_year, c_nation;
-- Query 4.2
select d_year, s_nation, p_category, sum(lo_revenue - lo_supplycost) as profit from denormalized where c_region = 'AMERICA' and s_region = 'AMERICA' and (d_year = 1997 or d_year = 1998) and (p_mfgr = 'MFGR#1' or p_mfgr = 'MFGR#2') group by d_year, s_nation, p_category order by d_year, s_nation, p_category;
-- Query 4.3
select d_year, s_city, p_brand, sum(lo_revenue - lo_supplycost) as profit from denormalized where c_region = 'AMERICA' and s_nation = 'UNITED STATES' and (d_year = 1997 or d_year = 1998) and p_category = 'MFGR#14' group by d_year, s_city, p_brand order by d_year, s_city, p_brand;

DROP TABLE denormalized;