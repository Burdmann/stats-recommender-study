SET threads TO 1;
ATTACH 'test.db' (ROW_GROUP_SIZE 8192);
USE test;
CREATE TABLE trips(VendorID  BIGINT,
                                    tpep_pickup_datetime TIMESTAMP,
                                    tpep_dropoff_datetime TIMESTAMP,
                                    passenger_count BIGINT,
                                    trip_distance DOUBLE,
                                    RatecodeID BIGINT,
                                    store_and_fwd_flag VARCHAR,
                                    PULocationID BIGINT,
                                    DOLocationID BIGINT,
                                    payment_type BIGINT,
                                    fare_amount DOUBLE,
                                    extra DOUBLE,
                                    mta_tax DOUBLE,
                                    tip_amount DOUBLE,
                                    tolls_amount DOUBLE,
                                    improvement_surcharge DOUBLE,
                                    total_amount DOUBLE,
                                    congestion_surcharge DOUBLE,
                                    Airport_fee DOUBLE);;
COPY trips FROM 'yeallow_2024/yellow_tripdata_2024.csv' (FORMAT 'csv', delimiter ',', header 1);
SELECT COUNT(*) FROM trips WHERE trip_distance BETWEEN ? AND ?;
SELECT COUNT(*) FROM trips WHERE total_amount BETWEEN ? AND ?;
SELECT COUNT(*) FROM trips WHERE passenger_count = ?;
SELECT COUNT(*) FROM trips WHERE pickup_location_id = ?;
SELECT COUNT(*) FROM trips WHERE pickup_datetime BETWEEN ? AND ? AND fare_amount BETWEEN ? AND ?;
SELECT COUNT(*) FROM trips WHERE pickup_datetime BETWEEN ? AND ? AND trip_distance > ?;
SELECT * FROM trips WHERE fare_amount > 200;
SELECT * FROM trips WHERE trip_distance > 50;
SELECT * FROM trips WHERE passenger_count >= 6;
SELECT COUNT(*) FROM trips WHERE pickup_location_id IN (?, ?, ?);
SELECT COUNT(*) FROM trips WHERE fare_amount = ?;
SELECT COUNT(*) FROM trips WHERE passenger_count = ?;
SELECT COUNT(*) FROM trips WHERE pickup_location_id = ?;
SELECT COUNT(*) FROM trips WHERE rate_code_id = ?;
SELECT COUNT(*)
FROM trips WHERE pickup_datetime BETWEEN ? AND ? AND pickup_location_id = ?;