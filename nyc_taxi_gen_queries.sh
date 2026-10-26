# DATA RANGE:
# VendorID: (1, 6)
# tpep_pickup_datetime: (2002-12-31 16:46:07, 2026-06-26 23:53:12)
# tpep_dropoff_datetime: (2002-12-31 17:24:07, 2026-06-27 20:59:10)
# passenger_count: (0, 9)
# trip_distance: (0.0, 312722.3)
# RatecodeID: (1, 99)
# store_and_fwd_flag: (N, Y)
# PULocationID: (1, 265)
# DOLocationID: (1, 265)
# payment_type: (0, 5)
# fare_amount: (-1285.4, 334076.32)
# extra: (-7.5, 65.99)
# mta_tax: (-0.5, 35.84)
# tip_amount: (-300.0, 999.99)
# tolls_amount: (-117.26, 1702.88)
# improvement_surcharge: (-1.0, 1.0)
# total_amount: (-1315.97, 334145.3)
# congestion_surcharge: (-2.5, 2.52)
# Airport_fee: (-1.75, 1.75)

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