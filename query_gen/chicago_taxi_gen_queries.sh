# DATA RANGE:
# COL MIN MAX NDV
# "Trip ID","00000019ef3c0b419a9e6b1843b83530b79627a4","fffffe03acfa1552c98fad12d73ff0aca70a5c2a" (6246699 distinct values)
# "Taxi ID","00110971c7c4a7173fcf93f49a22d6b9b0a02c27c4b9f8a1d4b3d8444b7d49cf690beb04d02d6a5119874a0621e8aff2d76d8ec1df1dd4ad4d674263ca817731","ffda53354c610fd3af1aee46d723028a49014e35f7280ca3d812716eb3cf1f0a763556368e4139b7183ee1fb36e26104de62bb86601f2fa16e3d96167e25b84e" (3396 distinct values)
# "Trip Start Timestamp","2023-01-19 23:30:00","2023-12-31 23:45:00" (33214 distinct values)
# "Trip End Timestamp","2011-09-29 16:45:00","2024-01-02 13:00:00" (33231 distinct values)
# "Trip Seconds","0","999" (15267 distinct values)
# "Trip Miles","0.0","1132.36" (7124 distinct values)
# "Pickup Census Tract","17031010100","17031980100" (649 distinct values)
# "Dropoff Census Tract","17031010100","17031980100" (792 distinct values)
# "Pickup Community Area","1","77" (77 distinct values)
# "Dropoff Community Area","1","77" (77 distinct values)
# "Fare","$0.00","$999.25" (8584 distinct values)
# "Tips","$0.00","$98.88" (3498 distinct values)
# "Tolls","$0.00","$99.99" (490 distinct values)
# "Extras","$0.00","$99.53" (2825 distinct values)
# "Trip Total","$0.00","$999.00" (14187 distinct values)
# "Payment Type","Cash","Unknown" (7 distinct values)
# "Company","2733 - 74600 Benny Jona","U Taxicab" (37 distinct values)
# "Pickup Centroid Latitude","41.650221676","42.021223593" (599 distinct values)
# "Pickup Centroid Longitude","-87.913624596","-87.534902901" (599 distinct values)
# "Pickup Centroid Location","POINT (-87.5349029012 41.707311449)","POINT (-87.913624596 41.9802643146)" (600 distinct values)
# "Dropoff Centroid Latitude","41.660136051","42.021223593" (646 distinct values)
# "Dropoff Centroid Longitude","-87.913624596","-87.534902901" (646 distinct values)
# "Dropoff Centroid  Location","POINT (-87.5349029012 41.707311449)","POINT (-87.913624596 41.9802643146)" (647 distinct values)

count=100
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Trip Start Timestamp\" >= ? AND \"Trip Start Timestamp\" < ?;" $count "Trip Start Timestamp" 0 "Trip Start Timestamp" 0
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Census Tract\" = ?;" $count "Pickup Census Tract" 0
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Dropoff Census Tract\" = ?;" $count "Dropoff Census Tract" 0
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Census Tract\" = ? AND \"Dropoff Census Tract\" = ?;" $count "Pickup Census Tract" 0 "Dropoff Census Tract" 1
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Community Area\" = ?;" $count "Pickup Community Area" 0
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Dropoff Community Area\" = ?;" $count "Dropoff Community Area" 0
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Community Area\" = ? AND \"Dropoff Community Area\" = ?;" $count "Pickup Community Area" 0 "Dropoff Community Area" 1
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Company\" = ?;" $count "Company" 0
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Payment Type\" = ?;" $count "Payment Type" 0
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Company\" = ? AND \"Payment Type\" = ?;" $count "Company" 0 "Payment Type" 1
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Taxi ID\" = ?;" $count "Taxi ID" 0
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Taxi ID\" = ? AND \"Trip Start Timestamp\" BETWEEN ? AND ?;" $count "Taxi ID" 0 "Trip Start Timestamp" 1 "Trip Start Timestamp" 1
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Census Tract\" = ? AND \"Trip Start Timestamp\" BETWEEN ? AND ?;" $count "Pickup Census Tract" 0 "Trip Start Timestamp" 1 "Trip Start Timestamp" 1
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Dropoff Census Tract\" = ? AND \"Trip Start Timestamp\" BETWEEN ? AND ?;" $count "Dropoff Census Tract" 0 "Trip Start Timestamp" 1 "Trip Start Timestamp" 1
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Company\" = ? AND \"Trip Start Timestamp\" BETWEEN ? AND ?;" $count "Company" 0 "Trip Start Timestamp" 1 "Trip Start Timestamp" 1
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Census Tract\" = ? AND \"Company\" = ?;" $count "Pickup Census Tract" 0 "Company" 1
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Census Tract\" = ? AND \"Company\" = ? AND \"Trip Start Timestamp\" BETWEEN ? AND ?;" $count "Pickup Census Tract" 0 "Company" 1 "Trip Start Timestamp" 2 "Trip Start Timestamp" 2
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Census Tract\" = ? AND \"Dropoff Census Tract\" = ? AND \"Trip Start Timestamp\" BETWEEN ? AND ?;" $count "Pickup Census Tract" 0 "Dropoff Census Tract" 1 "Trip Start Timestamp" 2 "Trip Start Timestamp" 2
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Pickup Census Tract\" = ? AND \"Dropoff Census Tract\" = ? AND \"Company\" = ?;" $count "Pickup Census Tract" 0 "Dropoff Census Tract" 1 "Company" 2
python3 query_gen/query_gen.py "chicago_taxi/chicago_taxi.csv" "SELECT COUNT(*) FROM chicago_taxi WHERE \"Taxi ID\" = ? AND \"Pickup Census Tract\" = ? AND \"Dropoff Census Tract\" = ?;" $count "Taxi ID" 0 "Pickup Census Tract" 1 "Dropoff Census Tract" 2