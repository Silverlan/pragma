#ifndef __SQL_TYPE_HPP__
#define __SQL_TYPE_HPP__

namespace sql
{
	// These correspond 1:1 to mysql types
	enum class Type : uint32_t
	{
		Unknown = 0,
		Bit,
		TinyInt,
		SmallInt,
		MediumInt,
		Integer,
		BigInt,
		Real,
		Double,
		Decimal,
		Numeric,
		Char,
		Binary,
		VarChar,
		VarBinary,
		LongVarChar,
		LongVarBinary,
		TimeStamp,
		Date,
		Time,
		Year,
		Geometry,
		Enum,
		Set,
		Null,
		Initial = std::numeric_limits<std::underlying_type_t<Type>>::max()
	};
};

#endif
