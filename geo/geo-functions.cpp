#include "geo-functions.hpp"

#include "duckdb/common/types/vector.hpp"
#include "duckdb/common/vector_operations/generic_executor.hpp"
#include "geometry.hpp"

namespace duckdb {

bool GeoFunctions::CastVarcharToGEO(Vector &source, Vector &result, idx_t count, CastParameters &parameters) {
	auto constant = source.GetVectorType() == VectorType::CONSTANT_VECTOR;

	UnifiedVectorFormat vdata;
	source.ToUnifiedFormat(count, vdata);

	auto input = (string_t *)vdata.data;
	auto result_data = FlatVector::GetData<string_t>(result);
	bool success = true;
	for (idx_t i = 0; i < (constant ? 1 : count); i++) {
		auto idx = vdata.sel->get_index(i);

		if (!vdata.validity.RowIsValid(idx)) {
			FlatVector::SetNull(result, i, true);
			continue;
		}

		auto gser = Geometry::ToGserialized(input[idx]);
		if (!gser) {
			FlatVector::SetNull(result, i, true);
			success = false;
			continue;
		}
		idx_t rv_size = Geometry::GetGeometrySize(gser);
		string_t rv = StringVector::EmptyString(result, rv_size);
		Geometry::ToGeometry(gser, (data_ptr_t)rv.GetDataWriteable());
		Geometry::DestroyGeometry(gser);
		rv.Finalize();
		result_data[i] = rv;
	}
	if (constant) {
		result.SetVectorType(VectorType::CONSTANT_VECTOR);
	}
	return success;
}

bool GeoFunctions::CastGeoToVarchar(Vector &source, Vector &result, idx_t count, CastParameters &parameters) {
	GenericExecutor::ExecuteUnary<PrimitiveType<string_t>, PrimitiveType<string_t>>(
	    source, result, count, [&](PrimitiveType<string_t> input) {
		    // auto text = Geometry::GetString(input.val, DataFormatType::FORMAT_VALUE_TYPE_GEOJSON);
		    auto text = Geometry::GetString(input.val);
		    return StringVector::AddString(result, text);
	    });
	return true;
}

struct MakePointBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA point_x, TB point_y) {
		auto gser = Geometry::MakePoint(point_x, point_y);
		idx_t rv_size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, rv_size);
	}
};

struct MakePointTernaryOperator {
	template <class TA, class TB, class TC, class TR>
	static inline TR Operation(TA point_x, TB point_y, TC point_z) {
		auto gser = Geometry::MakePoint(point_x, point_y, point_z);
		idx_t rv_size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, rv_size);
	}
};

template <typename TA, typename TB, typename TR>
static void MakePointBinaryExecutor(Vector &point_x, Vector &point_y, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, MakePointBinaryOperator>(point_x, point_y, result, count);
}

template <typename TA, typename TB, typename TC, typename TR>
static void MakePointTernaryExecutor(Vector &point_x, Vector &point_y, Vector &point_z, Vector &result, idx_t count) {
	TernaryExecutor::Execute<TA, TB, TC, TR>(point_x, point_y, point_z, result, count,
	                                         MakePointTernaryOperator::Operation<TA, TB, TC, TR>);
}

void GeoFunctions::MakePointFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &point_x_arg = args.data[0];
	auto &point_y_arg = args.data[1];
	if (args.data.size() == 2) {
		MakePointBinaryExecutor<double, double, string_t>(point_x_arg, point_y_arg, result, args.size());
	} else if (args.data.size() == 3) {
		auto &point_z_arg = args.data[2];
		MakePointTernaryExecutor<double, double, double, string_t>(point_x_arg, point_y_arg, point_z_arg, result,
		                                                           args.size());
	}
}

struct MakeLineBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA point1, TB point2) {
		if (point1.GetSize() == 0 || point2.GetSize() == 0) {
			return NULL;
		}
		auto gser1 = Geometry::GetGserialized(point1);
		auto gser2 = Geometry::GetGserialized(point2);
		if (!gser1 || !gser2) {
			throw ConversionException("Failure in geometry distance: could not calculate distance from geometries");
		}
		auto gser = Geometry::MakeLine(gser1, gser2);
		idx_t rv_size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, rv_size);
	}
};

template <typename TA, typename TB, typename TR>
static void MakeLineBinaryExecutor(Vector &point1, Vector &point2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, MakeLineBinaryOperator>(point1, point2, result, count);
}

void GeoFunctions::MakeLineFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &point1_arg = args.data[0];
	auto &point2_arg = args.data[1];
	if (args.data.size() == 2) {
		MakeLineBinaryExecutor<string_t, string_t, string_t>(point1_arg, point2_arg, result, args.size());
	}
}

void GeoFunctions::MakeLineArrayFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	Vector &input = args.data[0];
	auto count = args.size();
	result.SetVectorType(VectorType::CONSTANT_VECTOR);
	if (input.GetVectorType() != VectorType::CONSTANT_VECTOR) {
		result.SetVectorType(VectorType::FLAT_VECTOR);
	}

	auto result_entries = FlatVector::GetData<string_t>(result);
	auto &result_validity = FlatVector::Validity(result);

	auto list_size = ListVector::GetListSize(input);
	auto &child_vector = ListVector::GetEntry(input);

	UnifiedVectorFormat child_data;
	child_vector.ToUnifiedFormat(list_size, child_data);

	UnifiedVectorFormat list_data;
	input.ToUnifiedFormat(count, list_data);
	auto list_entries = (list_entry_t *)list_data.data;

	// not required for a comparison of nested types
	auto child_value = (string_t *)child_data.data;

	for (idx_t i = 0; i < count; i++) {
		auto list_index = list_data.sel->get_index(i);

		if (!list_data.validity.RowIsValid(list_index)) {
			result_validity.SetInvalid(i);
			continue;
		}

		const auto &list_entry = list_entries[list_index];
		std::vector<GSERIALIZED *> gserArray(list_entry.length);
		for (idx_t child_idx = 0; child_idx < list_entry.length; child_idx++) {
			auto child_value_idx = child_data.sel->get_index(list_entry.offset + child_idx);
			if (!child_data.validity.RowIsValid(child_value_idx)) {
				continue;
			}

			auto value = child_value[child_value_idx];
			if (value.GetSize() == 0) {
				continue;
			}
			auto gser = Geometry::GetGserialized(value);
			if (!gser) {
				continue;
			}
			gserArray[child_idx] = gser;
		}
		auto gserline = Geometry::MakeLineGArray(&gserArray[0], list_entry.length);
		idx_t rv_size = Geometry::GetGeometrySize(gserline);
		auto base = Geometry::GetBase(gserline);
		for (idx_t child_idx = 0; child_idx < list_entry.length; child_idx++) {
			Geometry::DestroyGeometry(gserArray[child_idx]);
		}
		Geometry::DestroyGeometry(gserline);
		result_entries[i] = string_t((const char *)base, rv_size);
	}
}

struct MakePolygonUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			// throw ConversionException(
			//     "Failure in geometry get X: could not get coordinate X from geometry");
			return string_t();
		}
		auto gser = Geometry::GetGserialized(geom);
		auto gserpoly = Geometry::MakePolygon(gser);
		idx_t rv_size = Geometry::GetGeometrySize(gserpoly);
		auto base = Geometry::GetBase(gserpoly);
		Geometry::DestroyGeometry(gser);
		Geometry::DestroyGeometry(gserpoly);
		return string_t((const char *)base, rv_size);
	}
};

template <typename TA, typename TR>
static void MakePolygonUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, MakePolygonUnaryOperator>(geom, result, count);
}

void GeoFunctions::MakePolygonFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 2) {
		Vector &geom_vector = args.data[0];
		Vector &input = args.data[1];
		auto count = args.size();
		result.SetVectorType(VectorType::CONSTANT_VECTOR);
		if (input.GetVectorType() != VectorType::CONSTANT_VECTOR) {
			result.SetVectorType(VectorType::FLAT_VECTOR);
		}

		auto result_entries = FlatVector::GetData<string_t>(result);
		auto &result_validity = FlatVector::Validity(result);

		auto list_size = ListVector::GetListSize(input);
		auto &child_vector = ListVector::GetEntry(input);

		UnifiedVectorFormat child_data;
		child_vector.ToUnifiedFormat(list_size, child_data);

		UnifiedVectorFormat geom_data;
		geom_vector.ToUnifiedFormat(count, geom_data);

		UnifiedVectorFormat list_data;
		input.ToUnifiedFormat(count, list_data);
		auto list_entries = (list_entry_t *)list_data.data;

		// not required for a comparison of nested types
		auto values = (string_t *)geom_data.data;
		auto child_value = (string_t *)child_data.data;

		for (idx_t i = 0; i < count; i++) {
			auto list_index = list_data.sel->get_index(i);
			auto value_index = geom_data.sel->get_index(i);

			if (!list_data.validity.RowIsValid(list_index) || !geom_data.validity.RowIsValid(value_index)) {
				result_validity.SetInvalid(i);
				continue;
			}

			const auto &list_entry = list_entries[list_index];
			std::vector<GSERIALIZED *> gserArray(list_entry.length);
			for (idx_t child_idx = 0; child_idx < list_entry.length; child_idx++) {
				auto child_value_idx = child_data.sel->get_index(list_entry.offset + child_idx);
				if (!child_data.validity.RowIsValid(child_value_idx)) {
					continue;
				}

				auto value = child_value[child_value_idx];
				if (value.GetSize() == 0) {
					continue;
				}
				auto gser = Geometry::GetGserialized(value);
				if (!gser) {
					continue;
				}
				gserArray[child_idx] = gser;
			}
			auto geom_value = values[value_index];
			auto gser = Geometry::GetGserialized(geom_value);
			auto gserpoly = Geometry::MakePolygon(gser, &gserArray[0], list_entry.length);
			idx_t rv_size = Geometry::GetGeometrySize(gserpoly);
			auto base = Geometry::GetBase(gserpoly);
			for (idx_t child_idx = 0; child_idx < list_entry.length; child_idx++) {
				Geometry::DestroyGeometry(gserArray[child_idx]);
			}
			Geometry::DestroyGeometry(gserpoly);
			Geometry::DestroyGeometry(gser);
			result_entries[i] = string_t((const char *)base, rv_size);
		}
		// MakePolygonBinaryExecutor<string_t, string_t>(point1_arg, result, args.size());
	} else {
		MakePolygonUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
	}
}

struct AsBinaryUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom, Vector &result) {
		if (geom.GetSize() == 0) {
			return geom;
		}
		auto gser = Geometry::GetGserialized(geom);
		auto binary = Geometry::AsBinary(gser);
		auto result_str = StringVector::EmptyString(result, binary->size);
		memcpy(result_str.GetDataWriteable(), binary->data, binary->size);
		result_str.Finalize();
		return result_str;
	}
};

static string_t AsBinaryScalarFunction(Vector &result, string_t geom, string_t text) {
	if (geom.GetSize() == 0 || text.GetSize() == 0) {
		return geom;
	}
	auto gser = Geometry::GetGserialized(geom);
	auto binary = Geometry::AsBinary(gser, text.GetString());
	auto result_str = StringVector::EmptyString(result, binary->size);
	memcpy(result_str.GetDataWriteable(), binary->data, binary->size);
	result_str.Finalize();
	return result_str;
}

template <typename TA, typename TR>
static void GeometryAsBinaryUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, AsBinaryUnaryOperator>(geom, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryAsBinaryBinaryExecutor(Vector &geom, Vector &text, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom, text, result, count, [&](TA value, TB text_val) {
		return AsBinaryScalarFunction(result, value, text_val);
	});
}

void GeoFunctions::GeometryAsBinaryFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 2) {
		auto &text_arg = args.data[1];
		GeometryAsBinaryBinaryExecutor<string_t, string_t, string_t>(geom_arg, text_arg, result, args.size());
	} else {
		GeometryAsBinaryUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
	}
}

struct AsTextUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA text, Vector &result) {
		if (text.GetSize() == 0) {
			return text;
		}
		string str = Geometry::AsText((data_ptr_t)text.GetDataUnsafe(), text.GetSize());
		auto result_str = StringVector::EmptyString(result, str.size());
		memcpy(result_str.GetDataWriteable(), str.c_str(), str.size());
		result_str.Finalize();
		return result_str;
	}
};

static string_t AsTextScalarFunction(Vector &result, string_t text, size_t max_digits) {
	if (text.GetSize() == 0) {
		return text;
	}
	string str = Geometry::AsText((data_ptr_t)text.GetDataUnsafe(), text.GetSize(), max_digits);
	auto result_str = StringVector::EmptyString(result, str.size());
	memcpy(result_str.GetDataWriteable(), str.c_str(), str.size());
	result_str.Finalize();
	return result_str;
}

template <typename TA, typename TR>
static void GeometryAsTextUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, AsTextUnaryOperator>(text, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryAsTextBinaryExecutor(Vector &text, Vector &max_digits, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(text, max_digits, result, count, [&](TA value, TB m_digits) {
		return AsTextScalarFunction(result, value, m_digits);
	});
}

void GeoFunctions::GeometryAsTextFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &text_arg = args.data[0];
	if (args.data.size() == 2) {
		auto &max_digit_arg = args.data[1];
		GeometryAsTextBinaryExecutor<string_t, int, string_t>(text_arg, max_digit_arg, result, args.size());
	} else {
		GeometryAsTextUnaryExecutor<string_t, string_t>(text_arg, result, args.size());
	}
}

struct AsGeojsonUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom, Vector &result) {
		if (geom.GetSize() == 0) {
			return geom;
		}
		auto gser = Geometry::GetGserialized(geom);
		auto geojson = Geometry::AsGeoJson(gser);
		std::string geoText = std::string(geojson->data);
		auto result_str = StringVector::EmptyString(result, geoText.size());
		memcpy(result_str.GetDataWriteable(), geoText.c_str(), geoText.size());
		result_str.Finalize();
		return result_str;
	}
};

struct AsGeojsonBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom, TB m_dec_digits) {
		if (geom.GetSize() == 0) {
			return string_t();
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry asgeojson");
		}
		auto geojson = Geometry::AsGeoJson(gser, m_dec_digits);
		std::string geoText = std::string(geojson->data);
		Geometry::DestroyGeometry(gser);
		return string_t(geoText.c_str(), geoText.size());
	}
};

template <typename TA, typename TR>
static void GeometryAsGeojsonUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, AsGeojsonUnaryOperator>(text, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryAsGeojsonBinaryExecutor(Vector &geom, Vector &m_dec_digits, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, AsGeojsonBinaryOperator>(geom, m_dec_digits, result, count);
}

void GeoFunctions::GeometryAsGeojsonFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryAsGeojsonUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &max_dec_digits_arg = args.data[1];
		GeometryAsGeojsonBinaryExecutor<string_t, int, string_t>(geom_arg, max_dec_digits_arg, result, args.size());
	}
}

struct GeoHashUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom, Vector &result) {
		if (geom.GetSize() == 0) {
			return geom;
		}
		auto gser = Geometry::GetGserialized(geom);
		auto geojson = Geometry::GeoHash(gser);
		std::string geoText = std::string(geojson->data);
		auto result_str = StringVector::EmptyString(result, geoText.size());
		memcpy(result_str.GetDataWriteable(), geoText.c_str(), geoText.size());
		result_str.Finalize();
		return result_str;
	}
};

struct GeoHashBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom, TB m_chars) {
		if (geom.GetSize() == 0) {
			return string_t();
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry geohash");
		}
		auto geojson = Geometry::GeoHash(gser, m_chars);
		std::string geoText = std::string(geojson->data);
		Geometry::DestroyGeometry(gser);
		return string_t(geoText.c_str(), geoText.size());
	}
};

template <typename TA, typename TR>
static void GeometryGeoHashUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, GeoHashUnaryOperator>(text, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryGeoHashBinaryExecutor(Vector &geom, Vector &m_chars, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, GeoHashBinaryOperator>(geom, m_chars, result, count);
}

void GeoFunctions::GeometryGeoHashFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryGeoHashUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &maxchars_arg = args.data[1];
		GeometryGeoHashBinaryExecutor<string_t, int, string_t>(geom_arg, maxchars_arg, result, args.size());
	}
}

struct GeometryDistanceBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		double dis = 0.00;
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return dis;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			throw ConversionException("Failure in geometry distance: could not calculate distance from geometries");
		}
		dis = Geometry::Distance(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return dis;
	}
};

struct GeometryDistanceTernaryOperator {
	template <class TA, class TB, class TC, class TR>
	static inline TR Operation(TA geom1, TB geom2, TC use_spheroid) {
		double dis = 0.00;
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return dis;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			throw ConversionException("Failure in geometry distance: could not calculate distance from geometries");
		}
		dis = Geometry::Distance(gser1, gser2, use_spheroid);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return dis;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryDistanceBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, GeometryDistanceBinaryOperator>(geom1, geom2, result, count);
}

template <typename TA, typename TB, typename TC, typename TR>
static void GeometryDistanceTernaryExecutor(Vector &geom1, Vector &geom2, Vector &use_spheroid, Vector &result,
                                            idx_t count) {
	TernaryExecutor::Execute<TA, TB, TC, TR>(geom1, geom2, use_spheroid, result, count,
	                                         GeometryDistanceTernaryOperator::Operation<TA, TB, TC, TR>);
}

void GeoFunctions::GeometryDistanceFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	if (args.data.size() == 2) {
		GeometryDistanceBinaryExecutor<string_t, string_t, double>(geom1_arg, geom2_arg, result, args.size());
	} else if (args.data.size() == 3) {
		auto &use_spheroid_arg = args.data[2];
		GeometryDistanceTernaryExecutor<string_t, string_t, bool, double>(geom1_arg, geom2_arg, use_spheroid_arg,
		                                                                  result, args.size());
	}
}

struct CentroidUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		return geom;
		// if (geom.GetSize() == 0) {
		// 	return NULL;
		// }
		// auto gser = Geometry::GetGserialized(geom);
		// if (!gser) {
		// 	throw ConversionException("Failure in geometry centroid: could not calculate centroid from geometry");
		// }
		// auto result = Geometry::Centroid(gser);
		// idx_t rv_size = Geometry::GetGeometrySize(result);
		// auto base = Geometry::GetBase(result);
		// Geometry::DestroyGeometry(result);
		// return string_t((const char *)base, rv_size);
	}
};

struct CentroidBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom, TB use_spheroid) {
		return geom;
		// if (geom.GetSize() == 0) {
		// 	return NULL;
		// }
		// auto gser = Geometry::GetGserialized(geom);
		// if (!gser) {
		// 	throw ConversionException("Failure in geometry centroid: could not calculate centroid from geometry");
		// }
		// auto result = Geometry::Centroid(gser, use_spheroid);
		// idx_t rv_size = Geometry::GetGeometrySize(result);
		// auto base = Geometry::GetBase(result);
		// Geometry::DestroyGeometry(result);
		// return string_t((const char *)base, rv_size);
	}
};

template <typename TA, typename TR>
static void GeometryCentroidUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, CentroidUnaryOperator>(geom, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryCentroidBinaryExecutor(Vector &geom, Vector &use_spheroid, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, CentroidBinaryOperator>(geom, use_spheroid, result, count);
}

void GeoFunctions::GeometryCentroidFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryCentroidUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &use_spheroid_arg = args.data[1];
		GeometryCentroidBinaryExecutor<string_t, bool, string_t>(geom_arg, use_spheroid_arg, result, args.size());
	}
}

struct FromTextUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA text) {
		if (text.GetSize() == 0) {
			return text;
		}
		auto gser = Geometry::FromText(&text.GetString()[0]);
		if (!gser) {
			throw ConversionException("Failure in geometry from text: could not convert text to geometry");
		}
		idx_t size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, size);
	}
};

struct FromTextBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA text, TB srid) {
		if (text.GetSize() == 0) {
			return text;
		}
		auto gser = Geometry::FromText(&text.GetString()[0], srid);
		if (!gser) {
			throw ConversionException("Failure in geometry from text: could not convert text to geometry");
		}
		idx_t size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, size);
	}
};

template <typename TA, typename TR>
static void GeometryFromTextUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, FromTextUnaryOperator>(text, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryFromTextBinaryExecutor(Vector &text, Vector &srid, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, FromTextBinaryOperator>(text, srid, result, count);
}

void GeoFunctions::GeometryFromTextFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &text_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryFromTextUnaryExecutor<string_t, string_t>(text_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &srid_arg = args.data[1];
		GeometryFromTextBinaryExecutor<string_t, int32_t, string_t>(text_arg, srid_arg, result, args.size());
	}
}

struct FromWKBUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA text) {
		if (text.GetSize() == 0) {
			return text;
		}
		auto gser = Geometry::FromWKB(text.GetDataUnsafe(), text.GetSize());
		if (!gser) {
			throw ConversionException("Failure in geometry from WKB: could not convert WKB to geometry");
		}
		idx_t size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, size);
	}
};

struct FromWKBBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA text, TB srid) {
		if (text.GetSize() == 0) {
			return text;
		}
		auto gser = Geometry::FromWKB(text.GetDataUnsafe(), text.GetSize(), srid);
		if (!gser) {
			throw ConversionException("Failure in geometry from WKB: could not convert WKB to geometry");
		}
		idx_t size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, size);
	}
};

template <typename TA, typename TR>
static void GeometryFromWKBUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, FromWKBUnaryOperator>(text, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryFromWKBBinaryExecutor(Vector &text, Vector &srid, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, FromWKBBinaryOperator>(text, srid, result, count);
}

void GeoFunctions::GeometryFromWKBFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &text_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryFromWKBUnaryExecutor<string_t, string_t>(text_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &srid_arg = args.data[1];
		GeometryFromWKBBinaryExecutor<string_t, int32_t, string_t>(text_arg, srid_arg, result, args.size());
	}
}

struct GetXUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA text) {
		if (text.GetSize() == 0) {
			// throw ConversionException(
			//     "Failure in geometry get X: could not get coordinate X from geometry");
			return 0.00;
		}
		double x_val = Geometry::XPoint(text.GetDataUnsafe(), text.GetSize());
		return x_val;
	}
};

template <typename TA, typename TR>
static void GeometryGetXUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, GetXUnaryOperator>(text, result, count);
}

void GeoFunctions::GeometryGetXFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &text_arg = args.data[0];
	GeometryGetXUnaryExecutor<string_t, double>(text_arg, result, args.size());
}

} // namespace duckdb
