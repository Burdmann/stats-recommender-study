//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/table/column_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "duckdb/storage/data_pointer.hpp"
#include "duckdb/storage/table/persistent_table_data.hpp"
#include "duckdb/storage/statistics/segment_statistics.hpp"
#include "duckdb/storage/table/segment_tree.hpp"
#include "duckdb/storage/table/column_segment_tree.hpp"
#include "duckdb/common/mutex.hpp"
#include "duckdb/common/enums/scan_vector_type.hpp"
#include "duckdb/common/serializer/serialization_traits.hpp"
#include "duckdb/common/atomic_ptr.hpp"
#include "duckdb/util/util.hpp"

#include "duckdb/storage/statistics/additional/empty_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/cluster_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/bloom_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/always_prune_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/dictionary_additional_stats.hpp"

namespace duckdb {
class ColumnData;
class ColumnSegment;
class DatabaseInstance;
class RowGroup;
class RowGroupWriter;
class StorageManager;
class TableDataWriter;
class TableStorageInfo;
struct DataTableInfo;
struct PrefetchState;
struct RowGroupWriteInfo;
struct TableScanOptions;
struct TransactionData;
struct PersistentColumnData;

using column_segment_vector_t = vector<SegmentNode<ColumnSegment>>;

struct ColumnCheckpointInfo {
	ColumnCheckpointInfo(RowGroupWriteInfo &info, idx_t column_idx) : info(info), column_idx(column_idx) {
	}

	RowGroupWriteInfo &info;
	idx_t column_idx;

public:
	CompressionType GetCompressionType();
};

class ColumnData {
	friend class ColumnDataCheckpointer;

public:
	ColumnData(BlockManager &block_manager, DataTableInfo &info, idx_t column_index, idx_t start_row, LogicalType type,
	           optional_ptr<ColumnData> parent);
	virtual ~ColumnData();

	//! The start row
	idx_t start;
	//! The count of the column data
	atomic<idx_t> count;
	//! The block manager
	BlockManager &block_manager;
	//! Table info for the column
	DataTableInfo &info;
	//! The column index of the column, either within the parent table or within the parent
	idx_t column_index;
	//! The type of the column
	LogicalType type;

public:
	virtual FilterPropagateResult CheckZonemap(ColumnScanState &state, TableFilter &filter);

	BlockManager &GetBlockManager() {
		return block_manager;
	}
	DatabaseInstance &GetDatabase() const;
	DataTableInfo &GetTableInfo() const;
	StorageManager &GetStorageManager() const;
	virtual idx_t GetMaxEntry();

	idx_t GetAllocationSize() const {
		return allocation_size;
	}
	optional_ptr<const CompressionFunction> GetCompressionFunction() const {
		return compression.get();
	}

	bool HasParent() const {
		return parent != nullptr;
	}
	const ColumnData &Parent() const {
		D_ASSERT(HasParent());
		return *parent;
	}

	virtual void SetStart(idx_t new_start);
	//! The root type of the column
	const LogicalType &RootType() const;
	//! Whether or not the column has any updates
	bool HasUpdates() const;
	bool HasChanges(idx_t start_row, idx_t end_row) const;
	//! Whether or not the column has changes at this level
	bool HasChanges() const;

	//! Whether or not the column has ANY changes, including in child columns
	virtual bool HasAnyChanges() const;
	//! Whether or not we can scan an entire vector
	virtual ScanVectorType GetVectorScanType(ColumnScanState &state, idx_t scan_count, Vector &result);

	//! Initialize prefetch state with required I/O data for the next N rows
	virtual void InitializePrefetch(PrefetchState &prefetch_state, ColumnScanState &scan_state, idx_t rows);
	//! Initialize a scan of the column
	virtual void InitializeScan(ColumnScanState &state);
	//! Initialize a scan starting at the specified offset
	virtual void InitializeScanWithOffset(ColumnScanState &state, idx_t row_idx);
	//! Scan the next vector from the column
	idx_t Scan(TransactionData transaction, idx_t vector_index, ColumnScanState &state, Vector &result);
	idx_t ScanCommitted(idx_t vector_index, ColumnScanState &state, Vector &result, bool allow_updates);
	virtual idx_t Scan(TransactionData transaction, idx_t vector_index, ColumnScanState &state, Vector &result,
	                   idx_t scan_count);
	virtual idx_t ScanCommitted(idx_t vector_index, ColumnScanState &state, Vector &result, bool allow_updates,
	                            idx_t scan_count);

	virtual void ScanCommittedRange(idx_t row_group_start, idx_t offset_in_row_group, idx_t count, Vector &result);
	virtual idx_t ScanCount(ColumnScanState &state, Vector &result, idx_t count, idx_t result_offset = 0);

	//! Select
	virtual void Filter(TransactionData transaction, idx_t vector_index, ColumnScanState &state, Vector &result,
	                    SelectionVector &sel, idx_t &count, const TableFilter &filter, TableFilterState &filter_state);
	virtual void Select(TransactionData transaction, idx_t vector_index, ColumnScanState &state, Vector &result,
	                    SelectionVector &sel, idx_t count);
	virtual void SelectCommitted(idx_t vector_index, ColumnScanState &state, Vector &result, SelectionVector &sel,
	                             idx_t count, bool allow_updates);

	//! Skip the scan forward by "count" rows
	virtual void Skip(ColumnScanState &state, idx_t count = STANDARD_VECTOR_SIZE);

	//! Initialize an appending phase for this column
	virtual void InitializeAppend(ColumnAppendState &state);
	//! Append a vector of type [type] to the end of the column
	virtual void Append(BaseStatistics &stats, ColumnAppendState &state, Vector &vector, idx_t count);
	//! Append a vector of type [type] to the end of the column
	void Append(ColumnAppendState &state, Vector &vector, idx_t count);
	virtual void AppendData(BaseStatistics &stats, ColumnAppendState &state, UnifiedVectorFormat &vdata, idx_t count);
	virtual void AppendDataWriteTemp(BaseStatistics &stats, ColumnAppendState &state, UnifiedVectorFormat &vdata,
	                                 idx_t count);
	//! Revert a set of appends to the ColumnData
	virtual void RevertAppend(row_t start_row);

	//! Fetch the vector from the column data that belongs to this specific row
	virtual idx_t Fetch(ColumnScanState &state, row_t row_id, Vector &result);
	//! Fetch a specific row id and append it to the vector
	virtual void FetchRow(TransactionData transaction, ColumnFetchState &state, row_t row_id, Vector &result,
	                      idx_t result_idx);

	virtual void Update(TransactionData transaction, DataTable &data_table, idx_t column_index, Vector &update_vector,
	                    row_t *row_ids, idx_t update_count);
	virtual void UpdateColumn(TransactionData transaction, DataTable &data_table, const vector<column_t> &column_path,
	                          Vector &update_vector, row_t *row_ids, idx_t update_count, idx_t depth);
	virtual unique_ptr<BaseStatistics> GetUpdateStatistics();

	virtual void CommitDropColumn();

	virtual unique_ptr<ColumnCheckpointState> CreateCheckpointState(RowGroup &row_group,
	                                                                PartialBlockManager &partial_block_manager);
	virtual unique_ptr<ColumnCheckpointState> Checkpoint(RowGroup &row_group, ColumnCheckpointInfo &info);

	virtual void CheckpointScan(ColumnSegment &segment, ColumnScanState &state, idx_t row_group_start, idx_t count,
	                            Vector &scan_vector);

	virtual bool IsPersistent();
	vector<DataPointer> GetDataPointers();

	virtual PersistentColumnData Serialize();
	void InitializeColumn(PersistentColumnData &column_data);
	virtual void InitializeColumn(PersistentColumnData &column_data, BaseStatistics &target_stats);
	static shared_ptr<ColumnData> Deserialize(BlockManager &block_manager, DataTableInfo &info, idx_t column_index,
	                                          idx_t start_row, ReadStream &source, const LogicalType &type);

	virtual void GetColumnSegmentInfo(idx_t row_group_index, vector<idx_t> col_path, vector<ColumnSegmentInfo> &result);
	virtual void Verify(RowGroup &parent);

	FilterPropagateResult CheckZonemap(TableFilter &filter);

	static shared_ptr<ColumnData> CreateColumn(BlockManager &block_manager, DataTableInfo &info, idx_t column_index,
	                                           idx_t start_row, const LogicalType &type,
	                                           optional_ptr<ColumnData> parent = nullptr);
	static unique_ptr<ColumnData> CreateColumnUnique(BlockManager &block_manager, DataTableInfo &info,
	                                                 idx_t column_index, idx_t start_row, const LogicalType &type,
	                                                 optional_ptr<ColumnData> parent = nullptr);

	void MergeStatistics(const BaseStatistics &other);
	void MergeIntoStatistics(BaseStatistics &other);
	unique_ptr<BaseStatistics> GetStatistics();

	template <class T>
	void AppendTemp(UnifiedVectorFormat &adata, idx_t offset, idx_t append_count, std::vector<T> &temp_storage) {
		auto sdata = UnifiedVectorFormat::GetData<T>(adata);
		if (!adata.validity.AllValid()) {
			for (idx_t i = 0; i < append_count; i++) {
				auto source_idx = adata.sel->get_index(offset + i);
				bool is_null = !adata.validity.RowIsValid(source_idx);
				if (!is_null) {
					// if (1768979491171 == *(uint64_t *)&sdata[source_idx])
					// 	printf("ALSO ALSO YES %p (%p)\n", this, &stats->statistics);
					temp_storage.push_back(sdata[source_idx]);
				}
			}
		} else {
			for (idx_t i = 0; i < append_count; i++) {
				auto source_idx = adata.sel->get_index(offset + i);
				// if (1768979491171 == *(uint64_t *)&sdata[source_idx])
				// printf("ALSO ALSO YES %p (%p)\n", this, &stats->statistics);
				temp_storage.push_back(sdata[source_idx]);
			}
		}
	}

	void AppendTempString(UnifiedVectorFormat &adata, idx_t offset, idx_t append_count,
	                      std::vector<std::string> &temp_storage) {
		auto sdata = UnifiedVectorFormat::GetData<string_t>(adata);
		if (!adata.validity.AllValid()) {
			for (idx_t i = 0; i < append_count; i++) {
				auto source_idx = adata.sel->get_index(offset + i);
				bool is_null = !adata.validity.RowIsValid(source_idx);
				if (!is_null) {
					temp_storage.push_back(sdata[source_idx].GetString());
				}
			}
		} else {
			for (idx_t i = 0; i < append_count; i++) {
				auto source_idx = adata.sel->get_index(offset + i);
				temp_storage.push_back(sdata[source_idx].GetString());
			}
		}
	}

	void AppendTemp(UnifiedVectorFormat &vdata, idx_t offset, idx_t copied_elements, BaseStatistics &stats) {
		map_mutex.lock();
		switch (vdata.physical_type) {
		case PhysicalType::BOOL:
			AppendTemp(vdata, offset, copied_elements, bool_temp_vectors[this]);
			break;
		case PhysicalType::INT8:
			AppendTemp(vdata, offset, copied_elements, int8_temp_vectors[this]);
			break;
		case PhysicalType::INT16:
			AppendTemp(vdata, offset, copied_elements, int16_temp_vectors[this]);
			break;
		case PhysicalType::INT32:
			AppendTemp(vdata, offset, copied_elements, int32_temp_vectors[this]);
			break;
		case PhysicalType::INT64:
			AppendTemp(vdata, offset, copied_elements, int64_temp_vectors[this]);
			break;
		case PhysicalType::UINT8:
			AppendTemp(vdata, offset, copied_elements, uint8_temp_vectors[this]);
			break;
		case PhysicalType::UINT16:
			AppendTemp(vdata, offset, copied_elements, uint16_temp_vectors[this]);
			break;
		case PhysicalType::UINT32:
			AppendTemp(vdata, offset, copied_elements, uint32_temp_vectors[this]);
			break;
		case PhysicalType::UINT64:
			AppendTemp(vdata, offset, copied_elements, uint64_temp_vectors[this]);
			break;
		case PhysicalType::INT128:
			AppendTemp(vdata, offset, copied_elements, hugeint_temp_vectors[this]);
			break;
		case PhysicalType::UINT128:
			AppendTemp(vdata, offset, copied_elements, uhugeint_temp_vectors[this]);
			break;
		case PhysicalType::FLOAT:
			AppendTemp(vdata, offset, copied_elements, float_temp_vectors[this]);
			break;
		case PhysicalType::DOUBLE:
			AppendTemp(vdata, offset, copied_elements, double_temp_vectors[this]);
			break;
		case PhysicalType::VARCHAR:
			AppendTempString(vdata, offset, copied_elements, string_temp_vectors[this]);
			break;
		default:
			throw InternalException("Unsupported type for appending to additional stats");
		}
		map_mutex.unlock();
	}

	template <class T>
	void InitAdditionalStats(std::vector<T> &temp_storage, BaseStatistics &stats) {
		uint64_t start_time = Util::GetTime();
		// fprintf(
		//     stderr,
		//     "%lx,%lu,%lu,START_INITIALISE_ADDITIONAL_STATS,\"{\"\"stats\"\":\"\"%p\"\",\"\"type\"\":\"\"%s\"\"}\"\n",
		//     Util::session_id, Util::command_count, start_time, &stats, ADDITIONAL_STATS<T>::GetStaticName());
		// std::cout << "INITIALISE STATS FOR PARTITION -- min: " << stats.stats_union.numeric_data.min.value_.integer
		//           << " max: " << stats.stats_union.numeric_data.max.value_.integer << " type: " <<
		//           stats.type.ToString()
		//           << std::endl;
		stats.additional_stats = new ADDITIONAL_STATS<T>(temp_storage);
		// printf("Column_data %p (%p,%p) GETS %p\n", this, &stats, &this->stats->statistics, stats.additional_stats);
		AdditionalStats<T> &astats = *((ADDITIONAL_STATS<T> *)stats.additional_stats);
		temp_storage.clear();
		fprintf(stderr,
		        "%lx,%lu,%lu,END_INITIALISE_ADDITIONAL_STATS,\"{\"\"stats\"\":\"\"%p\"\",\"\"type\"\":\"\"%s\"\","
		        "\"\"size\"\":%lu,\"\"start_time\"\":%lu}\"\n",
		        Util::session_id, Util::command_count, Util::GetTime(), &stats, ADDITIONAL_STATS<T>::GetStaticName(),
		        astats.Size(&astats), start_time);
	}

	void InitStats(BaseStatistics &stats) {
		map_mutex.lock();
		switch (type.InternalType()) {
		case PhysicalType::BOOL:
			InitAdditionalStats(bool_temp_vectors[this], stats);
			bool_temp_vectors.erase(this);
			break;
		case PhysicalType::INT8:
			InitAdditionalStats(int8_temp_vectors[this], stats);
			int8_temp_vectors.erase(this);
			break;
		case PhysicalType::INT16:
			InitAdditionalStats(int16_temp_vectors[this], stats);
			int16_temp_vectors.erase(this);
			break;
		case PhysicalType::INT32:
			InitAdditionalStats(int32_temp_vectors[this], stats);
			int32_temp_vectors.erase(this);
			break;
		case PhysicalType::INT64:
			InitAdditionalStats(int64_temp_vectors[this], stats);
			int64_temp_vectors.erase(this);
			break;
		case PhysicalType::INT128:
			InitAdditionalStats(hugeint_temp_vectors[this], stats);
			hugeint_temp_vectors.erase(this);
			break;
		case PhysicalType::UINT8:
			InitAdditionalStats(uint8_temp_vectors[this], stats);
			uint8_temp_vectors.erase(this);
			break;
		case PhysicalType::UINT16:
			InitAdditionalStats(uint16_temp_vectors[this], stats);
			uint16_temp_vectors.erase(this);
			break;
		case PhysicalType::UINT32:
			InitAdditionalStats(uint32_temp_vectors[this], stats);
			uint32_temp_vectors.erase(this);
			break;
		case PhysicalType::UINT64:
			InitAdditionalStats(uint64_temp_vectors[this], stats);
			uint64_temp_vectors.erase(this);
			break;
		case PhysicalType::UINT128:
			InitAdditionalStats(uhugeint_temp_vectors[this], stats);
			uhugeint_temp_vectors.erase(this);
			break;
		case PhysicalType::FLOAT:
			InitAdditionalStats(float_temp_vectors[this], stats);
			float_temp_vectors.erase(this);
			break;
		case PhysicalType::DOUBLE:
			InitAdditionalStats(double_temp_vectors[this], stats);
			double_temp_vectors.erase(this);
			break;
		case PhysicalType::VARCHAR:
			InitAdditionalStats(string_temp_vectors[this], stats);
			string_temp_vectors.erase(this);
			break;
		default:
			throw InternalException("Unsupported type for appending to numeric cluster stats");
		}
		map_mutex.unlock();
	}

	template <class T>
	static inline FilterPropagateResult QueryAdditionalStats(BaseStatistics &stats, ExpressionType comparison_type,
	                                                         const T &constant) {
		if (stats.additional_stats == NULL) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
		ADDITIONAL_STATS<T> &astats = *((ADDITIONAL_STATS<T> *)stats.additional_stats);
		uint64_t start_time = Util::GetTime();
		FilterPropagateResult result = astats.Query(&astats, comparison_type, constant);
		// if (result == FilterPropagateResult::NO_PRUNING_POSSIBLE) {
		// 	ClusterAdditionalStats<T>::Print(&astats);
		// }
		// fprintf(stderr,
		//         "%lx,%lu,%lu,EVAL_ADDITIONAL_STATISTICS_END,\"{\"\"statistic\"\":\"\"%p\"\",\"\"type\"\":\"\"%s\"\","
		//         "\"\"start_time\"\":%lu,\"\"result\"\":%u}\"\n",
		//         duckdb::Util::session_id, duckdb::Util::command_count, duckdb::Util::GetTime(), &stats, astats->name,
		//         start_time, (unsigned int)result);
		return result;
	}

	static inline FilterPropagateResult QueryAdditionalStats(BaseStatistics &stats, ExpressionType comparison_type,
	                                                         PhysicalType type, const Value *constant) {
		switch (type) {
		case PhysicalType::BOOL:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<bool>());
		case PhysicalType::INT8:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<int8_t>());
		case PhysicalType::INT16:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<int16_t>());
		case PhysicalType::INT32:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<int32_t>());
		case PhysicalType::INT64:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<int64_t>());
		case PhysicalType::INT128:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<hugeint_t>());
		case PhysicalType::UINT8:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<uint8_t>());
		case PhysicalType::UINT16:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<uint16_t>());
		case PhysicalType::UINT32:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<uint32_t>());
		case PhysicalType::UINT64:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<uint64_t>());
		case PhysicalType::UINT128:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<uhugeint_t>());
		case PhysicalType::FLOAT:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<float>());
		case PhysicalType::DOUBLE:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<double>());
		case PhysicalType::VARCHAR:
			return QueryAdditionalStats(stats, comparison_type, constant->GetValueUnsafe<string_t>().GetString());
		default:
			throw InternalException("Unsupported type querying additional stats");
		}
	}

	template <class T>
	static inline FilterPropagateResult RangeQueryAdditionalStats(BaseStatistics &stats, const T start, const T end) {
		if (stats.additional_stats == NULL)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		ADDITIONAL_STATS<T> &astats = *((ADDITIONAL_STATS<T> *)stats.additional_stats);
		uint64_t start_time = Util::GetTime();
		FilterPropagateResult result = astats.QueryRange(&astats, start, end);
		// fprintf(stderr,
		//         "%lx,%lu,%lu,EVAL_ADDITIONAL_STATISTICS_END,\"{\"\"statistic\"\":\"\"%p\"\",\"\"type\"\":\"\"%s\"\","
		//         "\"\"start_time\"\":%lu,\"\"result\"\":%u}\"\n",
		//         duckdb::Util::session_id, duckdb::Util::command_count, duckdb::Util::GetTime(), &stats, astats->name,
		//         start_time, (unsigned int)result);
		return result;
	}

	static inline FilterPropagateResult RangeQueryAdditionalStats(BaseStatistics &stats, PhysicalType type,
	                                                              const Value start, const Value end) {
		switch (type) {
		case PhysicalType::BOOL:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<bool>(), end.GetValueUnsafe<bool>());
		case PhysicalType::INT8:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<int8_t>(), end.GetValueUnsafe<int8_t>());
		case PhysicalType::INT16:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<int16_t>(), end.GetValueUnsafe<int16_t>());
		case PhysicalType::INT32:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<int32_t>(), end.GetValueUnsafe<int32_t>());
		case PhysicalType::INT64:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<int64_t>(), end.GetValueUnsafe<int64_t>());
		case PhysicalType::INT128:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<hugeint_t>(), end.GetValueUnsafe<hugeint_t>());
		case PhysicalType::UINT8:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<uint8_t>(), end.GetValueUnsafe<uint8_t>());
		case PhysicalType::UINT16:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<uint16_t>(), end.GetValueUnsafe<uint16_t>());
		case PhysicalType::UINT32:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<uint32_t>(), end.GetValueUnsafe<uint32_t>());
		case PhysicalType::UINT64:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<uint64_t>(), end.GetValueUnsafe<uint64_t>());
		case PhysicalType::UINT128:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<uhugeint_t>(),
			                                 end.GetValueUnsafe<uhugeint_t>());
		case PhysicalType::FLOAT:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<float>(), end.GetValueUnsafe<float>());
		case PhysicalType::DOUBLE:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<double>(), end.GetValueUnsafe<double>());
		case PhysicalType::VARCHAR:
			return RangeQueryAdditionalStats(stats, start.GetValueUnsafe<string_t>().GetString(),
			                                 end.GetValueUnsafe<string_t>().GetString());
		default:
			throw InternalException("Unsupported type querying additional stats");
		}
	}

	inline SegmentStatistics *GetStats() {
		return stats.get();
	}

protected:
	//! Append a transient segment
	void AppendTransientSegment(SegmentLock &l, idx_t start_row);
	void AppendSegment(SegmentLock &l, unique_ptr<ColumnSegment> segment);

	void BeginScanVectorInternal(ColumnScanState &state);
	//! Scans a base vector from the column
	idx_t ScanVector(ColumnScanState &state, Vector &result, idx_t remaining, ScanVectorType scan_type,
	                 idx_t result_offset = 0);
	//! Scans a vector from the column merged with any potential updates
	idx_t ScanVector(TransactionData transaction, idx_t vector_index, ColumnScanState &state, Vector &result,
	                 idx_t target_scan, ScanVectorType scan_type, ScanVectorMode mode);
	idx_t ScanVector(TransactionData transaction, idx_t vector_index, ColumnScanState &state, Vector &result,
	                 idx_t target_scan, ScanVectorMode mode);
	void SelectVector(ColumnScanState &state, Vector &result, idx_t target_count, const SelectionVector &sel,
	                  idx_t sel_count);
	void FilterVector(ColumnScanState &state, Vector &result, idx_t target_count, SelectionVector &sel,
	                  idx_t &sel_count, const TableFilter &filter, TableFilterState &filter_state);

	void ClearUpdates();
	void FetchUpdates(TransactionData transaction, idx_t vector_index, Vector &result, idx_t scan_count,
	                  bool allow_updates, bool scan_committed);
	void FetchUpdateRow(TransactionData transaction, row_t row_id, Vector &result, idx_t result_idx);
	void UpdateInternal(TransactionData transaction, DataTable &data_table, idx_t column_index, Vector &update_vector,
	                    row_t *row_ids, idx_t update_count, Vector &base_vector);
	idx_t FetchUpdateData(ColumnScanState &state, row_t *row_ids, Vector &base_vector);

	idx_t GetVectorCount(idx_t vector_index) const;

private:
	void UpdateCompressionFunction(SegmentLock &l, const CompressionFunction &function);

protected:
	//! The segments holding the data of this column segment
	ColumnSegmentTree data;
	//! The lock for the updates
	mutable mutex update_lock;
	//! The updates for this column segment
	unique_ptr<UpdateSegment> updates;
	//! The lock for the stats
	mutable mutex stats_lock;
	//! Total transient allocation size
	atomic<idx_t> allocation_size;

public:
	//! The stats of the root segment
	unique_ptr<SegmentStatistics> stats;

private:
	//! The parent column (if any)
	optional_ptr<ColumnData> parent;
	//!	The compression function used by the ColumnData
	//! This is empty if the segments have mixed compression or the ColumnData is empty
	atomic_ptr<const CompressionFunction> compression;

	//! altp: temporary storage for data between calls to append
	static std::unordered_map<void *, std::vector<bool>> bool_temp_vectors;
	static std::unordered_map<void *, std::vector<int8_t>> int8_temp_vectors;
	static std::unordered_map<void *, std::vector<int16_t>> int16_temp_vectors;
	static std::unordered_map<void *, std::vector<int32_t>> int32_temp_vectors;
	static std::unordered_map<void *, std::vector<int64_t>> int64_temp_vectors;
	static std::unordered_map<void *, std::vector<uint8_t>> uint8_temp_vectors;
	static std::unordered_map<void *, std::vector<uint16_t>> uint16_temp_vectors;
	static std::unordered_map<void *, std::vector<uint32_t>> uint32_temp_vectors;
	static std::unordered_map<void *, std::vector<uint64_t>> uint64_temp_vectors;
	static std::unordered_map<void *, std::vector<hugeint_t>> hugeint_temp_vectors;
	static std::unordered_map<void *, std::vector<uhugeint_t>> uhugeint_temp_vectors;
	static std::unordered_map<void *, std::vector<float>> float_temp_vectors;
	static std::unordered_map<void *, std::vector<double>> double_temp_vectors;
	static std::unordered_map<void *, std::vector<std::string>> string_temp_vectors;
	static std::mutex map_mutex;
};

struct PersistentColumnData {
	explicit PersistentColumnData(PhysicalType physical_type);
	PersistentColumnData(PhysicalType physical_type, vector<DataPointer> pointers);
	// disable copy constructors
	PersistentColumnData(const PersistentColumnData &other) = delete;
	PersistentColumnData &operator=(const PersistentColumnData &) = delete;
	//! enable move constructors
	PersistentColumnData(PersistentColumnData &&other) noexcept = default;
	PersistentColumnData &operator=(PersistentColumnData &&) = default;
	~PersistentColumnData();

	PhysicalType physical_type;
	vector<DataPointer> pointers;
	vector<PersistentColumnData> child_columns;
	bool has_updates = false;

	void Serialize(Serializer &serializer) const;
	static PersistentColumnData Deserialize(Deserializer &deserializer);
	void DeserializeField(Deserializer &deserializer, field_id_t field_idx, const char *field_name,
	                      const LogicalType &type);
	bool HasUpdates() const;
};

struct PersistentRowGroupData {
	explicit PersistentRowGroupData(vector<LogicalType> types);
	PersistentRowGroupData() = default;
	// disable copy constructors
	PersistentRowGroupData(const PersistentRowGroupData &other) = delete;
	PersistentRowGroupData &operator=(const PersistentRowGroupData &) = delete;
	//! enable move constructors
	PersistentRowGroupData(PersistentRowGroupData &&other) noexcept = default;
	PersistentRowGroupData &operator=(PersistentRowGroupData &&) = default;
	~PersistentRowGroupData() = default;

	vector<LogicalType> types;
	vector<PersistentColumnData> column_data;
	idx_t start;
	idx_t count;

	void Serialize(Serializer &serializer) const;
	static PersistentRowGroupData Deserialize(Deserializer &deserializer);
	bool HasUpdates() const;
};

struct PersistentCollectionData {
	PersistentCollectionData() = default;
	// disable copy constructors
	PersistentCollectionData(const PersistentCollectionData &other) = delete;
	PersistentCollectionData &operator=(const PersistentCollectionData &) = delete;
	//! enable move constructors
	PersistentCollectionData(PersistentCollectionData &&other) noexcept = default;
	PersistentCollectionData &operator=(PersistentCollectionData &&) = default;
	~PersistentCollectionData() = default;

	vector<PersistentRowGroupData> row_group_data;

	void Serialize(Serializer &serializer) const;
	static PersistentCollectionData Deserialize(Deserializer &deserializer);
	bool HasUpdates() const;
};

} // namespace duckdb
