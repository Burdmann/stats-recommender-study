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
SELECT COUNT(*) FROM trips WHERE trip_distance BETWEEN 170689.05139603166 AND 284447.03787541494;
SELECT COUNT(*) FROM trips WHERE total_amount BETWEEN 127386.78896198481 AND 322253.06060095795;
SELECT COUNT(*) FROM trips WHERE passenger_count = 3;
SELECT COUNT(*) FROM trips WHERE PULocationID = 264;
SELECT COUNT(*) FROM trips WHERE tpep_pickup_datetime BETWEEN '2004-08-26 10:51:45' AND '2006-08-28 22:03:06' AND fare_amount BETWEEN 224984.93814590524 AND 242223.36459971342;
SELECT COUNT(*) FROM trips WHERE tpep_dropoff_datetime BETWEEN '2009-05-12 01:37:23' AND '2014-03-19 22:29:01' AND trip_distance > 153660.56175269483;
SELECT COUNT(*) FROM trips WHERE fare_amount > 259256.32014593878;
SELECT COUNT(*) FROM trips WHERE trip_distance > 291597.0571561453;
SELECT COUNT(*) FROM trips WHERE PULocationID IN (112, 157, 167);
SELECT COUNT(*) FROM trips WHERE fare_amount = 178572.75257247264;
SELECT COUNT(*) FROM trips WHERE passenger_count = 6;
SELECT COUNT(*) FROM trips WHERE PULocationID = 246;
SELECT COUNT(*) FROM trips WHERE RatecodeID = 80;
SELECT COUNT(*) FROM trips WHERE trip_distance = 11.106895528699386;
SELECT COUNT(*) FROM trips WHERE tpep_pickup_datetime BETWEEN '2012-03-26 05:53:06' AND '2025-11-17 08:59:32' AND PULocationID = 261;
