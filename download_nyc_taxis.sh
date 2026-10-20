mkdir -p yellow_2024
cd yellow_2024
for m in 01 02 03 04 05 06; do     wget https://d37ci6vzurychx.cloudfront.net/trip-data/yellow_tripdata_2024-${m}.parquet; done
duckdb -c "create table tbl as select * from  read_parquet('yellow_tripdata_2024-*.parquet'); COPY tbl TO 'yellow_tripdata_2024.csv' (HEADER, DELIMITER ',');"