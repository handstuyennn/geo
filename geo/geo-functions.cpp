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
		Geometry::DestroyGeometry(gser);
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
	Geometry::DestroyGeometry(gser);
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
	static inline TR Operation(TA geom, Vector &result) {
		if (geom.GetSize() == 0) {
			return geom;
		}
		auto gser = Geometry::GetGserialized(geom);
		auto text = Geometry::AsText(gser);
		auto result_str = StringVector::EmptyString(result, text.size());
		memcpy(result_str.GetDataWriteable(), text.c_str(), text.size());
		result_str.Finalize();
		Geometry::DestroyGeometry(gser);
		return result_str;
	}
};

static string_t AsTextScalarFunction(Vector &result, string_t geom, size_t max_digits) {
	if (geom.GetSize() == 0) {
		return geom;
	}
	auto gser = Geometry::GetGserialized(geom);
	auto str = Geometry::AsText(gser, max_digits);
	auto result_str = StringVector::EmptyString(result, str.size());
	memcpy(result_str.GetDataWriteable(), str.c_str(), str.size());
	result_str.Finalize();
	Geometry::DestroyGeometry(gser);
	return result_str;
}

template <typename TA, typename TR>
static void GeometryAsTextUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, AsTextUnaryOperator>(geom, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryAsTextBinaryExecutor(Vector &text, Vector &max_digits, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(text, max_digits, result, count, [&](TA value, TB m_digits) {
		return AsTextScalarFunction(result, value, m_digits);
	});
}

void GeoFunctions::GeometryAsTextFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 2) {
		auto &max_digit_arg = args.data[1];
		GeometryAsTextBinaryExecutor<string_t, int, string_t>(geom_arg, max_digit_arg, result, args.size());
	} else {
		GeometryAsTextUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
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
		Geometry::DestroyGeometry(gser);
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

struct GeogFromUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA text, Vector &result) {
		if (text.GetSize() == 0) {
			return string_t();
		}
		auto gser = Geometry::ToGserialized(text);
		idx_t size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		auto result_str = StringVector::EmptyString(result, size);
		memcpy(result_str.GetDataWriteable(), base, size);
		result_str.Finalize();
		Geometry::DestroyGeometry(gser);
		return result_str;
	}
};

template <typename TA, typename TR>
static void GeometryGeogFromUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, GeogFromUnaryOperator>(text, result, count);
}

void GeoFunctions::GeometryGeogFromFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &text_arg = args.data[0];
	GeometryGeogFromUnaryExecutor<string_t, string_t>(text_arg, result, args.size());
}

struct GeomFromGeoJsonUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA text, Vector &result) {
		if (text.GetSize() == 0) {
			return string_t();
		}
		auto gser = Geometry::GeomFromGeoJson(text);
		if (!gser) {
			throw ConversionException("Failure in geometry from Json: could not convert JSON to geometry");
		}
		idx_t size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		auto result_str = StringVector::EmptyString(result, size);
		memcpy(result_str.GetDataWriteable(), base, size);
		result_str.Finalize();
		Geometry::DestroyGeometry(gser);
		return result_str;
	}
};

template <typename TA, typename TR>
static void GeometryGeomFromGeoJsonUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, GeomFromGeoJsonUnaryOperator>(text, result, count);
}

void GeoFunctions::GeometryGeomFromGeoJsonFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &text_arg = args.data[0];
	GeometryGeomFromGeoJsonUnaryExecutor<string_t, string_t>(text_arg, result, args.size());
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
	static inline TR Operation(TA geom, Vector &result) {
		if (geom.GetSize() == 0) {
			return NULL;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry centroid: could not calculate centroid from geometry");
		}
		auto gserCentroid = Geometry::Centroid(gser);
		idx_t rv_size = Geometry::GetGeometrySize(gserCentroid);
		auto base = Geometry::GetBase(gserCentroid);
		auto result_str = StringVector::EmptyString(result, rv_size);
		memcpy(result_str.GetDataWriteable(), base, rv_size);
		result_str.Finalize();
		Geometry::DestroyGeometry(gser);
		Geometry::DestroyGeometry(gserCentroid);
		return result_str;
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
	UnaryExecutor::ExecuteString<TA, TR, CentroidUnaryOperator>(geom, result, count);
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

struct FromGeoHashUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA text) {
		if (text.GetSize() == 0) {
			return text;
		}
		auto gser = Geometry::FromGeoHash(text);
		if (!gser) {
			throw ConversionException("Failure in geometry from geo hash: could not convert geo hash to geometry");
		}
		idx_t size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, size);
	}
};

struct FromGeoHashBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA text, TB precision) {
		if (text.GetSize() == 0) {
			return text;
		}
		auto gser = Geometry::FromGeoHash(text, precision);
		if (!gser) {
			throw ConversionException("Failure in geometry from geo hash: could not convert geo hash to geometry");
		}
		idx_t size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		return string_t((const char *)base, size);
	}
};

template <typename TA, typename TR>
static void GeometryFromGeoHashUnaryExecutor(Vector &text, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, FromGeoHashUnaryOperator>(text, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryFromGeoHashBinaryExecutor(Vector &text, Vector &precision, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, FromGeoHashBinaryOperator>(text, precision, result, count);
}

void GeoFunctions::GeometryFromGeoHashFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &text_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryFromGeoHashUnaryExecutor<string_t, string_t>(text_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &precision_arg = args.data[1];
		GeometryFromGeoHashBinaryExecutor<string_t, int32_t, string_t>(text_arg, precision_arg, result, args.size());
	}
}

struct BoundaryUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			return geom;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			return string_t();
		}
		auto gserBoundary = Geometry::LWGEOM_boundary(gser);
		if (!gserBoundary) {
			throw ConversionException("Failure in geometry boundary: could not getting boundary from geom");
		}
		idx_t size = Geometry::GetGeometrySize(gserBoundary);
		auto base = Geometry::GetBase(gserBoundary);
		Geometry::DestroyGeometry(gser);
		Geometry::DestroyGeometry(gserBoundary);
		return string_t((const char *)base, size);
	}
};

template <typename TA, typename TR>
static void GeometryBoundaryUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, BoundaryUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryBoundaryFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryBoundaryUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
}

struct DimensionUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			return -1;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry dimension: could not getting dimension from geom");
			return -1;
		}
		auto dimension = Geometry::LWGEOM_dimension(gser);
		Geometry::DestroyGeometry(gser);
		return dimension;
	}
};

template <typename TA, typename TR>
static void GeometryDimensionUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, DimensionUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryDimensionFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryDimensionUnaryExecutor<string_t, int>(geom_arg, result, args.size());
}

void GeoFunctions::GeometryDumpFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	D_ASSERT(args.GetTypes().size() == 1);
	auto &geom_arg = args.data[0];
	auto child_type = ListType::GetChildType(result.GetType());

	auto geom = args.GetValue(0, 0).GetValueUnsafe<string_t>();

	auto gser = Geometry::GetGserialized(geom);
	auto gserArray = Geometry::LWGEOM_dump(gser);

	vector<Value> geom_values;
	for (idx_t i = 0; i < gserArray.size(); i++) {
		auto gserChild = gserArray[i];
		idx_t rv_size = Geometry::GetGeometrySize(gserChild);
		auto base = Geometry::GetBase(gserChild);
		Geometry::DestroyGeometry(gserChild);
		auto value = Value::BLOB((const_data_ptr_t)base, rv_size);
		value.type().CopyAuxInfo(child_type);
		geom_values.emplace_back(value);
	}
	auto val = Value::LIST(child_type, geom_values);
	result.Reference(val);
	Geometry::DestroyGeometry(gser);
}

struct EndPointUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			return string_t();
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry dimension: could not getting dimension from geom");
			return string_t();
		}
		auto gserEndpoint = Geometry::LWGEOM_endpoint_linestring(gser);
		idx_t size = Geometry::GetGeometrySize(gserEndpoint);
		auto base = Geometry::GetBase(gserEndpoint);
		Geometry::DestroyGeometry(gser);
		Geometry::DestroyGeometry(gserEndpoint);
		return string_t((const char *)base, size);
	}
};

template <typename TA, typename TR>
static void GeometryEndPointUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, EndPointUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryEndPointFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryEndPointUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
}

struct TypeUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom, Vector &result) {
		if (geom.GetSize() == 0) {
			return string_t();
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry dimension: could not getting dimension from geom");
			return string_t();
		}
		auto geometrytype = Geometry::Geometrytype(gser);
		auto rv_size = geometrytype.size();
		auto result_str = StringVector::EmptyString(result, rv_size);
		memcpy(result_str.GetDataWriteable(), geometrytype.c_str(), rv_size);
		result_str.Finalize();
		Geometry::DestroyGeometry(gser);
		return result_str;
	}
};

template <typename TA, typename TR>
static void GeometryTypeUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, TypeUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryTypeFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryTypeUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
}

struct IsClosedUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom) {
		if (geom.GetSize() == 0) {
			return false;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry is closed: could not getting closed from geom");
			return false;
		}
		auto isClosed = Geometry::IsClosed(gser);
		Geometry::DestroyGeometry(gser);
		return isClosed;
	}
};

template <typename TA, typename TR>
static void GeometryIsClosedUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, IsClosedUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryIsClosedFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryIsClosedUnaryExecutor<string_t, bool>(geom_arg, result, args.size());
}

struct IsCollectionUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom) {
		if (geom.GetSize() == 0) {
			return false;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry is collection: could not getting collection from geom");
			return false;
		}
		auto isCollection = Geometry::IsCollection(gser);
		Geometry::DestroyGeometry(gser);
		return isCollection;
	}
};

template <typename TA, typename TR>
static void GeometryIsCollectionUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, IsCollectionUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryIsCollectionFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryIsCollectionUnaryExecutor<string_t, bool>(geom_arg, result, args.size());
}

struct IsEmptyUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom) {
		if (geom.GetSize() == 0) {
			return true;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry is empty: could not getting empty from geom");
			return true;
		}
		auto isEmpty = Geometry::IsEmpty(gser);
		Geometry::DestroyGeometry(gser);
		return isEmpty;
	}
};

template <typename TA, typename TR>
static void GeometryIsEmptyUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, IsEmptyUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryIsEmptyFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryIsEmptyUnaryExecutor<string_t, bool>(geom_arg, result, args.size());
}

struct IsRingUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom) {
		if (geom.GetSize() == 0) {
			return true;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry is ring: could not getting ring from geom");
			return true;
		}
		auto isRing = Geometry::IsRing(gser);
		Geometry::DestroyGeometry(gser);
		return isRing;
	}
};

template <typename TA, typename TR>
static void GeometryIsRingUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, IsRingUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryIsRingFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryIsRingUnaryExecutor<string_t, bool>(geom_arg, result, args.size());
}

struct NPointsUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom) {
		if (geom.GetSize() == 0) {
			return true;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry is ring: could not getting ring from geom");
			return true;
		}
		auto nPoints = Geometry::NPoints(gser);
		Geometry::DestroyGeometry(gser);
		return nPoints;
	}
};

template <typename TA, typename TR>
static void GeometryNPointsUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, NPointsUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryNPointsFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryNPointsUnaryExecutor<string_t, int>(geom_arg, result, args.size());
}

struct NumGeometriesUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom) {
		if (geom.GetSize() == 0) {
			return true;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry is ring: could not getting ring from geom");
			return true;
		}
		auto numGeometries = Geometry::NumGeometries(gser);
		Geometry::DestroyGeometry(gser);
		return numGeometries;
	}
};

template <typename TA, typename TR>
static void GeometryNumGeometriesUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, NumGeometriesUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryNumGeometriesFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryNumGeometriesUnaryExecutor<string_t, int>(geom_arg, result, args.size());
}

struct NumPointsUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom) {
		if (geom.GetSize() == 0) {
			return true;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry is ring: could not getting ring from geom");
			return true;
		}
		auto numGeometries = Geometry::NumPoints(gser);
		Geometry::DestroyGeometry(gser);
		return numGeometries;
	}
};

template <typename TA, typename TR>
static void GeometryNumPointsUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, NumPointsUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryNumPointsFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryNumPointsUnaryExecutor<string_t, int>(geom_arg, result, args.size());
}

template <typename TA, typename TB, typename TR>
static TR PointNScalarFunction(Vector &result, TA geom, TB index) {
	if (geom.GetSize() == 0) {
		return string_t();
	}
	auto gser = Geometry::GetGserialized(geom);
	if (!gser) {
		throw ConversionException("Failure in geometry get point n: could not getting point n from geom");
		return string_t();
	}
	auto gserPointN = Geometry::PointN(gser, index);
	idx_t rv_size = Geometry::GetGeometrySize(gserPointN);
	auto base = Geometry::GetBase(gserPointN);
	auto result_str = StringVector::EmptyString(result, rv_size);
	memcpy(result_str.GetDataWriteable(), base, rv_size);
	result_str.Finalize();
	Geometry::DestroyGeometry(gser);
	Geometry::DestroyGeometry(gserPointN);
	return result_str;
}

template <typename TA, typename TB, typename TR>
static void GeometryPointNBinaryExecutor(Vector &geom_vec, Vector &index_vec, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom_vec, index_vec, result, count, [&](TA geom, TB index) {
		return PointNScalarFunction<TA, TB, TR>(result, geom, index);
	});
}

void GeoFunctions::GeometryPointNFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	auto &index_arg = args.data[1];
	GeometryPointNBinaryExecutor<string_t, int, string_t>(geom_arg, index_arg, result, args.size());
}

struct StartPointUnaryOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE geom, Vector &result) {
		if (geom.GetSize() == 0) {
			return string_t();
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry gets start point: could not getting start point from geom");
			return string_t();
		}
		auto gserStartPoint = Geometry::StartPoint(gser);
		idx_t rv_size = Geometry::GetGeometrySize(gserStartPoint);
		auto base = Geometry::GetBase(gserStartPoint);
		auto result_str = StringVector::EmptyString(result, rv_size);
		memcpy(result_str.GetDataWriteable(), base, rv_size);
		result_str.Finalize();
		Geometry::DestroyGeometry(gser);
		Geometry::DestroyGeometry(gserStartPoint);
		return result_str;
	}
};

template <typename TA, typename TR>
static void GeometryStartPointUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, StartPointUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryStartPointFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryStartPointUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
}

struct GetXUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			// throw ConversionException(
			//     "Failure in geometry get X: could not get coordinate X from geometry");
			return 0.00;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry gets X: could not getting X from geom");
			return 0.00;
		}
		double x_val = Geometry::XPoint(gser);
		Geometry::DestroyGeometry(gser);
		return x_val;
	}
};

template <typename TA, typename TR>
static void GeometryGetXUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, GetXUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryGetXFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryGetXUnaryExecutor<string_t, double>(geom_arg, result, args.size());
}

struct GetYUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			// throw ConversionException(
			//     "Failure in geometry get X: could not get coordinate X from geometry");
			return 0.00;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry gets Y: could not getting Y from geom");
			return 0.00;
		}
		double y_val = Geometry::YPoint(gser);
		Geometry::DestroyGeometry(gser);
		return y_val;
	}
};

template <typename TA, typename TR>
static void GeometryGetYUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, GetYUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryGetYFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryGetYUnaryExecutor<string_t, double>(geom_arg, result, args.size());
}

template <typename TA, typename TB, typename TR>
static TR DifferenceScalarFunction(Vector &result, TA geom1, TB geom2) {
	if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
		return string_t();
	}
	if (geom1.GetSize() == 0) {
		return geom2;
	} else if (geom2.GetSize() == 0) {
		return geom1;
	}
	auto gser1 = Geometry::GetGserialized(geom1);
	auto gser2 = Geometry::GetGserialized(geom2);
	if (!gser1 || !gser2) {
		throw ConversionException("Failure in geometry get difference: could not getting difference from geom");
		return string_t();
	}
	auto gserDiff = Geometry::Difference(gser1, gser2);
	idx_t rv_size = Geometry::GetGeometrySize(gserDiff);
	auto base = Geometry::GetBase(gserDiff);
	auto result_str = StringVector::EmptyString(result, rv_size);
	memcpy(result_str.GetDataWriteable(), base, rv_size);
	result_str.Finalize();
	Geometry::DestroyGeometry(gser1);
	Geometry::DestroyGeometry(gser2);
	Geometry::DestroyGeometry(gserDiff);
	return result_str;
}

template <typename TA, typename TB, typename TR>
static void GeometryDifferenceBinaryExecutor(Vector &geom1_vec, Vector &geom2_vec, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom1_vec, geom2_vec, result, count, [&](TA geom1, TB geom2) {
		return DifferenceScalarFunction<TA, TB, TR>(result, geom1, geom2);
	});
}

void GeoFunctions::GeometryDifferenceFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryDifferenceBinaryExecutor<string_t, string_t, string_t>(geom1_arg, geom2_arg, result, args.size());
}

template <typename TA, typename TB, typename TR>
static TR ClosestPointScalarFunction(Vector &result, TA geom1, TB geom2) {
	if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
		return string_t();
	}
	auto gser1 = Geometry::GetGserialized(geom1);
	auto gser2 = Geometry::GetGserialized(geom2);
	if (!gser1 || !gser2) {
		throw ConversionException("Failure in geometry get closest point: could not getting closest point from geom");
		return string_t();
	}
	auto gserClosestPoint = Geometry::ClosestPoint(gser1, gser2);
	idx_t rv_size = Geometry::GetGeometrySize(gserClosestPoint);
	auto base = Geometry::GetBase(gserClosestPoint);
	auto result_str = StringVector::EmptyString(result, rv_size);
	memcpy(result_str.GetDataWriteable(), base, rv_size);
	result_str.Finalize();
	Geometry::DestroyGeometry(gser1);
	Geometry::DestroyGeometry(gser2);
	Geometry::DestroyGeometry(gserClosestPoint);
	return result_str;
}

template <typename TA, typename TB, typename TR>
static void GeometryClosestPointBinaryExecutor(Vector &geom1_vec, Vector &geom2_vec, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom1_vec, geom2_vec, result, count, [&](TA geom1, TB geom2) {
		return ClosestPointScalarFunction<TA, TB, TR>(result, geom1, geom2);
	});
}

void GeoFunctions::GeometryClosestPointFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryClosestPointBinaryExecutor<string_t, string_t, string_t>(geom1_arg, geom2_arg, result, args.size());
}

template <typename TA, typename TB, typename TR>
static TR UnionScalarFunction(Vector &result, TA geom1, TB geom2) {
	if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
		return string_t();
	}
	auto gser1 = Geometry::GetGserialized(geom1);
	auto gser2 = Geometry::GetGserialized(geom2);
	if (!gser1 || !gser2) {
		throw ConversionException("Failure in geometry get union: could not getting union from geom");
		return string_t();
	}
	auto gserUnion = Geometry::GeometryUnion(gser1, gser2);
	idx_t rv_size = Geometry::GetGeometrySize(gserUnion);
	auto base = Geometry::GetBase(gserUnion);
	auto result_str = StringVector::EmptyString(result, rv_size);
	memcpy(result_str.GetDataWriteable(), base, rv_size);
	result_str.Finalize();
	Geometry::DestroyGeometry(gser1);
	Geometry::DestroyGeometry(gser2);
	Geometry::DestroyGeometry(gserUnion);
	return result_str;
}

template <typename TA, typename TB, typename TR>
static void GeometryUnionBinaryExecutor(Vector &geom1_vec, Vector &geom2_vec, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom1_vec, geom2_vec, result, count, [&](TA geom1, TB geom2) {
		return UnionScalarFunction<TA, TB, TR>(result, geom1, geom2);
	});
}

void GeoFunctions::GeometryUnionFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryUnionBinaryExecutor<string_t, string_t, string_t>(geom1_arg, geom2_arg, result, args.size());
}

void GeoFunctions::GeometryUnionArrayFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
		auto gsergeom = Geometry::GeometryUnionGArray(&gserArray[0], list_entry.length);
		idx_t rv_size = Geometry::GetGeometrySize(gsergeom);
		auto base = Geometry::GetBase(gsergeom);
		for (idx_t child_idx = 0; child_idx < list_entry.length; child_idx++) {
			Geometry::DestroyGeometry(gserArray[child_idx]);
		}
		Geometry::DestroyGeometry(gsergeom);
		result_entries[i] = string_t((const char *)base, rv_size);
	}
}

template <typename TA, typename TB, typename TR>
static TR IntersectionScalarFunction(Vector &result, TA geom1, TB geom2) {
	if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
		return string_t();
	}
	auto gser1 = Geometry::GetGserialized(geom1);
	auto gser2 = Geometry::GetGserialized(geom2);
	if (!gser1 || !gser2) {
		throw ConversionException("Failure in geometry get intersection: could not getting intersection from geom");
		return string_t();
	}
	auto gserIntersection = Geometry::GeometryIntersection(gser1, gser2);
	idx_t rv_size = Geometry::GetGeometrySize(gserIntersection);
	auto base = Geometry::GetBase(gserIntersection);
	auto result_str = StringVector::EmptyString(result, rv_size);
	memcpy(result_str.GetDataWriteable(), base, rv_size);
	result_str.Finalize();
	Geometry::DestroyGeometry(gser1);
	Geometry::DestroyGeometry(gser2);
	Geometry::DestroyGeometry(gserIntersection);
	return result_str;
}

template <typename TA, typename TB, typename TR>
static void GeometryIntersectionBinaryExecutor(Vector &geom1_vec, Vector &geom2_vec, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom1_vec, geom2_vec, result, count, [&](TA geom1, TB geom2) {
		return IntersectionScalarFunction<TA, TB, TR>(result, geom1, geom2);
	});
}

void GeoFunctions::GeometryIntersectionFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryIntersectionBinaryExecutor<string_t, string_t, string_t>(geom1_arg, geom2_arg, result, args.size());
}

template <typename TA, typename TB, typename TR>
static TR SimplifyScalarFunction(Vector &result, TA geom, TB dist) {
	if (geom.GetSize() == 0) {
		return string_t();
	}
	auto gser = Geometry::GetGserialized(geom);
	if (!gser) {
		throw ConversionException("Failure in geometry get simplify: could not getting simplify from geom");
		return string_t();
	}
	auto gserSimplify = Geometry::GeometrySimplify(gser, dist);
	if (!gserSimplify) {
		Geometry::DestroyGeometry(gser);
		return string_t();
	}
	if (gser == gserSimplify) {
		Geometry::DestroyGeometry(gser);
		return geom;
	}
	idx_t rv_size = Geometry::GetGeometrySize(gserSimplify);
	auto base = Geometry::GetBase(gserSimplify);
	auto result_str = StringVector::EmptyString(result, rv_size);
	memcpy(result_str.GetDataWriteable(), base, rv_size);
	result_str.Finalize();
	Geometry::DestroyGeometry(gser);
	Geometry::DestroyGeometry(gserSimplify);
	return result_str;
}

template <typename TA, typename TB, typename TR>
static void GeometrySimplifyBinaryExecutor(Vector &geom_vec, Vector &dist_vec, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom_vec, dist_vec, result, count, [&](TA geom, TB dist) {
		return SimplifyScalarFunction<TA, TB, TR>(result, geom, dist);
	});
}

void GeoFunctions::GeometrySimplifyFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	auto &dist_arg = args.data[1];
	GeometrySimplifyBinaryExecutor<string_t, double, string_t>(geom_arg, dist_arg, result, args.size());
}

struct ConvexhullUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			return geom;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			return string_t();
		}
		auto gserConvex = Geometry::Convexhull(gser);
		if (!gserConvex) {
			throw ConversionException("Failure in geometry convex hull: could not getting convex hull from geom");
		}
		if (gser == gserConvex) {
			Geometry::DestroyGeometry(gser);
			return string_t();
		}
		idx_t size = Geometry::GetGeometrySize(gserConvex);
		auto base = Geometry::GetBase(gserConvex);
		Geometry::DestroyGeometry(gser);
		Geometry::DestroyGeometry(gserConvex);
		return string_t((const char *)base, size);
	}
};

template <typename TA, typename TR>
static void GeometryConvexhullUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, ConvexhullUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryConvexhullFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryConvexhullUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
}

template <typename TA, typename TB, typename TR>
static TR SnapToGridScalarFunction(Vector &result, TA geom, TB size) {
	if (geom.GetSize() == 0) {
		return string_t();
	}
	auto gser = Geometry::GetGserialized(geom);
	if (!gser) {
		throw ConversionException("Failure in geometry get snap to grid: could not getting snap to grid from geom");
		return string_t();
	}
	auto gserSnapTogrid = Geometry::GeometrySnapToGrid(gser, size);
	if (!gserSnapTogrid) {
		Geometry::DestroyGeometry(gser);
		return string_t();
	}
	if (gser == gserSnapTogrid) {
		Geometry::DestroyGeometry(gser);
		return geom;
	}
	idx_t rv_size = Geometry::GetGeometrySize(gserSnapTogrid);
	auto base = Geometry::GetBase(gserSnapTogrid);
	auto result_str = StringVector::EmptyString(result, rv_size);
	memcpy(result_str.GetDataWriteable(), base, rv_size);
	result_str.Finalize();
	Geometry::DestroyGeometry(gser);
	Geometry::DestroyGeometry(gserSnapTogrid);
	return result_str;
}

template <typename TA, typename TB, typename TR>
static void GeometrySnapToGridBinaryExecutor(Vector &geom_vec, Vector &size_vec, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom_vec, size_vec, result, count, [&](TA geom, TB size) {
		return SnapToGridScalarFunction<TA, TB, TR>(result, geom, size);
	});
}

void GeoFunctions::GeometrySnapToGridFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	auto &size_arg = args.data[1];
	GeometrySnapToGridBinaryExecutor<string_t, double, string_t>(geom_arg, size_arg, result, args.size());
}

template <typename TA, typename TB, typename TR>
static TR BufferScalarFunction(Vector &result, TA geom, TB radius) {
	if (geom.GetSize() == 0) {
		return string_t();
	}
	auto gser = Geometry::GetGserialized(geom);
	if (!gser) {
		throw ConversionException("Failure in geometry get buffer: could not getting buffer from geom");
		return string_t();
	}
	auto gserBuffer = Geometry::GeometryBuffer(gser, radius);
	if (!gserBuffer) {
		Geometry::DestroyGeometry(gser);
		return string_t();
	}
	if (gser == gserBuffer) {
		Geometry::DestroyGeometry(gser);
		return geom;
	}
	idx_t rv_size = Geometry::GetGeometrySize(gserBuffer);
	auto base = Geometry::GetBase(gserBuffer);
	auto result_str = StringVector::EmptyString(result, rv_size);
	memcpy(result_str.GetDataWriteable(), base, rv_size);
	result_str.Finalize();
	Geometry::DestroyGeometry(gser);
	Geometry::DestroyGeometry(gserBuffer);
	return result_str;
}

template <typename TA, typename TB, typename TR>
static void GeometryBufferBinaryExecutor(Vector &geom_vec, Vector &radius_vec, Vector &result, idx_t count) {
	BinaryExecutor::Execute<TA, TB, TR>(geom_vec, radius_vec, result, count, [&](TA geom, TB radius) {
		return BufferScalarFunction<TA, TB, TR>(result, geom, radius);
	});
}

void GeoFunctions::GeometryBufferFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	auto &radius_arg = args.data[1];
	GeometryBufferBinaryExecutor<string_t, double, string_t>(geom_arg, radius_arg, result, args.size());
}

struct BufferTextTernaryOperator {
	template <class TA, class TB, class TC, class TR>
	static inline TR Operation(TA geom, TB radius, TC styles) {
		if (geom.GetSize() == 0) {
			return string_t();
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry get buffer: could not getting buffer from geom");
			return string_t();
		}
		auto gserBuffer = Geometry::GeometryBufferText(gser, radius, styles.GetString());
		if (!gserBuffer) {
			Geometry::DestroyGeometry(gser);
			return string_t();
		}
		if (gser == gserBuffer) {
			Geometry::DestroyGeometry(gser);
			return geom;
		}
		idx_t rv_size = Geometry::GetGeometrySize(gser);
		auto base = Geometry::GetBase(gser);
		Geometry::DestroyGeometry(gser);
		Geometry::DestroyGeometry(gserBuffer);
		return string_t((const char *)base, rv_size);
	}
};

template <typename TA, typename TB, typename TC, typename TR>
static void BufferTextTernaryExecutor(Vector &geom, Vector &radius, Vector &styles, Vector &result, idx_t count) {
	TernaryExecutor::Execute<TA, TB, TC, TR>(geom, radius, styles, result, count,
	                                         BufferTextTernaryOperator::Operation<TA, TB, TC, TR>);
}

void GeoFunctions::GeometryBufferTextFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	auto &radius_arg = args.data[1];
	auto &styles_arg = args.data[2];
	BufferTextTernaryExecutor<string_t, double, string_t, string_t>(geom_arg, radius_arg, styles_arg, result,
	                                                                args.size());
}

struct EqualsBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get equals: could not getting equals from geom");
			return false;
		}
		auto equalsRv = Geometry::GeometryEquals(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return equalsRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryEqualsBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, EqualsBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryEqualsFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryEqualsBinaryExecutor<string_t, string_t, bool>(geom1_arg, geom2_arg, result, args.size());
}

struct ContainsBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get equals: could not getting equals from geom");
			return false;
		}
		auto equalsRv = Geometry::GeometryContains(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return equalsRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryContainsBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, ContainsBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryContainsFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryContainsBinaryExecutor<string_t, string_t, bool>(geom1_arg, geom2_arg, result, args.size());
}

struct TouchesBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get touches: could not getting touches from geom");
			return false;
		}
		auto touchesRv = Geometry::GeometryTouches(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return touchesRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryTouchesBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, TouchesBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryTouchesFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryTouchesBinaryExecutor<string_t, string_t, bool>(geom1_arg, geom2_arg, result, args.size());
}

struct WithInBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get within: could not getting within from geom");
			return false;
		}
		auto withinRv = Geometry::GeometryWithin(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return withinRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryWithinBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, WithInBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryWithinFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryWithinBinaryExecutor<string_t, string_t, bool>(geom1_arg, geom2_arg, result, args.size());
}

struct IntersectsBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get intersects: could not getting intersects from geom");
			return false;
		}
		auto intersectsRv = Geometry::GeometryIntersects(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return intersectsRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryIntersectsBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, IntersectsBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryIntersectsFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryIntersectsBinaryExecutor<string_t, string_t, bool>(geom1_arg, geom2_arg, result, args.size());
}

struct CoversBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get covers: could not getting covers from geom");
			return false;
		}
		auto coversRv = Geometry::GeometryCovers(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return coversRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryCoversBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, CoversBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryCoversFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryCoversBinaryExecutor<string_t, string_t, bool>(geom1_arg, geom2_arg, result, args.size());
}

struct CoveredByBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get covered by: could not getting covered by from geom");
			return false;
		}
		auto coveredbyRv = Geometry::GeometryCoveredby(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return coveredbyRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryCoveredByBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, CoveredByBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryCoveredByFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryCoveredByBinaryExecutor<string_t, string_t, bool>(geom1_arg, geom2_arg, result, args.size());
}

struct DisjointBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get disjoint: could not getting disjoint from geom");
			return false;
		}
		auto disjointRv = Geometry::GeometryDisjoint(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return disjointRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryDisjointBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, DisjointBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryDisjointFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryDisjointBinaryExecutor<string_t, string_t, bool>(geom1_arg, geom2_arg, result, args.size());
}

struct DWithinTernaryOperator {
	template <class TA, class TB, class TC, class TR>
	static inline TR Operation(TA geom1, TB geom2, TC distance) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get dwithin: could not getting dwithin from geom");
			return false;
		}
		auto dWithinRv = Geometry::GeometryDWithin(gser1, gser2, distance);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return dWithinRv;
	}
};

template <typename TA, typename TB, typename TC, typename TR>
static void GeometryDWithinTernaryExecutor(Vector &geom1, Vector &geom2, Vector &distance, Vector &result,
                                           idx_t count) {
	TernaryExecutor::Execute<TA, TB, TC, TR>(geom1, geom2, distance, result, count,
	                                         DWithinTernaryOperator::Operation<TA, TB, TC, TR>);
}

void GeoFunctions::GeometryDWithinFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	auto &distance_arg = args.data[2];
	GeometryDWithinTernaryExecutor<string_t, string_t, double, bool>(geom1_arg, geom2_arg, distance_arg, result,
	                                                                 args.size());
}

struct AreaOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			return 0;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			return 0;
		}
		auto area = Geometry::GeometryArea(gser);
		Geometry::DestroyGeometry(gser);
		return area;
	}
};

struct AreaBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom, TB use_spheroid) {
		if (geom.GetSize() == 0) {
			return 0;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry get area: could not getting area from geom");
			return false;
		}
		auto area = Geometry::GeometryArea(gser, use_spheroid);
		Geometry::DestroyGeometry(gser);
		return area;
	}
};

template <typename TA, typename TR>
static void GeometryAreaUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, AreaOperator>(geom, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryAreaBinaryExecutor(Vector &geom, Vector &use_spheroid, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, AreaBinaryOperator>(geom, use_spheroid, result, count);
}

void GeoFunctions::GeometryAreaFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryAreaUnaryExecutor<string_t, double>(geom_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &use_spheroid_arg = args.data[1];
		GeometryAreaBinaryExecutor<string_t, bool, double>(geom_arg, use_spheroid_arg, result, args.size());
	}
}

struct AngleTernaryOperator {
	template <class TA, class TB, class TC, class TR>
	static inline TR Operation(TA geom1, TB geom2, TC geom3) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0 && geom3.GetSize() == 0) {
			return 0.0;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0 || geom3.GetSize() == 0) {
			return 0.0;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		auto gser3 = Geometry::GetGserialized(geom3);
		if (!gser1 || !gser2 || !gser3) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			if (gser3) {
				Geometry::DestroyGeometry(gser3);
			}
			throw ConversionException("Failure in geometry get angle: could not getting angle from geom");
			return false;
		}
		auto angle = Geometry::GeometryAngle(gser1, gser2, gser3);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		Geometry::DestroyGeometry(gser3);
		return angle;
	}
};

template <typename TA, typename TB, typename TC, typename TR>
static void GeometryAngleTernaryExecutor(Vector &geom1, Vector &geom2, Vector &geom3, Vector &result, idx_t count) {
	TernaryExecutor::Execute<TA, TB, TC, TR>(geom1, geom2, geom3, result, count,
	                                         AngleTernaryOperator::Operation<TA, TB, TC, TR>);
}

void GeoFunctions::GeometryAngleFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	auto &geom3_arg = args.data[2];
	GeometryAngleTernaryExecutor<string_t, string_t, string_t, double>(geom1_arg, geom2_arg, geom3_arg, result,
	                                                                   args.size());
}

struct PerimeterUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			return 0.0;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			return 0.0;
		}
		auto perimeter = Geometry::GeometryPerimeter(gser);
		Geometry::DestroyGeometry(gser);
		return perimeter;
	}
};

struct PerimeterBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom, TB use_spheroid) {
		if (geom.GetSize() == 0) {
			return 0;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry get perimeter: could not getting perimeter from geom");
			return false;
		}
		auto perimeter = Geometry::GeometryPerimeter(gser, use_spheroid);
		Geometry::DestroyGeometry(gser);
		return perimeter;
	}
};

template <typename TA, typename TR>
static void GeometryPerimeterUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, PerimeterUnaryOperator>(geom, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryPerimeterBinaryExecutor(Vector &geom, Vector &use_spheroid, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, PerimeterBinaryOperator>(geom, use_spheroid, result, count);
}

void GeoFunctions::GeometryPerimeterFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryPerimeterUnaryExecutor<string_t, double>(geom_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &use_spheroid_arg = args.data[1];
		GeometryPerimeterBinaryExecutor<string_t, bool, double>(geom_arg, use_spheroid_arg, result, args.size());
	}
}

struct AzimuthBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		if (geom1.GetSize() == 0 && geom2.GetSize() == 0) {
			return true;
		}
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return false;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			if (gser1) {
				Geometry::DestroyGeometry(gser1);
			}
			if (gser2) {
				Geometry::DestroyGeometry(gser2);
			}
			throw ConversionException("Failure in geometry get azimuth: could not getting azimuth from geom");
			return false;
		}
		auto azimuthRv = Geometry::GeometryAzimuth(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return azimuthRv;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryAzimuthBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, AzimuthBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryAzimuthFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryAzimuthBinaryExecutor<string_t, string_t, double>(geom1_arg, geom2_arg, result, args.size());
}

struct LengthUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom) {
		if (geom.GetSize() == 0) {
			return 0.0;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			return 0.0;
		}
		auto length = Geometry::GeometryLength(gser);
		Geometry::DestroyGeometry(gser);
		return length;
	}
};

struct LengthBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom, TB use_spheroid) {
		if (geom.GetSize() == 0) {
			return 0;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry get length: could not getting length from geom");
			return false;
		}
		auto length = Geometry::GeometryLength(gser, use_spheroid);
		Geometry::DestroyGeometry(gser);
		return length;
	}
};

template <typename TA, typename TR>
static void GeometryLengthUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::Execute<TA, TR, LengthUnaryOperator>(geom, result, count);
}

template <typename TA, typename TB, typename TR>
static void GeometryLengthBinaryExecutor(Vector &geom, Vector &use_spheroid, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, LengthBinaryOperator>(geom, use_spheroid, result, count);
}

void GeoFunctions::GeometryLengthFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	if (args.data.size() == 1) {
		GeometryLengthUnaryExecutor<string_t, double>(geom_arg, result, args.size());
	} else if (args.data.size() == 2) {
		auto &use_spheroid_arg = args.data[1];
		GeometryLengthBinaryExecutor<string_t, bool, double>(geom_arg, use_spheroid_arg, result, args.size());
	}
}

struct BoundingBoxUnaryOperator {
	template <class TA, class TR>
	static inline TR Operation(TA geom, Vector &result) {
		if (geom.GetSize() == 0) {
			return geom;
		}
		auto gser = Geometry::GetGserialized(geom);
		if (!gser) {
			throw ConversionException("Failure in geometry get bounding box: could not getting bounding box from geom");
			return string_t();
		}
		auto gserBoundingBox = Geometry::GeometryBoundingBox(gser);
		if (gser == gserBoundingBox) {
			Geometry::DestroyGeometry(gser);
			return geom;
		}
		idx_t rv_size = Geometry::GetGeometrySize(gserBoundingBox);
		auto base = Geometry::GetBase(gserBoundingBox);
		auto result_str = StringVector::EmptyString(result, rv_size);
		memcpy(result_str.GetDataWriteable(), base, rv_size);
		result_str.Finalize();
		Geometry::DestroyGeometry(gser);
		Geometry::DestroyGeometry(gserBoundingBox);
		return result_str;
	}
};

template <typename TA, typename TR>
static void GeometryBoundingBoxUnaryExecutor(Vector &geom, Vector &result, idx_t count) {
	UnaryExecutor::ExecuteString<TA, TR, BoundingBoxUnaryOperator>(geom, result, count);
}

void GeoFunctions::GeometryBoundingBoxFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom_arg = args.data[0];
	GeometryBoundingBoxUnaryExecutor<string_t, string_t>(geom_arg, result, args.size());
}

struct GeometryMaxDistanceBinaryOperator {
	template <class TA, class TB, class TR>
	static inline TR Operation(TA geom1, TB geom2) {
		double dis = 0.00;
		if (geom1.GetSize() == 0 || geom2.GetSize() == 0) {
			return dis;
		}
		auto gser1 = Geometry::GetGserialized(geom1);
		auto gser2 = Geometry::GetGserialized(geom2);
		if (!gser1 || !gser2) {
			throw ConversionException(
			    "Failure in geometry maximum distance: could not calculate maximum distance from geometries");
		}
		dis = Geometry::MaxDistance(gser1, gser2);
		Geometry::DestroyGeometry(gser1);
		Geometry::DestroyGeometry(gser2);
		return dis;
	}
};

template <typename TA, typename TB, typename TR>
static void GeometryMaxDistanceBinaryExecutor(Vector &geom1, Vector &geom2, Vector &result, idx_t count) {
	BinaryExecutor::ExecuteStandard<TA, TB, TR, GeometryMaxDistanceBinaryOperator>(geom1, geom2, result, count);
}

void GeoFunctions::GeometryMaxDistanceFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &geom1_arg = args.data[0];
	auto &geom2_arg = args.data[1];
	GeometryMaxDistanceBinaryExecutor<string_t, string_t, double>(geom1_arg, geom2_arg, result, args.size());
}

void GeoFunctions::GeometryExtentFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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
		auto gserExtent = Geometry::GeometryExtent(&gserArray[0], list_entry.length);
		if (!gserExtent) {
			for (idx_t child_idx = 0; child_idx < list_entry.length; child_idx++) {
				Geometry::DestroyGeometry(gserArray[child_idx]);
			}
			continue;
		}
		idx_t rv_size = Geometry::GetGeometrySize(gserExtent);
		auto base = Geometry::GetBase(gserExtent);
		for (idx_t child_idx = 0; child_idx < list_entry.length; child_idx++) {
			Geometry::DestroyGeometry(gserArray[child_idx]);
		}
		Geometry::DestroyGeometry(gserExtent);
		result_entries[i] = string_t((const char *)base, rv_size);
	}
}

} // namespace duckdb
