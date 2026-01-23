// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.property_generic;

LGenericFloatPropertyWrapper::LGenericFloatPropertyWrapper(const pragma::util::PFloatProperty &prop) : m_property(prop), m_propertyType(ArithmeticFloatPropertyType::Float) {}
LGenericFloatPropertyWrapper::LGenericFloatPropertyWrapper(const pragma::util::PDoubleProperty &prop) : m_property(prop), m_propertyType(ArithmeticFloatPropertyType::Double) {}
LGenericFloatPropertyWrapper::LGenericFloatPropertyWrapper(const pragma::util::PLongDoubleProperty &prop) : m_property(prop), m_propertyType(ArithmeticFloatPropertyType::LongDouble) {}
LGenericFloatPropertyWrapper::LGenericFloatPropertyWrapper(double v) : LGenericFloatPropertyWrapper(pragma::util::DoubleProperty::Create(v)) {}
LGenericFloatPropertyWrapper::LGenericFloatPropertyWrapper() : LGenericFloatPropertyWrapper(0.0) {}

LGenericFloatPropertyWrapper *LGenericFloatPropertyWrapper::operator->() { return this; }
void LGenericFloatPropertyWrapper::InvokeCallbacks()
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property).InvokeCallbacks();
		break;
	}
}
CallbackHandle LGenericFloatPropertyWrapper::AddCallback(const std::function<void(std::reference_wrapper<const double>, std::reference_wrapper<const double>)> &callback)
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return static_cast<pragma::util::FloatProperty &>(*m_property).AddCallback([callback](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) {
			auto dOldVal = static_cast<double>(oldVal);
			auto dNewVal = static_cast<double>(newVal);
			callback(dOldVal, dNewVal);
		});
	case ArithmeticFloatPropertyType::Double:
		return static_cast<pragma::util::DoubleProperty &>(*m_property).AddCallback(callback);
	case ArithmeticFloatPropertyType::LongDouble:
		return static_cast<pragma::util::LongDoubleProperty &>(*m_property).AddCallback([callback](std::reference_wrapper<const long double> oldVal, std::reference_wrapper<const long double> newVal) {
			auto dOldVal = static_cast<double>(oldVal);
			auto dNewVal = static_cast<double>(newVal);
			callback(dOldVal, dNewVal);
		});
	}
	return {};
}
CallbackHandle LGenericFloatPropertyWrapper::AddModifier(const std::function<void(std::reference_wrapper<double>)> &fModifier)
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return static_cast<pragma::util::FloatProperty &>(*m_property).AddModifier([fModifier](std::reference_wrapper<float> val) {
			auto dVal = static_cast<double>(val);
			fModifier(dVal);
			val.get() = static_cast<float>(dVal);
		});
	case ArithmeticFloatPropertyType::Double:
		return static_cast<pragma::util::DoubleProperty &>(*m_property).AddModifier(fModifier);
	case ArithmeticFloatPropertyType::LongDouble:
		return static_cast<pragma::util::LongDoubleProperty &>(*m_property).AddModifier([fModifier](std::reference_wrapper<long double> val) {
			auto dVal = static_cast<double>(val);
			fModifier(dVal);
			val.get() = static_cast<long double>(dVal);
		});
	}
	return {};
}
void LGenericFloatPropertyWrapper::LinkOther(pragma::util::StringProperty &prop)
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		prop.Link<pragma::util::FloatProperty, float>(static_cast<pragma::util::FloatProperty &>(*m_property), [](const float &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticFloatPropertyType::Double:
		prop.Link<pragma::util::DoubleProperty, double>(static_cast<pragma::util::DoubleProperty &>(*m_property), [](const double &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		prop.Link<pragma::util::LongDoubleProperty, long double>(static_cast<pragma::util::LongDoubleProperty &>(*m_property), [](const long double &other) -> std::string { return std::to_string(other); });
		break;
	}
}
void LGenericFloatPropertyWrapper::Link(pragma::util::StringProperty &prop)
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> float { return pragma::util::to_float(other); });
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> double { return static_cast<double>(pragma::util::to_float(other)); });
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> long double { return static_cast<long double>(pragma::util::to_float(other)); });
		break;
	}
}
void LGenericFloatPropertyWrapper::Link(LGenericFloatPropertyWrapper &prop)
{
	switch(prop.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		Link(static_cast<pragma::util::FloatProperty &>(*prop.m_property));
		break;
	case ArithmeticFloatPropertyType::Double:
		Link(static_cast<pragma::util::DoubleProperty &>(*prop.m_property));
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		Link(static_cast<pragma::util::LongDoubleProperty &>(*prop.m_property));
		break;
	}
}
void LGenericFloatPropertyWrapper::Link(LGenericIntPropertyWrapper &prop)
{
	switch(prop.m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		Link(static_cast<pragma::util::Int8Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::UInt8:
		Link(static_cast<pragma::util::UInt8Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::Int16:
		Link(static_cast<pragma::util::Int16Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::UInt16:
		Link(static_cast<pragma::util::UInt16Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::Int32:
		Link(static_cast<pragma::util::Int32Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::UInt32:
		Link(static_cast<pragma::util::UInt32Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::Int64:
		Link(static_cast<pragma::util::Int64Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::UInt64:
		Link(static_cast<pragma::util::UInt64Property &>(*prop.m_property));
		break;
	}
}
void LGenericFloatPropertyWrapper::Unlink()
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property).Unlink();
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property).Unlink();
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property).Unlink();
		break;
	}
}
float LGenericFloatPropertyWrapper::GetValue() const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return static_cast<pragma::util::FloatProperty &>(*m_property).GetValue();
	case ArithmeticFloatPropertyType::Double:
		return static_cast<float>(static_cast<pragma::util::DoubleProperty &>(*m_property).GetValue());
	case ArithmeticFloatPropertyType::LongDouble:
		return static_cast<float>(static_cast<pragma::util::LongDoubleProperty &>(*m_property).GetValue());
	}
	return 0.f;
}
void LGenericFloatPropertyWrapper::SetValue(float f)
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		*static_cast<pragma::util::FloatProperty &>(*m_property) = f;
		break;
	case ArithmeticFloatPropertyType::Double:
		*static_cast<pragma::util::DoubleProperty &>(*m_property) = f;
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		*static_cast<pragma::util::LongDoubleProperty &>(*m_property) = f;
		break;
	}
	return;
}
LGenericFloatPropertyWrapper &LGenericFloatPropertyWrapper::operator=(const double &other)
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property) = other;
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property) = other;
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property) = other;
		break;
	}
	return *this;
}
void LGenericFloatPropertyWrapper::SetLocked(bool bLocked, bool bLinkLocked)
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	}
}
bool LGenericFloatPropertyWrapper::IsLocked() const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return static_cast<pragma::util::FloatProperty &>(*m_property).IsLocked();
	case ArithmeticFloatPropertyType::Double:
		return static_cast<pragma::util::DoubleProperty &>(*m_property).IsLocked();
	case ArithmeticFloatPropertyType::LongDouble:
		return static_cast<pragma::util::LongDoubleProperty &>(*m_property).IsLocked();
	}
	return false;
}

LGenericFloatPropertyWrapper LGenericFloatPropertyWrapper::operator+(const double &val) const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property) += static_cast<float>(val);
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property) += val;
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property) += static_cast<long double>(val);
		break;
	}
	return *this;
}
LGenericFloatPropertyWrapper LGenericFloatPropertyWrapper::operator+(const LGenericFloatPropertyWrapper &propOther) const
{
	switch(propOther.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return operator+(static_cast<pragma::util::FloatProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::Double:
		return operator+(static_cast<pragma::util::DoubleProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::LongDouble:
		return operator+(static_cast<pragma::util::LongDoubleProperty &>(*propOther.m_property));
	}
	return *this;
}
LGenericFloatPropertyWrapper LGenericFloatPropertyWrapper::operator-(const double &val) const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property) -= static_cast<float>(val);
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property) -= val;
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property) -= static_cast<long double>(val);
		break;
	}
	return *this;
}
LGenericFloatPropertyWrapper LGenericFloatPropertyWrapper::operator-(const LGenericFloatPropertyWrapper &propOther) const
{
	switch(propOther.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return operator-(static_cast<pragma::util::FloatProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::Double:
		return operator-(static_cast<pragma::util::DoubleProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::LongDouble:
		return operator-(static_cast<pragma::util::LongDoubleProperty &>(*propOther.m_property));
	}
	return *this;
}

LGenericFloatPropertyWrapper LGenericFloatPropertyWrapper::operator*(const double &val) const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property) *= static_cast<float>(val);
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property) *= val;
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property) *= static_cast<long double>(val);
		break;
	}
	return *this;
}
LGenericFloatPropertyWrapper LGenericFloatPropertyWrapper::operator*(const LGenericFloatPropertyWrapper &propOther) const
{
	switch(propOther.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return operator*(static_cast<pragma::util::FloatProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::Double:
		return operator*(static_cast<pragma::util::DoubleProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::LongDouble:
		return operator*(static_cast<pragma::util::LongDoubleProperty &>(*propOther.m_property));
	}
	return *this;
}

LGenericFloatPropertyWrapper LGenericFloatPropertyWrapper::operator/(const double &val) const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		static_cast<pragma::util::FloatProperty &>(*m_property) /= static_cast<float>(val);
		break;
	case ArithmeticFloatPropertyType::Double:
		static_cast<pragma::util::DoubleProperty &>(*m_property) /= val;
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		static_cast<pragma::util::LongDoubleProperty &>(*m_property) /= static_cast<long double>(val);
		break;
	}
	return *this;
}
LGenericFloatPropertyWrapper LGenericFloatPropertyWrapper::operator/(const LGenericFloatPropertyWrapper &propOther) const
{
	switch(propOther.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return operator/(static_cast<pragma::util::FloatProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::Double:
		return operator/(static_cast<pragma::util::DoubleProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::LongDouble:
		return operator/(static_cast<pragma::util::LongDoubleProperty &>(*propOther.m_property));
	}
	return *this;
}

bool LGenericFloatPropertyWrapper::operator==(const double &val) const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return static_cast<pragma::util::FloatProperty &>(*m_property) == static_cast<float>(val);
	case ArithmeticFloatPropertyType::Double:
		return static_cast<pragma::util::DoubleProperty &>(*m_property) == val;
	case ArithmeticFloatPropertyType::LongDouble:
		return static_cast<pragma::util::LongDoubleProperty &>(*m_property) == static_cast<long double>(val);
	}
	return false;
}
bool LGenericFloatPropertyWrapper::operator==(const LGenericFloatPropertyWrapper &propOther) const
{
	switch(propOther.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return operator==(static_cast<pragma::util::FloatProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::Double:
		return operator==(static_cast<pragma::util::DoubleProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::LongDouble:
		return operator==(static_cast<pragma::util::LongDoubleProperty &>(*propOther.m_property));
	}
	return false;
}

// Comparison
bool LGenericFloatPropertyWrapper::operator<(const double &val) const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return static_cast<pragma::util::FloatProperty &>(*m_property) < static_cast<float>(val);
	case ArithmeticFloatPropertyType::Double:
		return static_cast<pragma::util::DoubleProperty &>(*m_property) < val;
	case ArithmeticFloatPropertyType::LongDouble:
		return static_cast<pragma::util::LongDoubleProperty &>(*m_property) < static_cast<long double>(val);
	}
	return false;
}
bool LGenericFloatPropertyWrapper::operator<(const LGenericFloatPropertyWrapper &propOther) const
{
	switch(propOther.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return operator<(static_cast<pragma::util::FloatProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::Double:
		return operator<(static_cast<pragma::util::DoubleProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::LongDouble:
		return operator<(static_cast<pragma::util::LongDoubleProperty &>(*propOther.m_property));
	}
	return false;
}
bool LGenericFloatPropertyWrapper::operator<=(const double &val) const
{
	switch(m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return static_cast<pragma::util::FloatProperty &>(*m_property) <= static_cast<float>(val);
	case ArithmeticFloatPropertyType::Double:
		return static_cast<pragma::util::DoubleProperty &>(*m_property) <= val;
	case ArithmeticFloatPropertyType::LongDouble:
		return static_cast<pragma::util::LongDoubleProperty &>(*m_property) <= static_cast<long double>(val);
	}
	return false;
}
bool LGenericFloatPropertyWrapper::operator<=(const LGenericFloatPropertyWrapper &propOther) const
{
	switch(propOther.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		return operator<=(static_cast<pragma::util::FloatProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::Double:
		return operator<=(static_cast<pragma::util::DoubleProperty &>(*propOther.m_property));
	case ArithmeticFloatPropertyType::LongDouble:
		return operator<=(static_cast<pragma::util::LongDoubleProperty &>(*propOther.m_property));
	}
	return false;
}

////////////////

LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(const pragma::util::PInt8Property &prop) : m_property(prop), m_propertyType(ArithmeticIntPropertyType::Int8) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(const pragma::util::PUInt8Property &prop) : m_property(prop), m_propertyType(ArithmeticIntPropertyType::UInt8) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(const pragma::util::PInt16Property &prop) : m_property(prop), m_propertyType(ArithmeticIntPropertyType::Int16) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(const pragma::util::PUInt16Property &prop) : m_property(prop), m_propertyType(ArithmeticIntPropertyType::UInt16) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(const pragma::util::PInt32Property &prop) : m_property(prop), m_propertyType(ArithmeticIntPropertyType::Int32) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(const pragma::util::PUInt32Property &prop) : m_property(prop), m_propertyType(ArithmeticIntPropertyType::UInt32) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(const pragma::util::PInt64Property &prop) : m_property(prop), m_propertyType(ArithmeticIntPropertyType::Int64) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(const pragma::util::PUInt64Property &prop) : m_property(prop), m_propertyType(ArithmeticIntPropertyType::UInt64) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper(int32_t v) : LGenericIntPropertyWrapper(pragma::util::Int32Property::Create(v)) {}
LGenericIntPropertyWrapper::LGenericIntPropertyWrapper() : LGenericIntPropertyWrapper(0) {}

LGenericIntPropertyWrapper *LGenericIntPropertyWrapper::operator->() { return this; }
void LGenericIntPropertyWrapper::InvokeCallbacks()
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property).InvokeCallbacks();
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property).InvokeCallbacks();
		break;
	}
}
CallbackHandle LGenericIntPropertyWrapper::AddCallback(const std::function<void(std::reference_wrapper<const int32_t>, std::reference_wrapper<const int32_t>)> &callback)
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return static_cast<pragma::util::Int8Property &>(*m_property).AddCallback([callback](std::reference_wrapper<const int8_t> oldVal, std::reference_wrapper<const int8_t> newVal) {
			auto dOldVal = static_cast<int32_t>(oldVal);
			auto dNewVal = static_cast<int32_t>(newVal);
			callback(dOldVal, dNewVal);
		});
	case ArithmeticIntPropertyType::UInt8:
		return static_cast<pragma::util::UInt8Property &>(*m_property).AddCallback([callback](std::reference_wrapper<const uint8_t> oldVal, std::reference_wrapper<const uint8_t> newVal) {
			auto dOldVal = static_cast<int32_t>(oldVal);
			auto dNewVal = static_cast<int32_t>(newVal);
			callback(dOldVal, dNewVal);
		});
	case ArithmeticIntPropertyType::Int16:
		return static_cast<pragma::util::Int16Property &>(*m_property).AddCallback([callback](std::reference_wrapper<const int16_t> oldVal, std::reference_wrapper<const int16_t> newVal) {
			auto dOldVal = static_cast<int32_t>(oldVal);
			auto dNewVal = static_cast<int32_t>(newVal);
			callback(dOldVal, dNewVal);
		});
	case ArithmeticIntPropertyType::UInt16:
		return static_cast<pragma::util::UInt16Property &>(*m_property).AddCallback([callback](std::reference_wrapper<const uint16_t> oldVal, std::reference_wrapper<const uint16_t> newVal) {
			auto dOldVal = static_cast<int32_t>(oldVal);
			auto dNewVal = static_cast<int32_t>(newVal);
			callback(dOldVal, dNewVal);
		});
	case ArithmeticIntPropertyType::Int32:
		return static_cast<pragma::util::Int32Property &>(*m_property).AddCallback(callback);
	case ArithmeticIntPropertyType::UInt32:
		return static_cast<pragma::util::UInt32Property &>(*m_property).AddCallback([callback](std::reference_wrapper<const uint32_t> oldVal, std::reference_wrapper<const uint32_t> newVal) {
			auto dOldVal = static_cast<int32_t>(oldVal);
			auto dNewVal = static_cast<int32_t>(newVal);
			callback(dOldVal, dNewVal);
		});
	case ArithmeticIntPropertyType::Int64:
		return static_cast<pragma::util::Int64Property &>(*m_property).AddCallback([callback](std::reference_wrapper<const int64_t> oldVal, std::reference_wrapper<const int64_t> newVal) {
			auto dOldVal = static_cast<int32_t>(oldVal);
			auto dNewVal = static_cast<int32_t>(newVal);
			callback(dOldVal, dNewVal);
		});
	case ArithmeticIntPropertyType::UInt64:
		return static_cast<pragma::util::UInt64Property &>(*m_property).AddCallback([callback](std::reference_wrapper<const uint64_t> oldVal, std::reference_wrapper<const uint64_t> newVal) {
			auto dOldVal = static_cast<int32_t>(oldVal);
			auto dNewVal = static_cast<int32_t>(newVal);
			callback(dOldVal, dNewVal);
		});
	}
	return {};
}
CallbackHandle LGenericIntPropertyWrapper::AddModifier(const std::function<void(std::reference_wrapper<int32_t>)> &fModifier)
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return static_cast<pragma::util::Int8Property &>(*m_property).AddModifier([fModifier](std::reference_wrapper<int8_t> val) {
			auto dVal = static_cast<int32_t>(val);
			fModifier(dVal);
			val.get() = static_cast<int8_t>(dVal);
		});
	case ArithmeticIntPropertyType::UInt8:
		return static_cast<pragma::util::UInt8Property &>(*m_property).AddModifier([fModifier](std::reference_wrapper<uint8_t> val) {
			auto dVal = static_cast<int32_t>(val);
			fModifier(dVal);
			val.get() = static_cast<uint8_t>(dVal);
		});
	case ArithmeticIntPropertyType::Int16:
		return static_cast<pragma::util::Int16Property &>(*m_property).AddModifier([fModifier](std::reference_wrapper<int16_t> val) {
			auto dVal = static_cast<int32_t>(val);
			fModifier(dVal);
			val.get() = static_cast<int16_t>(dVal);
		});
	case ArithmeticIntPropertyType::UInt16:
		return static_cast<pragma::util::UInt16Property &>(*m_property).AddModifier([fModifier](std::reference_wrapper<uint16_t> val) {
			auto dVal = static_cast<int32_t>(val);
			fModifier(dVal);
			val.get() = static_cast<uint16_t>(dVal);
		});
	case ArithmeticIntPropertyType::Int32:
		return static_cast<pragma::util::Int32Property &>(*m_property).AddModifier(fModifier);
	case ArithmeticIntPropertyType::UInt32:
		return static_cast<pragma::util::UInt32Property &>(*m_property).AddModifier([fModifier](std::reference_wrapper<uint32_t> val) {
			auto dVal = static_cast<int32_t>(val);
			fModifier(dVal);
			val.get() = static_cast<uint32_t>(dVal);
		});
	case ArithmeticIntPropertyType::Int64:
		return static_cast<pragma::util::Int64Property &>(*m_property).AddModifier([fModifier](std::reference_wrapper<int64_t> val) {
			auto dVal = static_cast<int32_t>(val);
			fModifier(dVal);
			val.get() = static_cast<int64_t>(dVal);
		});
	case ArithmeticIntPropertyType::UInt64:
		return static_cast<pragma::util::UInt64Property &>(*m_property).AddModifier([fModifier](std::reference_wrapper<uint64_t> val) {
			auto dVal = static_cast<int32_t>(val);
			fModifier(dVal);
			val.get() = static_cast<uint64_t>(dVal);
		});
	}
	return {};
}
void LGenericIntPropertyWrapper::LinkOther(pragma::util::StringProperty &prop)
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		prop.Link<pragma::util::Int8Property, int8_t>(static_cast<pragma::util::Int8Property &>(*m_property), [](const int8_t &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticIntPropertyType::UInt8:
		prop.Link<pragma::util::UInt8Property, uint8_t>(static_cast<pragma::util::UInt8Property &>(*m_property), [](const uint8_t &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticIntPropertyType::Int16:
		prop.Link<pragma::util::Int16Property, int16_t>(static_cast<pragma::util::Int16Property &>(*m_property), [](const int16_t &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticIntPropertyType::UInt16:
		prop.Link<pragma::util::UInt16Property, uint16_t>(static_cast<pragma::util::UInt16Property &>(*m_property), [](const uint16_t &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticIntPropertyType::Int32:
		prop.Link<pragma::util::Int32Property, int32_t>(static_cast<pragma::util::Int32Property &>(*m_property), [](const int32_t &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticIntPropertyType::UInt32:
		prop.Link<pragma::util::UInt32Property, uint32_t>(static_cast<pragma::util::UInt32Property &>(*m_property), [](const uint32_t &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticIntPropertyType::Int64:
		prop.Link<pragma::util::Int64Property, int64_t>(static_cast<pragma::util::Int64Property &>(*m_property), [](const int64_t &other) -> std::string { return std::to_string(other); });
		break;
	case ArithmeticIntPropertyType::UInt64:
		prop.Link<pragma::util::UInt64Property, uint64_t>(static_cast<pragma::util::UInt64Property &>(*m_property), [](const uint64_t &other) -> std::string { return std::to_string(other); });
		break;
	}
}
void LGenericIntPropertyWrapper::Link(pragma::util::StringProperty &prop)
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> int8_t { return pragma::util::to_int(other); });
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> uint8_t { return pragma::util::to_int(other); });
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> int16_t { return pragma::util::to_int(other); });
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> uint16_t { return pragma::util::to_int(other); });
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> int32_t { return pragma::util::to_int(other); });
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> uint32_t { return pragma::util::to_int(other); });
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> int64_t { return pragma::util::to_int(other); });
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property).Link<pragma::util::StringProperty, std::string>(prop, [](const std::string &other) -> uint64_t { return pragma::util::to_int(other); });
		break;
	}
}
void LGenericIntPropertyWrapper::Link(LGenericFloatPropertyWrapper &prop)
{
	switch(prop.m_propertyType) {
	case ArithmeticFloatPropertyType::Float:
		Link(static_cast<pragma::util::FloatProperty &>(*prop.m_property));
		break;
	case ArithmeticFloatPropertyType::Double:
		Link(static_cast<pragma::util::DoubleProperty &>(*prop.m_property));
		break;
	case ArithmeticFloatPropertyType::LongDouble:
		Link(static_cast<pragma::util::LongDoubleProperty &>(*prop.m_property));
		break;
	}
}
void LGenericIntPropertyWrapper::Link(LGenericIntPropertyWrapper &prop)
{
	switch(prop.m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		Link(static_cast<pragma::util::Int8Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::UInt8:
		Link(static_cast<pragma::util::UInt8Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::Int16:
		Link(static_cast<pragma::util::Int16Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::UInt16:
		Link(static_cast<pragma::util::UInt16Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::Int32:
		Link(static_cast<pragma::util::Int32Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::UInt32:
		Link(static_cast<pragma::util::UInt32Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::Int64:
		Link(static_cast<pragma::util::Int64Property &>(*prop.m_property));
		break;
	case ArithmeticIntPropertyType::UInt64:
		Link(static_cast<pragma::util::UInt64Property &>(*prop.m_property));
		break;
	}
}
void LGenericIntPropertyWrapper::Unlink()
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property).Unlink();
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property).Unlink();
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property).Unlink();
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property).Unlink();
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property).Unlink();
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property).Unlink();
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property).Unlink();
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property).Unlink();
		break;
	}
}
int32_t LGenericIntPropertyWrapper::GetValue() const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return static_cast<int32_t>(static_cast<pragma::util::Int8Property &>(*m_property).GetValue());
	case ArithmeticIntPropertyType::UInt8:
		return static_cast<int32_t>(static_cast<pragma::util::UInt8Property &>(*m_property).GetValue());
	case ArithmeticIntPropertyType::Int16:
		return static_cast<int32_t>(static_cast<pragma::util::Int16Property &>(*m_property).GetValue());
	case ArithmeticIntPropertyType::UInt16:
		return static_cast<int32_t>(static_cast<pragma::util::UInt16Property &>(*m_property).GetValue());
	case ArithmeticIntPropertyType::Int32:
		return static_cast<pragma::util::Int32Property &>(*m_property).GetValue();
	case ArithmeticIntPropertyType::UInt32:
		return static_cast<int32_t>(static_cast<pragma::util::UInt32Property &>(*m_property).GetValue());
	case ArithmeticIntPropertyType::Int64:
		return static_cast<int32_t>(static_cast<pragma::util::Int64Property &>(*m_property).GetValue());
	case ArithmeticIntPropertyType::UInt64:
		return static_cast<int32_t>(static_cast<pragma::util::UInt64Property &>(*m_property).GetValue());
	}
	return 0;
}
void LGenericIntPropertyWrapper::SetValue(int32_t i)
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		*static_cast<pragma::util::Int8Property &>(*m_property) = i;
		break;
	case ArithmeticIntPropertyType::UInt8:
		*static_cast<pragma::util::UInt8Property &>(*m_property) = i;
		break;
	case ArithmeticIntPropertyType::Int16:
		*static_cast<pragma::util::Int16Property &>(*m_property) = i;
		break;
	case ArithmeticIntPropertyType::UInt16:
		*static_cast<pragma::util::UInt16Property &>(*m_property) = i;
		break;
	case ArithmeticIntPropertyType::Int32:
		*static_cast<pragma::util::Int32Property &>(*m_property) = i;
		break;
	case ArithmeticIntPropertyType::UInt32:
		*static_cast<pragma::util::UInt32Property &>(*m_property) = i;
		break;
	case ArithmeticIntPropertyType::Int64:
		*static_cast<pragma::util::Int64Property &>(*m_property) = i;
		break;
	case ArithmeticIntPropertyType::UInt64:
		*static_cast<pragma::util::UInt64Property &>(*m_property) = i;
		break;
	}
	return;
}
LGenericIntPropertyWrapper &LGenericIntPropertyWrapper::operator=(const int32_t &other)
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property) = other;
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property) = other;
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property) = other;
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property) = other;
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property) = other;
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property) = other;
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property) = other;
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property) = other;
		break;
	}
	return *this;
}
void LGenericIntPropertyWrapper::SetLocked(bool bLocked, bool bLinkLocked)
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property).SetLocked(bLocked, bLinkLocked);
		break;
	}
}
bool LGenericIntPropertyWrapper::IsLocked() const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return static_cast<pragma::util::Int8Property &>(*m_property).IsLocked();
	case ArithmeticIntPropertyType::UInt8:
		return static_cast<pragma::util::UInt8Property &>(*m_property).IsLocked();
	case ArithmeticIntPropertyType::Int16:
		return static_cast<pragma::util::Int16Property &>(*m_property).IsLocked();
	case ArithmeticIntPropertyType::UInt16:
		return static_cast<pragma::util::UInt16Property &>(*m_property).IsLocked();
	case ArithmeticIntPropertyType::Int32:
		return static_cast<pragma::util::Int32Property &>(*m_property).IsLocked();
	case ArithmeticIntPropertyType::UInt32:
		return static_cast<pragma::util::UInt32Property &>(*m_property).IsLocked();
	case ArithmeticIntPropertyType::Int64:
		return static_cast<pragma::util::Int64Property &>(*m_property).IsLocked();
	case ArithmeticIntPropertyType::UInt64:
		return static_cast<pragma::util::UInt64Property &>(*m_property).IsLocked();
	}
	return false;
}
LGenericIntPropertyWrapper LGenericIntPropertyWrapper::operator+(const int32_t &val) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property) += static_cast<int8_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property) += static_cast<uint8_t>(val);
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property) += static_cast<int16_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property) += static_cast<uint16_t>(val);
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property) += static_cast<int32_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property) += static_cast<uint32_t>(val);
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property) += static_cast<int64_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property) += static_cast<uint64_t>(val);
		break;
	}
	return *this;
}
LGenericIntPropertyWrapper LGenericIntPropertyWrapper::operator+(const LGenericIntPropertyWrapper &propOther) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return operator+(static_cast<pragma::util::Int8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt8:
		return operator+(static_cast<pragma::util::UInt8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int16:
		return operator+(static_cast<pragma::util::Int16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt16:
		return operator+(static_cast<pragma::util::UInt16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int32:
		return operator+(static_cast<pragma::util::Int32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt32:
		return operator+(static_cast<pragma::util::UInt32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int64:
		return operator+(static_cast<pragma::util::Int64Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt64:
		return operator+(static_cast<pragma::util::UInt64Property &>(*propOther.m_property));
	}
	return *this;
}
LGenericIntPropertyWrapper LGenericIntPropertyWrapper::operator-(const int32_t &val) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property) -= static_cast<int8_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property) -= static_cast<uint8_t>(val);
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property) -= static_cast<int16_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property) -= static_cast<uint16_t>(val);
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property) -= static_cast<int32_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property) -= static_cast<uint32_t>(val);
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property) -= static_cast<int64_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property) -= static_cast<uint64_t>(val);
		break;
	}
	return *this;
}
LGenericIntPropertyWrapper LGenericIntPropertyWrapper::operator-(const LGenericIntPropertyWrapper &propOther) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return operator-(static_cast<pragma::util::Int8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt8:
		return operator-(static_cast<pragma::util::UInt8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int16:
		return operator-(static_cast<pragma::util::Int16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt16:
		return operator-(static_cast<pragma::util::UInt16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int32:
		return operator-(static_cast<pragma::util::Int32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt32:
		return operator-(static_cast<pragma::util::UInt32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int64:
		return operator-(static_cast<pragma::util::Int64Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt64:
		return operator-(static_cast<pragma::util::UInt64Property &>(*propOther.m_property));
	}
	return *this;
}

LGenericIntPropertyWrapper LGenericIntPropertyWrapper::operator*(const int32_t &val) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property) *= static_cast<int8_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property) *= static_cast<uint8_t>(val);
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property) *= static_cast<int16_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property) *= static_cast<uint16_t>(val);
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property) *= static_cast<int32_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property) *= static_cast<uint32_t>(val);
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property) *= static_cast<int64_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property) *= static_cast<uint64_t>(val);
		break;
	}
	return *this;
}
LGenericIntPropertyWrapper LGenericIntPropertyWrapper::operator*(const LGenericIntPropertyWrapper &propOther) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return operator*(static_cast<pragma::util::Int8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt8:
		return operator*(static_cast<pragma::util::UInt8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int16:
		return operator*(static_cast<pragma::util::Int16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt16:
		return operator*(static_cast<pragma::util::UInt16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int32:
		return operator*(static_cast<pragma::util::Int32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt32:
		return operator*(static_cast<pragma::util::UInt32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int64:
		return operator*(static_cast<pragma::util::Int64Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt64:
		return operator*(static_cast<pragma::util::UInt64Property &>(*propOther.m_property));
	}
	return *this;
}

LGenericIntPropertyWrapper LGenericIntPropertyWrapper::operator/(const int32_t &val) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		static_cast<pragma::util::Int8Property &>(*m_property) /= static_cast<int8_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt8:
		static_cast<pragma::util::UInt8Property &>(*m_property) /= static_cast<uint8_t>(val);
		break;
	case ArithmeticIntPropertyType::Int16:
		static_cast<pragma::util::Int16Property &>(*m_property) /= static_cast<int16_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt16:
		static_cast<pragma::util::UInt16Property &>(*m_property) /= static_cast<uint16_t>(val);
		break;
	case ArithmeticIntPropertyType::Int32:
		static_cast<pragma::util::Int32Property &>(*m_property) /= static_cast<int32_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt32:
		static_cast<pragma::util::UInt32Property &>(*m_property) /= static_cast<uint32_t>(val);
		break;
	case ArithmeticIntPropertyType::Int64:
		static_cast<pragma::util::Int64Property &>(*m_property) /= static_cast<int64_t>(val);
		break;
	case ArithmeticIntPropertyType::UInt64:
		static_cast<pragma::util::UInt64Property &>(*m_property) /= static_cast<uint64_t>(val);
		break;
	}
	return *this;
}
LGenericIntPropertyWrapper LGenericIntPropertyWrapper::operator/(const LGenericIntPropertyWrapper &propOther) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return operator/(static_cast<pragma::util::Int8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt8:
		return operator/(static_cast<pragma::util::UInt8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int16:
		return operator/(static_cast<pragma::util::Int16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt16:
		return operator/(static_cast<pragma::util::UInt16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int32:
		return operator/(static_cast<pragma::util::Int32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt32:
		return operator/(static_cast<pragma::util::UInt32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int64:
		return operator/(static_cast<pragma::util::Int64Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt64:
		return operator/(static_cast<pragma::util::UInt64Property &>(*propOther.m_property));
	}
	return *this;
}

bool LGenericIntPropertyWrapper::operator==(const int32_t &val) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return static_cast<pragma::util::Int8Property &>(*m_property) == static_cast<int8_t>(val);
	case ArithmeticIntPropertyType::UInt8:
		return static_cast<pragma::util::UInt8Property &>(*m_property) == static_cast<uint8_t>(val);
	case ArithmeticIntPropertyType::Int16:
		return static_cast<pragma::util::Int16Property &>(*m_property) == static_cast<int16_t>(val);
	case ArithmeticIntPropertyType::UInt16:
		return static_cast<pragma::util::UInt16Property &>(*m_property) == static_cast<uint16_t>(val);
	case ArithmeticIntPropertyType::Int32:
		return static_cast<pragma::util::Int32Property &>(*m_property) == static_cast<int32_t>(val);
	case ArithmeticIntPropertyType::UInt32:
		return static_cast<pragma::util::UInt32Property &>(*m_property) == static_cast<uint32_t>(val);
	case ArithmeticIntPropertyType::Int64:
		return static_cast<pragma::util::Int64Property &>(*m_property) == static_cast<int64_t>(val);
	case ArithmeticIntPropertyType::UInt64:
		return static_cast<pragma::util::UInt64Property &>(*m_property) == static_cast<uint64_t>(val);
	}
	return false;
}
bool LGenericIntPropertyWrapper::operator==(const LGenericIntPropertyWrapper &propOther) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return operator==(static_cast<pragma::util::Int8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt8:
		return operator==(static_cast<pragma::util::UInt8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int16:
		return operator==(static_cast<pragma::util::Int16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt16:
		return operator==(static_cast<pragma::util::UInt16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int32:
		return operator==(static_cast<pragma::util::Int32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt32:
		return operator==(static_cast<pragma::util::UInt32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int64:
		return operator==(static_cast<pragma::util::Int64Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt64:
		return operator==(static_cast<pragma::util::UInt64Property &>(*propOther.m_property));
	}
	return false;
}

// Comparison
bool LGenericIntPropertyWrapper::operator<(const int32_t &val) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return static_cast<pragma::util::Int8Property &>(*m_property) < static_cast<int8_t>(val);
	case ArithmeticIntPropertyType::UInt8:
		return static_cast<pragma::util::UInt8Property &>(*m_property) < static_cast<uint8_t>(val);
	case ArithmeticIntPropertyType::Int16:
		return static_cast<pragma::util::Int16Property &>(*m_property) < static_cast<int16_t>(val);
	case ArithmeticIntPropertyType::UInt16:
		return static_cast<pragma::util::UInt16Property &>(*m_property) < static_cast<uint16_t>(val);
	case ArithmeticIntPropertyType::Int32:
		return static_cast<pragma::util::Int32Property &>(*m_property) < static_cast<int32_t>(val);
	case ArithmeticIntPropertyType::UInt32:
		return static_cast<pragma::util::UInt32Property &>(*m_property) < static_cast<uint32_t>(val);
	case ArithmeticIntPropertyType::Int64:
		return static_cast<pragma::util::Int64Property &>(*m_property) < static_cast<int64_t>(val);
	case ArithmeticIntPropertyType::UInt64:
		return static_cast<pragma::util::UInt64Property &>(*m_property) < static_cast<uint64_t>(val);
	}
	return false;
}
bool LGenericIntPropertyWrapper::operator<(const LGenericIntPropertyWrapper &propOther) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return operator<(static_cast<pragma::util::Int8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt8:
		return operator<(static_cast<pragma::util::UInt8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int16:
		return operator<(static_cast<pragma::util::Int16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt16:
		return operator<(static_cast<pragma::util::UInt16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int32:
		return operator<(static_cast<pragma::util::Int32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt32:
		return operator<(static_cast<pragma::util::UInt32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int64:
		return operator<(static_cast<pragma::util::Int64Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt64:
		return operator<(static_cast<pragma::util::UInt64Property &>(*propOther.m_property));
	}
	return false;
}
bool LGenericIntPropertyWrapper::operator<=(const int32_t &val) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return static_cast<pragma::util::Int8Property &>(*m_property) <= static_cast<int8_t>(val);
	case ArithmeticIntPropertyType::UInt8:
		return static_cast<pragma::util::UInt8Property &>(*m_property) <= static_cast<uint8_t>(val);
	case ArithmeticIntPropertyType::Int16:
		return static_cast<pragma::util::Int16Property &>(*m_property) <= static_cast<int16_t>(val);
	case ArithmeticIntPropertyType::UInt16:
		return static_cast<pragma::util::UInt16Property &>(*m_property) <= static_cast<uint16_t>(val);
	case ArithmeticIntPropertyType::Int32:
		return static_cast<pragma::util::Int32Property &>(*m_property) <= static_cast<int32_t>(val);
	case ArithmeticIntPropertyType::UInt32:
		return static_cast<pragma::util::UInt32Property &>(*m_property) <= static_cast<uint32_t>(val);
	case ArithmeticIntPropertyType::Int64:
		return static_cast<pragma::util::Int64Property &>(*m_property) <= static_cast<int64_t>(val);
	case ArithmeticIntPropertyType::UInt64:
		return static_cast<pragma::util::UInt64Property &>(*m_property) <= static_cast<uint64_t>(val);
	}
	return false;
}
bool LGenericIntPropertyWrapper::operator<=(const LGenericIntPropertyWrapper &propOther) const
{
	switch(m_propertyType) {
	case ArithmeticIntPropertyType::Int8:
		return operator<=(static_cast<pragma::util::Int8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt8:
		return operator<=(static_cast<pragma::util::UInt8Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int16:
		return operator<=(static_cast<pragma::util::Int16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt16:
		return operator<=(static_cast<pragma::util::UInt16Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int32:
		return operator<=(static_cast<pragma::util::Int32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt32:
		return operator<=(static_cast<pragma::util::UInt32Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::Int64:
		return operator<=(static_cast<pragma::util::Int64Property &>(*propOther.m_property));
	case ArithmeticIntPropertyType::UInt64:
		return operator<=(static_cast<pragma::util::UInt64Property &>(*propOther.m_property));
	}
	return false;
}
