# "VendorID", (3 distinct values)
# "tpep_pickup_datetime", (9955675 distinct values)
# "tpep_dropoff_datetime", (9948395 distinct values)
# "passenger_count", (10 distinct values)
# "trip_distance", (7056 distinct values)
# "RatecodeID", (7 distinct values)
# "store_and_fwd_flag", (2 distinct values)
# "PULocationID", (262 distinct values)
# "DOLocationID", (261 distinct values)
# "payment_type", (6 distinct values)
# "fare_amount", (14681 distinct values)
# "extra", (116 distinct values)
# "mta_tax", (20 distinct values)
# "tip_amount", (6638 distinct values)
# "tolls_amount", (2831 distinct values)
# "improvement_surcharge", (5 distinct values)
# "total_amount", (33401 distinct values)
# "congestion_surcharge", (8 distinct values)
# "Airport_fee", (4 distinct values)

count=100
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE trip_distance BETWEEN ? AND ?;" $count "float" 0.0 312722.3 0 "float" 0.0 312722.3 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE total_amount BETWEEN ? AND ?;" $count "float" -1315.97 334145.3 0 "float" -1315.97 334145.3 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE passenger_count = ?;" $count "int" 0 9 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE PULocationID = ?;" $count "int" 1 265 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE tpep_pickup_datetime BETWEEN ? AND ? AND fare_amount BETWEEN ? AND ?;"  $count "time" "2002-12-31 16:46:07" "2026-06-26 23:53:12" 0 "time" "2002-12-31 16:46:07" "2026-06-26 23:53:12" 0 "float" -1315.97 334145.3 1 "float" -1315.97 334145.3 1 
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE tpep_dropoff_datetime BETWEEN ? AND ? AND trip_distance > ?;"  $count "time" "2002-12-31 17:24:07" "2026-06-27 20:59:10" 0 "time" "2002-12-31 17:24:07" "2026-06-27 20:59:10" 0 "float" 0.0 312722.3 1
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE fare_amount > ?;" $count "float" -1315.97 334145.3 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE trip_distance > ?;" $count "float" 0.0 312722.3 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE PULocationID IN (?, ?, ?);" $count "int" 1 265 0 "int" 1 265 0 "int" 1 265 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE fare_amount = ?;"  $count "float" -1315.97 334145.3 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE passenger_count = ?;" $count "int" 0 9 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE PULocationID = ?;" $count "int" 1 265 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE RatecodeID = ?;" $count "int" 1 99 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE trip_distance = ?;" $count "float" 0 21.4 0
python3 nyc_taxi_query_gen.py "SELECT COUNT(*) FROM trips WHERE tpep_pickup_datetime BETWEEN ? AND ? AND PULocationID = ?;" $count "time" "2002-12-31 16:46:07" "2026-06-26 23:53:12" 0 "time" "2002-12-31 16:46:07" "2026-06-26 23:53:12" 0 "int" 1 265 1