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
COPY trips FROM 'yellow_2024/small.csv' (FORMAT 'csv', delimiter ',', header 1);
SELECT COUNT(*) FROM trips WHERE trip_distance BETWEEN 84717.3654660687 AND 262789.4380071186;
SELECT COUNT(*) FROM trips WHERE tpep_pickup_datetime BETWEEN '2023-07-25 11:04:38' AND '2015-01-16 12:16:15' AND fare_amount BETWEEN 7221.125816179053 AND 244239.65362113802;
SELECT COUNT(*) FROM trips WHERE fare_amount > 239860.55583461394;
SELECT COUNT(*) FROM trips WHERE trip_distance > 179388.88834604202;
SELECT COUNT(*) FROM trips WHERE PULocationID IN (76, 105, 164);