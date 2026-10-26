CREATE TABLE customer (
    c_custkey       INTEGER,
    c_name          VARCHAR,
    c_address       VARCHAR,
    c_city          VARCHAR,
    c_nation        VARCHAR,
    c_region        VARCHAR,
    c_phone         VARCHAR,
    c_mktsegment    VARCHAR
);

CREATE TABLE supplier (
    s_suppkey       INTEGER,
    s_name          VARCHAR,
    s_address       VARCHAR,
    s_city          VARCHAR,
    s_nation        VARCHAR,
    s_region        VARCHAR,
    s_phone         VARCHAR
);

CREATE TABLE part (
    p_partkey       INTEGER,
    p_name          VARCHAR,
    p_mfgr          VARCHAR,
    p_category      VARCHAR,
    p_brand         VARCHAR,
    p_color         VARCHAR,
    p_type          VARCHAR,
    p_size          INTEGER,
    p_container     VARCHAR
);

CREATE TABLE dwdate (
    d_datekey              INTEGER,
    d_date                 VARCHAR,
    d_dayofweek            VARCHAR,
    d_month                VARCHAR,
    d_year                 INTEGER,
    d_yearmonthnum         INTEGER,
    d_yearmonth            VARCHAR,
    d_daynuminweek         INTEGER,
    d_daynuminmonth        INTEGER,
    d_daynuminyear         INTEGER,
    d_monthnuminyear       INTEGER,
    d_weeknuminyear        INTEGER,
    d_sellingseason        VARCHAR,
    d_lastdayinweekfl      INTEGER,
    d_lastdayinmonthfl     INTEGER,
    d_holidayfl            INTEGER,
    d_weekdayfl            INTEGER
);

CREATE TABLE lineorder (
    lo_orderkey            INTEGER,
    lo_linenumber          INTEGER,
    lo_custkey             INTEGER,
    lo_partkey             INTEGER,
    lo_suppkey             INTEGER,
    lo_orderdate           DATE,
    lo_orderpriority       VARCHAR,
    lo_shippriority        INTEGER,
    lo_quantity            INTEGER,
    lo_extendedprice       INTEGER,
    lo_ordertotalprice     INTEGER,
    lo_discount            INTEGER,
    lo_revenue             INTEGER,
    lo_supplycost          INTEGER,
    lo_tax                 INTEGER,
    lo_commitdate          DATE,
    lo_shipmode            VARCHAR
);

INSERT INTO customer
SELECT *
FROM read_csv(
    './ssb-dbgen/customer.tbl',
    delim=',',
    ignore_errors=false,
    null_padding=true
);

INSERT INTO supplier
SELECT *
FROM read_csv(
    'ssb-dbgen/supplier.tbl',
    delim=',',
    ignore_errors=false,
    null_padding=true
);

INSERT INTO part
SELECT *
FROM read_csv(
    'ssb-dbgen/part.tbl',
    delim=',',
    ignore_errors=false,
    null_padding=true
);

INSERT INTO dwdate
SELECT *
FROM read_csv(
    'ssb-dbgen/date.tbl',
    delim=',',
    ignore_errors=false,
    null_padding=true
);

INSERT INTO lineorder
SELECT *
FROM read_csv(
    'ssb-dbgen/lineorder.tbl',
    delim=',',
    ignore_errors=false,
    null_padding=true
);


CREATE TABLE flat AS
SELECT *
FROM lineorder lo
JOIN customer c ON lo.lo_custkey = c.c_custkey
JOIN supplier s ON lo.lo_suppkey = s.s_suppkey
JOIN part p ON lo.lo_partkey = p.p_partkey
JOIN dwdate d
  ON lo.lo_orderdate = strptime(CAST(d.d_datekey AS VARCHAR), '%Y%m%d');

COPY flat TO 'ssb_flat.csv';