# for m in {1..12}; do
#     wget \
#     "https://transtats.bts.gov/PREZIP/On_Time_Reporting_Carrier_On_Time_Performance_1987_present_2025_${m}.zip"
# done
for m in {1..12}; do
    yes | unzip "On_Time_Reporting_Carrier_On_Time_Performance_1987_present_2025_${m}.zip"
    # rm "On_Time_Reporting_Carrier_On_Time_Performance_1987_present_2025_${m}.zip"
done

