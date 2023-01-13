//===----------------------------------------------------------------------===//
//                         DuckDB
//
// geo_aggregate_function.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "geometry.hpp"

namespace duckdb {

// convert epsilon from km to radians
double MS_PER_RADIAN = 6371.0088 * 1000;

struct ClusterDBScanIncluded {
	inline explicit ClusterDBScanIncluded(const ValidityMask &fmask_p, const ValidityMask &gmask_p,
	                                      const ValidityMask &emask_p, const ValidityMask &mmask_p, idx_t bias_p)
	    : fmask(fmask_p), gmask(gmask_p), emask(emask_p), mmask(mmask_p), bias(bias_p) {
	}

	inline bool operator()(const idx_t &idx) const {
		return fmask.RowIsValid(idx) && gmask.RowIsValid(idx - bias) && emask.RowIsValid(idx - bias) &&
		       mmask.RowIsValid(idx - bias);
	}
	const ValidityMask &fmask;
	const ValidityMask &gmask;
	const ValidityMask &emask;
	const ValidityMask &mmask;
	const idx_t bias;
};

class GeoAggregateExecutor {
private:
	template <class STATE_TYPE, class A_TYPE, class B_TYPE, class C_TYPE, class OP>
	static inline void TernaryScatterLoop(A_TYPE *__restrict adata, AggregateInputData &aggr_input_data,
	                                      B_TYPE *__restrict bdata, C_TYPE *__restrict cdata,
	                                      STATE_TYPE **__restrict states, idx_t count, const SelectionVector &asel,
	                                      const SelectionVector &bsel, const SelectionVector &csel,
	                                      const SelectionVector &ssel, ValidityMask &avalidity, ValidityMask &bvalidity,
	                                      ValidityMask &cvalidity) {
		if (OP::IgnoreNull() && (!avalidity.AllValid() || !bvalidity.AllValid() || !cvalidity.AllValid())) {
			// potential NULL values and NULL values are ignored
			for (idx_t i = 0; i < count; i++) {
				auto aidx = asel.get_index(i);
				auto bidx = bsel.get_index(i);
				auto cidx = csel.get_index(i);
				auto sidx = ssel.get_index(i);
				if (avalidity.RowIsValid(aidx) && bvalidity.RowIsValid(bidx) && cvalidity.RowIsValid(cidx)) {
					OP::template Operation<A_TYPE, B_TYPE, C_TYPE, STATE_TYPE, OP>(states[sidx], aggr_input_data, adata,
					                                                               bdata, cdata, avalidity, bvalidity,
					                                                               cvalidity, aidx, bidx, cidx);
				}
			}
		} else {
			// quick path: no NULL values or NULL values are not ignored
			for (idx_t i = 0; i < count; i++) {
				auto aidx = asel.get_index(i);
				auto bidx = bsel.get_index(i);
				auto cidx = csel.get_index(i);
				auto sidx = ssel.get_index(i);
				OP::template Operation<A_TYPE, B_TYPE, C_TYPE, STATE_TYPE, OP>(states[sidx], aggr_input_data, adata,
				                                                               bdata, cdata, avalidity, bvalidity,
				                                                               cvalidity, aidx, bidx, cidx);
			}
		}
	}

	template <class STATE_TYPE, class A_TYPE, class B_TYPE, class C_TYPE, class OP>
	static inline void TernaryUpdateLoop(A_TYPE *__restrict adata, AggregateInputData &aggr_input_data,
	                                     B_TYPE *__restrict bdata, C_TYPE *__restrict cdata,
	                                     STATE_TYPE *__restrict state, idx_t count, const SelectionVector &asel,
	                                     const SelectionVector &bsel, const SelectionVector &csel,
	                                     ValidityMask &avalidity, ValidityMask &bvalidity, ValidityMask &cvalidity) {
		if (OP::IgnoreNull() && (!avalidity.AllValid() || !bvalidity.AllValid() || !cvalidity.AllValid())) {
			// potential NULL values and NULL values are ignored
			for (idx_t i = 0; i < count; i++) {
				auto aidx = asel.get_index(i);
				auto bidx = bsel.get_index(i);
				auto cidx = csel.get_index(i);
				if (avalidity.RowIsValid(aidx) && bvalidity.RowIsValid(bidx) && cvalidity.RowIsValid(cidx)) {
					OP::template Operation<A_TYPE, B_TYPE, C_TYPE, STATE_TYPE, OP>(
					    state, aggr_input_data, adata, bdata, cdata, avalidity, bvalidity, cvalidity, aidx, bidx, cidx);
				}
			}
		} else {
			// quick path: no NULL values or NULL values are not ignored
			for (idx_t i = 0; i < count; i++) {
				auto aidx = asel.get_index(i);
				auto bidx = bsel.get_index(i);
				auto cidx = csel.get_index(i);
				OP::template Operation<A_TYPE, B_TYPE, C_TYPE, STATE_TYPE, OP>(
				    state, aggr_input_data, adata, bdata, cdata, avalidity, bvalidity, cvalidity, aidx, bidx, cidx);
			}
		}
	}

public:
	template <class STATE_TYPE, class A_TYPE, class B_TYPE, class C_TYPE, class OP>
	static void TernaryScatter(AggregateInputData &aggr_input_data, Vector &a, Vector &b, Vector &c, Vector &states,
	                           idx_t count) {
		UnifiedVectorFormat adata, bdata, cdata, sdata;

		a.ToUnifiedFormat(count, adata);
		b.ToUnifiedFormat(count, bdata);
		c.ToUnifiedFormat(count, cdata);
		states.ToUnifiedFormat(count, sdata);

		TernaryScatterLoop<STATE_TYPE, A_TYPE, B_TYPE, C_TYPE, OP>(
		    (A_TYPE *)adata.data, aggr_input_data, (B_TYPE *)bdata.data, (C_TYPE *)cdata.data,
		    (STATE_TYPE **)sdata.data, count, *adata.sel, *bdata.sel, *cdata.sel, *sdata.sel, adata.validity,
		    bdata.validity, cdata.validity);
	}

	template <class STATE_TYPE, class A_TYPE, class B_TYPE, class C_TYPE, class OP>
	static void TernaryUpdate(AggregateInputData &aggr_input_data, Vector &a, Vector &b, Vector &c, data_ptr_t state,
	                          idx_t count) {
		UnifiedVectorFormat adata, bdata, cdata;

		a.ToUnifiedFormat(count, adata);
		b.ToUnifiedFormat(count, bdata);
		c.ToUnifiedFormat(count, cdata);

		TernaryUpdateLoop<STATE_TYPE, A_TYPE, B_TYPE, C_TYPE, OP>(
		    (A_TYPE *)adata.data, aggr_input_data, (B_TYPE *)bdata.data, (C_TYPE *)cdata.data, (STATE_TYPE *)state,
		    count, *adata.sel, *bdata.sel, *cdata.sel, adata.validity, bdata.validity, cdata.validity);
	}

	template <class STATE, class A_TYPE, class B_TYPE, class C_TYPE, class RESULT_TYPE, class OP>
	static void TernaryWindow(Vector &a, Vector &b, Vector &c, const ValidityMask &ifilter,
	                          AggregateInputData &aggr_input_data, data_ptr_t state, const FrameBounds &frame,
	                          const FrameBounds &prev, Vector &result, idx_t rid, idx_t bias) {

		auto adata = FlatVector::GetData<const A_TYPE>(a) - bias;
		const auto &avalid = FlatVector::Validity(a);
		auto bdata = FlatVector::GetData<const B_TYPE>(b) - bias;
		const auto &bvalid = FlatVector::Validity(b);
		auto cdata = FlatVector::GetData<const C_TYPE>(c) - bias;
		const auto &cvalid = FlatVector::Validity(c);
		OP::template Window<STATE, A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE>(adata, bdata, cdata, ifilter, avalid, bvalid,
		                                                                cvalid, aggr_input_data, (STATE *)state, frame,
		                                                                prev, result, rid, bias);
	}
};

struct ClusterDBScanState {
	bool isset;
	double epsilon;
	int minpoints;
	std::vector<int> clusters;

	void Initialize() {
		this->isset = false;
		this->epsilon = 0;
		this->minpoints = 0;
		this->clusters = {};
	}

	void Combine(const ClusterDBScanState &other) {
		this->isset = other.isset || this->isset;
		this->epsilon = other.epsilon;
		this->minpoints = other.minpoints;
		this->clusters.insert(this->clusters.end(), other.clusters.begin(), other.clusters.end());
	}
};

struct ClusterDBScanOperation {
	template <class STATE>
	static void Initialize(STATE *state) {
		state->clusters = {};
		state->epsilon = 0;
		state->minpoints = 0;
	}

	template <class STATE, class OP>
	static void Combine(const STATE &source, STATE *target, AggregateInputData &aggr_input_data) {
		target->Combine(source);
	}

	template <class A_TYPE, class B_TYPE, class C_TYPE, class STATE, class OP>
	static void Operation(STATE *state, AggregateInputData &, A_TYPE *x_data, B_TYPE *y_data, C_TYPE *z_data,
	                      ValidityMask &amask, ValidityMask &bmask, ValidityMask &cmask, idx_t xidx, idx_t yidx,
	                      idx_t zidx) {
		// state->isset = true;
		// state->epsilon = y_data[yidx];
		// state->minpoints = z_data[zidx];
	}

	template <class A_TYPE, class B_TYPE, class C_TYPE, class STATE, class OP>
	static void ConstantOperation(STATE *state, AggregateInputData &, A_TYPE *x_data, B_TYPE *y_data, C_TYPE *z_data,
	                              ValidityMask &mask, idx_t count) {
		// state->isset = true;
		// for (size_t i = 0; i < count; i++) {
		// 	state->epsilon = y_data[i];
		// 	state->minpoints = z_data[i];
		// }
	}

	static bool IgnoreNull() {
		return true;
	}

	template <class T, class STATE>
	static void Finalize(Vector &result, AggregateInputData &, STATE *state, T *target, ValidityMask &mask, idx_t idx) {
		// if (!state->isset) {
		// 	mask.SetInvalid(idx);
		// } else {
		// }
	}

	template <class STATE, class A_TYPE, class B_TYPE, class C_TYPE, class RESULT_TYPE>
	static void Window(const A_TYPE *adata, const B_TYPE *bdata, const C_TYPE *cdata, const ValidityMask &fmask,
	                   const ValidityMask &amask, const ValidityMask &bmask, const ValidityMask &cmask,
	                   AggregateInputData &aggr_input_data, STATE *state, const FrameBounds &frame,
	                   const FrameBounds &prev, Vector &result, idx_t ridx, idx_t bias) {
		ClusterDBScanIncluded include(fmask, amask, bmask, cmask, bias);

		auto rdata = FlatVector::GetData<RESULT_TYPE>(result);
		auto &rmask = FlatVector::Validity(result);
		double epsilon = bdata[ridx] / MS_PER_RADIAN;
		int minpoints = cdata[ridx];
		if (!state->isset || frame.first != prev.first || frame.second != prev.second || state->epsilon != epsilon ||
		    state->minpoints != minpoints) {
			state->isset = true;
			state->epsilon = epsilon;
			state->minpoints = minpoints;
			size_t asize = frame.second - frame.first;
			std::vector<GSERIALIZED *> gserArray {};
			std::vector<int> indexVec(asize, -1);
			int idx = 0;

			for (size_t i = frame.first; i < frame.second; i++) {
				if (include(i)) {
					auto gser = Geometry::GetGserialized(adata[i]);
					if (!Geometry::IsEmpty(gser)) {
						gserArray.push_back(gser);
						indexVec[i - frame.first] = idx++;
					} else {
						Geometry::DestroyGeometry(gser);
					}
				}
			}

			// Doing cluster db scan
			auto clusters = Geometry::GeometryClusterDBScan(&gserArray[0], gserArray.size(), epsilon, minpoints);

			state->clusters = {};

			for (idx_t i = 0; i < asize; i++) {
				if (indexVec[i] == -1) {
					state->clusters.push_back(-1);
				} else {
					state->clusters.push_back(clusters[indexVec[i]]);
				}
			}

			// Free memory
			for (idx_t child_idx = 0; child_idx < gserArray.size(); child_idx++) {
				Geometry::DestroyGeometry(gserArray[child_idx]);
			}

			if (state->clusters[ridx - frame.first] == -1) {
				rmask.SetInvalid(ridx);
			} else {
				rdata[ridx] = state->clusters[ridx - frame.first];
			}
		} else {
			if (state->clusters[ridx - frame.first] == -1) {
				rmask.SetInvalid(ridx);
			} else {
				rdata[ridx] = state->clusters[ridx - frame.first];
			}
		}
	}

	template <class STATE>
	static void Destroy(STATE *state) {
	}
};

template <class STATE, class A_TYPE, class B_TYPE, class C_TYPE, class OP>
static void TernaryScatterUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                                 Vector &states, idx_t count) {
	D_ASSERT(input_count == 3);
	GeoAggregateExecutor::TernaryScatter<STATE, A_TYPE, B_TYPE, C_TYPE, OP>(aggr_input_data, inputs[0], inputs[1],
	                                                                        inputs[2], states, count);
}

template <class STATE, class A_TYPE, class B_TYPE, class C_TYPE, class OP>
static void TernaryUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, data_ptr_t state,
                          idx_t count) {
	D_ASSERT(input_count == 3);
	GeoAggregateExecutor::TernaryUpdate<STATE, A_TYPE, B_TYPE, C_TYPE, OP>(aggr_input_data, inputs[0], inputs[1],
	                                                                       inputs[2], state, count);
}

template <class STATE, class A_TYPE, class B_TYPE, class C_TYPE, class RESULT_TYPE, class OP>
static void TernaryWindow(Vector inputs[], const ValidityMask &filter_mask, AggregateInputData &aggr_input_data,
                          idx_t input_count, data_ptr_t state, const FrameBounds &frame, const FrameBounds &prev,
                          Vector &result, idx_t rid, idx_t bias) {
	D_ASSERT(input_count == 3);
	GeoAggregateExecutor::TernaryWindow<STATE, A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, OP>(
	    inputs[0], inputs[1], inputs[2], filter_mask, aggr_input_data, state, frame, prev, result, rid, bias);
}

unique_ptr<FunctionData> BindGeometryClusterDBScan(ClientContext &context, AggregateFunction &function,
                                                   vector<unique_ptr<Expression>> &arguments) {
	auto geo_type = arguments[0]->return_type;
	function =
	    AggregateFunction({geo_type, LogicalType::DOUBLE, LogicalType::INTEGER}, LogicalType::INTEGER,
	                      AggregateFunction::StateSize<ClusterDBScanState>,
	                      AggregateFunction::StateInitialize<ClusterDBScanState, ClusterDBScanOperation>,
	                      TernaryScatterUpdate<ClusterDBScanState, string_t, double, int, ClusterDBScanOperation>,
	                      AggregateFunction::StateCombine<ClusterDBScanState, ClusterDBScanOperation>,
	                      AggregateFunction::StateFinalize<ClusterDBScanState, double, ClusterDBScanOperation>,
	                      FunctionNullHandling::DEFAULT_NULL_HANDLING,
	                      TernaryUpdate<ClusterDBScanState, string_t, double, int, ClusterDBScanOperation>, nullptr,
	                      AggregateFunction::StateDestroy<ClusterDBScanState, ClusterDBScanOperation>, nullptr,
	                      TernaryWindow<ClusterDBScanState, string_t, double, int, int, ClusterDBScanOperation>);
	function.name = "st_clusterdbscan";
	function.arguments[0] = geo_type;
	return nullptr;
}

static const AggregateFunctionSet GetClusterDBScanAggregateFunction(LogicalType geo_type) {
	// ST_CLUSTERDBSCAN
	AggregateFunctionSet cluster_dbscan("st_clusterdbscan");
	cluster_dbscan.AddFunction(AggregateFunction(
	    {geo_type, LogicalType::DOUBLE, LogicalType::INTEGER}, LogicalTypeId::INTEGER, nullptr, nullptr, nullptr,
	    nullptr, nullptr, FunctionNullHandling::DEFAULT_NULL_HANDLING, nullptr, BindGeometryClusterDBScan));

	return cluster_dbscan;
}

} // namespace duckdb
