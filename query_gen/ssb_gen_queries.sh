# DATA RANGE:
# COL MIN MAX NDV
# "lo_orderkey","1","6000000" (1500000 distinct values)
# "lo_linenumber","1","7" (7 distinct values)
# "lo_custkey","1","29999" (20000 distinct values)
# "lo_partkey","1","200000" (200000 distinct values)
# "lo_suppkey","1","2000" (2000 distinct values)
# "lo_orderdate","1992-01-01","1998-08-02" (2406 distinct values)
# "lo_orderpriority","1-URGENT","5-LOW" (5 distinct values)
# "lo_shippriority","0","0" (1 distinct values)
# "lo_quantity","1","50" (50 distinct values)
# "lo_extendedprice","90100","10494950" (933900 distinct values)
# "lo_ordertotalprice","85771","55528516" (1464556 distinct values)
# "lo_discount","0","10" (11 distinct values)
# "lo_revenue","81360","10464950" (3345301 distinct values)
# "lo_supplycost","54060","125939" (12939 distinct values)
# "lo_tax","0","8" (9 distinct values)
# "lo_commitdate","1992-01-31","1998-10-31" (2466 distinct values)
# "lo_shipmode","AIR","TRUCK" (7 distinct values)
# "c_custkey","1","29999" (20000 distinct values)
# "c_name","Customer#000000001","Customer#000029999" (20000 distinct values)
# "c_address","  3SUEwTaEs35Z","zzIPP0HlCZtPnLc99i," (20000 distinct values)
# "c_city","ALGERIA  0","VIETNAM  9" (250 distinct values)
# "c_nation","ALGERIA","VIETNAM" (25 distinct values)
# "c_region","AFRICA","MIDDLE EAST" (5 distinct values)
# "c_phone","10-100-991-5371","34-999-209-9805" (20000 distinct values)
# "c_mktsegment","AUTOMOBILE","MACHINERY" (5 distinct values)
# "s_suppkey","1","2000" (2000 distinct values)
# "s_name","Supplier#000000001","Supplier#000002000" (2000 distinct values)
# "s_address","  Bg4GNb1 64E,NhHO","zw9CeCD4NaTK" (2000 distinct values)
# "s_city","ALGERIA  0","VIETNAM  9" (250 distinct values)
# "s_nation","ALGERIA","VIETNAM" (25 distinct values)
# "s_region","AFRICA","MIDDLE EAST" (5 distinct values)
# "s_phone","10-127-851-8031","34-985-422-6009" (2000 distinct values)
# "p_partkey","1","200000" (200000 distinct values)
# "p_name","almond antique","yellow white" (8372 distinct values)
# "p_mfgr","MFGR#1","MFGR#5" (5 distinct values)
# "p_category","MFGR#11","MFGR#55" (25 distinct values)
# "p_brand","MFGR#111","MFGR#559" (1000 distinct values)
# "p_color","almond","yellow" (92 distinct values)
# "p_type","ECONOMY ANODIZED BRASS","STANDARD POLISHED TIN" (150 distinct values)
# "p_size","1","50" (50 distinct values)
# "p_container","JUMBO BAG","WRAP PKG" (40 distinct values)
# "d_datekey","19920101","19980802" (2406 distinct values)
# "d_date","April 1, 1992","September 9, 1997" (2406 distinct values)
# "d_dayofweek","Friday","Wednesday" (7 distinct values)
# "d_month","April","September" (12 distinct values)
# "d_year","1992","1998" (7 distinct values)
# "d_yearmonthnum","199201","199808" (80 distinct values)
# "d_yearmonth","Apr1992","Sep1997" (80 distinct values)
# "d_daynuminweek","1","7" (7 distinct values)
# "d_daynuminmonth","1","31" (31 distinct values)
# "d_daynuminyear","1","366" (366 distinct values)
# "d_monthnuminyear","1","12" (12 distinct values)
# "d_weeknuminyear","1","53" (53 distinct values)
# "d_sellingseason","Christmas","Winter" (5 distinct values)
# "d_lastdayinweekfl","0","1" (2 distinct values)
# "d_lastdayinmonthfl","0","1" (2 distinct values)
# "d_holidayfl","0","1" (2 distinct values)
# "d_weekdayfl","0","1" (2 distinct values)

count=100
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_extendedprice * lo_discount) FROM ssb_flat WHERE d_year = ? AND lo_discount BETWEEN ? AND ? AND lo_quantity < ?;" $count "d_year" 0 "lo_discount" 1 "lo_discount" 1 "lo_quantity" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_extendedprice * lo_discount) FROM ssb_flat WHERE d_yearmonthnum = ? AND lo_discount BETWEEN ? AND ? AND lo_quantity BETWEEN ? AND ?;" $count "d_yearmonthnum" 0 "lo_discount" 1 "lo_discount" 1 "lo_quantity" 2 "lo_quantity" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_extendedprice * lo_discount) FROM ssb_flat WHERE d_weeknuminyear = ? AND d_year = ? AND lo_discount BETWEEN ? AND ? AND lo_quantity BETWEEN ? AND ?;" $count "d_weeknuminyear" 0 "d_year" 1 "lo_discount" 2 "lo_discount" 2 "lo_quantity" 3 "lo_quantity" 3
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue) FROM ssb_flat WHERE p_category = ? AND s_region = ?;" $count "p_category" 0 "s_region" 1
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue) FROM ssb_flat WHERE p_brand = ? AND s_region = ?;" $count "p_brand" 0 "s_region" 1
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue) FROM ssb_flat WHERE p_brand = ? AND s_region = ? AND d_year = ?;" $count "p_brand" 0 "s_region" 1 "d_year" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue) FROM ssb_flat WHERE c_region = ? AND s_region = ? AND d_year BETWEEN ? AND ?;" $count "c_region" 0 "s_region" 1 "d_year" 2 "d_year" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue) FROM ssb_flat WHERE c_nation = ? AND s_nation = ? AND d_year BETWEEN ? AND ?;" $count "c_nation" 0 "s_nation" 1 "d_year" 2 "d_year" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue) FROM ssb_flat WHERE c_city = ? AND s_city = ? AND d_year BETWEEN ? AND ?;" $count "c_city" 0 "s_city" 1 "d_year" 2 "d_year" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue) FROM ssb_flat WHERE c_city = ? AND s_city = ? AND d_yearmonth = ?;" $count "c_city" 0 "s_city" 1 "d_yearmonth" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue - lo_supplycost) FROM ssb_flat WHERE c_region = ? AND s_region = ? AND p_mfgr IN (?, ?) AND d_year = ?;" $count "c_region" 0 "s_region" 1 "p_mfgr" 2 "p_mfgr" 3 "d_year" 4
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue - lo_supplycost) FROM ssb_flat WHERE c_nation = ? AND s_nation = ? AND p_category = ? AND d_year BETWEEN ? AND ?;" $count "c_nation" 0 "s_nation" 1 "p_category" 2 "d_year" 3 "d_year" 3
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT SUM(lo_revenue - lo_supplycost) FROM ssb_flat WHERE s_city = ? AND p_brand = ? AND d_year = ?;" $count "s_city" 0 "p_brand" 1 "d_year" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE p_brand = ?;" $count "p_brand" 0
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE c_city = ?;" $count "c_city" 0
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE s_city = ?;" $count "s_city" 0
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE c_city = ? AND s_city = ?;" $count "c_city" 0 "s_city" 1
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE p_brand = ? AND s_region = ?;" $count "p_brand" 0 "s_region" 1
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE p_brand = ? AND c_city = ?;" $count "p_brand" 0 "c_city" 1
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE p_brand = ? AND c_city = ? AND d_year = ?;" $count "p_brand" 0 "c_city" 1 "d_year" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE p_category = ? AND s_region = ? AND d_year = ?;" $count "p_category" 0 "s_region" 1 "d_year" 2
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE lo_quantity BETWEEN ? AND ?;" $count "lo_quantity" 0 "lo_quantity" 0
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE lo_discount BETWEEN ? AND ?;" $count "lo_discount" 0 "lo_discount" 0
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE lo_quantity BETWEEN ? AND ? AND p_brand = ?;" $count "lo_quantity" 0 "lo_quantity" 0 "p_brand" 1
python3 query_gen/query_gen.py "ssb_flat/ssb_flat.csv" "SELECT COUNT(*) FROM ssb_flat WHERE lo_discount BETWEEN ? AND ? AND s_region = ?;" $count "lo_discount" 0 "lo_discount" 0 "s_region" 1