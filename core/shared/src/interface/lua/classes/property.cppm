// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.property;

import :entities.property;
import :scripting.lua.call;
import :scripting.lua.classes.property_generic;
import :scripting.lua.core;
export import pragma.lua;

#define DEFINE_LUA_NUMBER_PROPERTY(TYPE, UNDERLYING_TYPE)                                                                                                                                                                                                                                        \
	using L##TYPE##Property = TLNumberPropertyWrapper<pragma::util::TYPE##Property, UNDERLYING_TYPE>;                                                                                                                                                                                                    \
	UNDERLYING_TYPE operator+(UNDERLYING_TYPE v, const L##TYPE##Property &prop) { return v + prop->GetValue(); }                                                                                                                                                                                 \
	UNDERLYING_TYPE operator-(UNDERLYING_TYPE v, const L##TYPE##Property &prop) { return v - prop->GetValue(); }                                                                                                                                                                                 \
	UNDERLYING_TYPE operator*(UNDERLYING_TYPE v, const L##TYPE##Property &prop) { return v * prop->GetValue(); }                                                                                                                                                                                 \
	UNDERLYING_TYPE operator/(UNDERLYING_TYPE v, const L##TYPE##Property &prop) { return v / prop->GetValue(); }                                                                                                                                                                                 \
	bool operator<(UNDERLYING_TYPE v, const L##TYPE##Property &prop) { return v < *prop; }                                                                                                                                                                                                       \
	bool operator<=(UNDERLYING_TYPE v, const L##TYPE##Property &prop) { return v <= *prop; }                                                                                                                                                                                                     \
	bool operator==(UNDERLYING_TYPE v, const L##TYPE##Property &prop) { return **prop == v; }                                                                                                                                                                                                    \
	std::ostream &operator<<(std::ostream &str, const L##TYPE##Property &v) { return str << **v; }

export {
	DEFINE_LUA_NUMBER_PROPERTY(Int8, int8_t);
	DEFINE_LUA_NUMBER_PROPERTY(UInt8, uint8_t);
	DEFINE_LUA_NUMBER_PROPERTY(Int16, int16_t);
	DEFINE_LUA_NUMBER_PROPERTY(UInt16, uint16_t);
	DEFINE_LUA_NUMBER_PROPERTY(Int32, int32_t);
	DEFINE_LUA_NUMBER_PROPERTY(UInt32, uint32_t);
	DEFINE_LUA_NUMBER_PROPERTY(Int64, int64_t);
	DEFINE_LUA_NUMBER_PROPERTY(UInt64, uint64_t);

	DEFINE_LUA_NUMBER_PROPERTY(Float, float);
	DEFINE_LUA_NUMBER_PROPERTY(Double, double);
	DEFINE_LUA_NUMBER_PROPERTY(LongDouble, long double);

	//////////

	// Bool
	class LBoolPropertyWrapper : public LSimplePropertyWrapper<pragma::util::BoolProperty, bool> {
	  public:
		LBoolPropertyWrapper(const bool &v) : LSimplePropertyWrapper<pragma::util::BoolProperty, bool> {v} {}
		LBoolPropertyWrapper(const std::shared_ptr<pragma::util::BoolProperty> &v) : LSimplePropertyWrapper<pragma::util::BoolProperty, bool> {v} {}
		LBoolPropertyWrapper() : LSimplePropertyWrapper<pragma::util::BoolProperty, bool> {} {}

		pragma::util::BoolProperty &operator*() { return GetProperty(); }
		const pragma::util::BoolProperty &operator*() const { return const_cast<LBoolPropertyWrapper *>(this)->operator*(); }
		pragma::util::BoolProperty *operator->() { return &GetProperty(); }
		const pragma::util::BoolProperty *operator->() const { return const_cast<LBoolPropertyWrapper *>(this)->operator->(); }

		virtual pragma::util::BoolProperty &GetProperty() const override { return *static_cast<pragma::util::BoolProperty *>(prop.get()); }
	};
	using LBoolProperty = LBoolPropertyWrapper;

	// Color
	class LColorPropertyWrapper : public LSimplePropertyWrapper<pragma::util::ColorProperty, Color> {
	  public:
		pragma::util::ColorProperty &operator*() { return GetProperty(); }
		const pragma::util::ColorProperty &operator*() const { return const_cast<LColorPropertyWrapper *>(this)->operator*(); }
		pragma::util::ColorProperty *operator->() { return &GetProperty(); }
		const pragma::util::ColorProperty *operator->() const { return const_cast<LColorPropertyWrapper *>(this)->operator->(); }

		virtual pragma::util::ColorProperty &GetProperty() const override { return *static_cast<pragma::util::ColorProperty *>(prop.get()); }

		LColorPropertyWrapper() : LSimplePropertyWrapper() {}
		LColorPropertyWrapper(const std::shared_ptr<pragma::util::ColorProperty> &v) : LSimplePropertyWrapper(v) {}
		LColorPropertyWrapper(const Color &col) : LSimplePropertyWrapper(col) {}
		LColorPropertyWrapper(Int16 r, Int16 g, Int16 b, Int16 a) : LSimplePropertyWrapper(Color {r, g, b, a}) {}
		LColorPropertyWrapper(const std::string &str) : LSimplePropertyWrapper(Color {str}) {}
		LColorPropertyWrapper(const Vector3 &v) : LSimplePropertyWrapper(Color {v}) {}
		LColorPropertyWrapper(const Vector4 &v) : LSimplePropertyWrapper(Color {v}) {}
		LColorPropertyWrapper operator/(float f)
		{
			GetProperty() /= f;
			return *this;
		}
		LColorPropertyWrapper operator*(float f)
		{
			GetProperty() *= f;
			return *this;
		}
		LColorPropertyWrapper operator+(const Color &other)
		{
			GetProperty() += other;
			return *this;
		}
		LColorPropertyWrapper operator+(const LColorPropertyWrapper &propOther)
		{
			GetProperty() += *propOther;
			return *this;
		}
		LColorPropertyWrapper operator-(const Color &other)
		{
			GetProperty() -= other;
			return *this;
		}
		LColorPropertyWrapper operator-(const LColorPropertyWrapper &propOther)
		{
			GetProperty() -= *propOther;
			return *this;
		}
	};
	using LColorProperty = LColorPropertyWrapper;

	// Euler Angles
	class LEulerAnglesPropertyWrapper : public LSimplePropertyWrapper<pragma::util::EulerAnglesProperty, EulerAngles> {
	  public:
		pragma::util::EulerAnglesProperty &operator*() { return GetProperty(); }
		const pragma::util::EulerAnglesProperty &operator*() const { return const_cast<LEulerAnglesPropertyWrapper *>(this)->operator*(); }
		pragma::util::EulerAnglesProperty *operator->() { return &GetProperty(); }
		const pragma::util::EulerAnglesProperty *operator->() const { return const_cast<LEulerAnglesPropertyWrapper *>(this)->operator->(); }

		virtual pragma::util::EulerAnglesProperty &GetProperty() const override { return *static_cast<pragma::util::EulerAnglesProperty *>(prop.get()); }

		LEulerAnglesPropertyWrapper() : LSimplePropertyWrapper() {}
		LEulerAnglesPropertyWrapper(const std::shared_ptr<pragma::util::EulerAnglesProperty> &v) : LSimplePropertyWrapper(v) {}
		LEulerAnglesPropertyWrapper(const EulerAngles &col) : LSimplePropertyWrapper(col) {}
		LEulerAnglesPropertyWrapper(float p, float y, float r) : LSimplePropertyWrapper(EulerAngles {p, y, r}) {}
		LEulerAnglesPropertyWrapper(const std::string &str) : LSimplePropertyWrapper(EulerAngles {str}) {}
		LEulerAnglesPropertyWrapper operator/(float f)
		{
			GetProperty() /= f;
			return *this;
		}
		LEulerAnglesPropertyWrapper operator*(float f)
		{
			GetProperty() *= f;
			return *this;
		}
		LEulerAnglesPropertyWrapper operator+(const EulerAngles &other)
		{
			GetProperty() += other;
			return *this;
		}
		LEulerAnglesPropertyWrapper operator+(const LEulerAnglesPropertyWrapper &propOther)
		{
			GetProperty() += *propOther;
			return *this;
		}
		LEulerAnglesPropertyWrapper operator-(const EulerAngles &other)
		{
			GetProperty() -= other;
			return *this;
		}
		LEulerAnglesPropertyWrapper operator-(const LEulerAnglesPropertyWrapper &propOther)
		{
			GetProperty() -= *propOther;
			return *this;
		}
	};
	using LEulerAnglesProperty = LEulerAnglesPropertyWrapper;

	// Transform
	class LTransformPropertyWrapper : public LSimplePropertyWrapper<pragma::util::TransformProperty, pragma::math::Transform> {
	  public:
		pragma::util::TransformProperty &operator*() { return GetProperty(); }
		const pragma::util::TransformProperty &operator*() const { return const_cast<LTransformPropertyWrapper *>(this)->operator*(); }
		pragma::util::TransformProperty *operator->() { return &GetProperty(); }
		const pragma::util::TransformProperty *operator->() const { return const_cast<LTransformPropertyWrapper *>(this)->operator->(); }

		virtual pragma::util::TransformProperty &GetProperty() const override { return *static_cast<pragma::util::TransformProperty *>(prop.get()); }

		LTransformPropertyWrapper() : LSimplePropertyWrapper() {}
		LTransformPropertyWrapper(const std::shared_ptr<pragma::util::TransformProperty> &v) : LSimplePropertyWrapper(v) {}
		LTransformPropertyWrapper(const pragma::math::Transform &col) : LSimplePropertyWrapper(col) {}
		LTransformPropertyWrapper(const Vector3 &pos, const Quat &rot) : LSimplePropertyWrapper(pragma::math::Transform {pos, rot}) {}
		LTransformPropertyWrapper operator*(const pragma::math::Transform &other)
		{
			GetProperty() *= other;
			return *this;
		}
		LTransformPropertyWrapper operator*(const LTransformPropertyWrapper &propOther)
		{
			GetProperty() *= *propOther;
			return *this;
		}
	};
	using LTransformProperty = LTransformPropertyWrapper;

	class LScaledTransformPropertyWrapper : public LSimplePropertyWrapper<pragma::util::ScaledTransformProperty, pragma::math::ScaledTransform> {
	  public:
		pragma::util::ScaledTransformProperty &operator*() { return GetProperty(); }
		const pragma::util::ScaledTransformProperty &operator*() const { return const_cast<LScaledTransformPropertyWrapper *>(this)->operator*(); }
		pragma::util::ScaledTransformProperty *operator->() { return &GetProperty(); }
		const pragma::util::ScaledTransformProperty *operator->() const { return const_cast<LScaledTransformPropertyWrapper *>(this)->operator->(); }

		virtual pragma::util::ScaledTransformProperty &GetProperty() const override { return *static_cast<pragma::util::ScaledTransformProperty *>(prop.get()); }

		LScaledTransformPropertyWrapper() : LSimplePropertyWrapper() {}
		LScaledTransformPropertyWrapper(const std::shared_ptr<pragma::util::ScaledTransformProperty> &v) : LSimplePropertyWrapper(v) {}
		LScaledTransformPropertyWrapper(const pragma::math::ScaledTransform &col) : LSimplePropertyWrapper(col) {}
		LScaledTransformPropertyWrapper(const Vector3 &pos, const Quat &rot, const Vector3 &scale) : LSimplePropertyWrapper(pragma::math::ScaledTransform {pos, rot, scale}) {}
		LScaledTransformPropertyWrapper operator*(const pragma::math::ScaledTransform &other)
		{
			GetProperty() *= other;
			return *this;
		}
		LScaledTransformPropertyWrapper operator*(const LScaledTransformPropertyWrapper &propOther)
		{
			GetProperty() *= *propOther;
			return *this;
		}
	};
	using LScaledTransformProperty = LScaledTransformPropertyWrapper;

	template<class TProperty, typename T>
	class TLVectorPropertyWrapper : public LSimplePropertyWrapper<TProperty, T> {
	  public:
		TLVectorPropertyWrapper() : LSimplePropertyWrapper<TProperty, T>() {}
		TLVectorPropertyWrapper(const T &v) : LSimplePropertyWrapper<TProperty, T>(v) {}
		TLVectorPropertyWrapper(const std::shared_ptr<TProperty> &v) : LSimplePropertyWrapper<TProperty, T>(v) {}

		TProperty &operator*() { return GetProperty(); }
		const TProperty &operator*() const { return const_cast<TLVectorPropertyWrapper *>(this)->operator*(); }
		TProperty *operator->() { return &GetProperty(); }
		const TProperty *operator->() const { return const_cast<TLVectorPropertyWrapper *>(this)->operator->(); }

		virtual TProperty &GetProperty() const override { return *static_cast<TProperty *>(this->prop.get()); }
		TLVectorPropertyWrapper<TProperty, T> operator/(float f)
		{
			GetProperty() /= f;
			return *this;
		}
		TLVectorPropertyWrapper<TProperty, T> operator*(float f)
		{
			GetProperty() /= f;
			return *this;
		}
		TLVectorPropertyWrapper<TProperty, T> operator+(const T &other)
		{
			GetProperty() += other;
			return *this;
		}
		TLVectorPropertyWrapper<TProperty, T> operator+(const TLVectorPropertyWrapper<TProperty, T> &propOther)
		{
			GetProperty() += *propOther;
			return *this;
		}
		TLVectorPropertyWrapper<TProperty, T> operator-(const T &other)
		{
			GetProperty() -= other;
			return *this;
		}
		TLVectorPropertyWrapper<TProperty, T> operator-(const TLVectorPropertyWrapper<TProperty, T> &propOther)
		{
			GetProperty() -= *propOther;
			return *this;
		}
	};

	class LVector2PropertyWrapper : public TLVectorPropertyWrapper<pragma::util::Vector2Property, Vector2> {
	  public:
		LVector2PropertyWrapper() : TLVectorPropertyWrapper<pragma::util::Vector2Property, Vector2>() {}
		LVector2PropertyWrapper(const Vector2 &v) : TLVectorPropertyWrapper<pragma::util::Vector2Property, Vector2>(v) {}
		LVector2PropertyWrapper(float x, float y) : TLVectorPropertyWrapper<pragma::util::Vector2Property, Vector2>(Vector2 {x, y}) {}
		LVector2PropertyWrapper(const std::shared_ptr<pragma::util::Vector2Property> &v) : TLVectorPropertyWrapper(v) {}
	};

	class LVector2iPropertyWrapper : public TLVectorPropertyWrapper<pragma::util::Vector2iProperty, Vector2i> {
	  public:
		LVector2iPropertyWrapper() : TLVectorPropertyWrapper<pragma::util::Vector2iProperty, Vector2i>() {}
		LVector2iPropertyWrapper(const Vector2i &v) : TLVectorPropertyWrapper<pragma::util::Vector2iProperty, Vector2i>(v) {}
		LVector2iPropertyWrapper(int32_t x, int32_t y) : TLVectorPropertyWrapper<pragma::util::Vector2iProperty, Vector2i>(Vector2i {x, y}) {}
		LVector2iPropertyWrapper(const std::shared_ptr<pragma::util::Vector2iProperty> &v) : TLVectorPropertyWrapper(v) {}
	};

	class LVector3PropertyWrapper : public TLVectorPropertyWrapper<pragma::util::Vector3Property, Vector3> {
	  public:
		LVector3PropertyWrapper() : TLVectorPropertyWrapper<pragma::util::Vector3Property, Vector3>() {}
		LVector3PropertyWrapper(const Vector3 &v) : TLVectorPropertyWrapper<pragma::util::Vector3Property, Vector3>(v) {}
		LVector3PropertyWrapper(float x, float y, float z) : TLVectorPropertyWrapper<pragma::util::Vector3Property, Vector3>(Vector3 {x, y, z}) {}
		LVector3PropertyWrapper(const std::shared_ptr<pragma::util::Vector3Property> &v) : TLVectorPropertyWrapper(v) {}
	};

	class LVector3iPropertyWrapper : public TLVectorPropertyWrapper<pragma::util::Vector3iProperty, Vector3i> {
	  public:
		LVector3iPropertyWrapper() : TLVectorPropertyWrapper<pragma::util::Vector3iProperty, Vector3i>() {}
		LVector3iPropertyWrapper(const Vector3i &v) : TLVectorPropertyWrapper<pragma::util::Vector3iProperty, Vector3i>(v) {}
		LVector3iPropertyWrapper(int32_t x, int32_t y, int32_t z) : TLVectorPropertyWrapper<pragma::util::Vector3iProperty, Vector3i>(Vector3i {x, y, z}) {}
		LVector3iPropertyWrapper(const std::shared_ptr<pragma::util::Vector3iProperty> &v) : TLVectorPropertyWrapper(v) {}
	};

	class LVector4PropertyWrapper : public TLVectorPropertyWrapper<pragma::util::Vector4Property, Vector4> {
	  public:
		LVector4PropertyWrapper() : TLVectorPropertyWrapper<pragma::util::Vector4Property, Vector4>() {}
		LVector4PropertyWrapper(const Vector4 &v) : TLVectorPropertyWrapper<pragma::util::Vector4Property, Vector4>(v) {}
		LVector4PropertyWrapper(float x, float y, float z, float w) : TLVectorPropertyWrapper<pragma::util::Vector4Property, Vector4>(Vector4 {x, y, z, w}) {}
		LVector4PropertyWrapper(const std::shared_ptr<pragma::util::Vector4Property> &v) : TLVectorPropertyWrapper(v) {}
	};

	class LVector4iPropertyWrapper : public TLVectorPropertyWrapper<pragma::util::Vector4iProperty, Vector4i> {
	  public:
		LVector4iPropertyWrapper() : TLVectorPropertyWrapper<pragma::util::Vector4iProperty, Vector4i>() {}
		LVector4iPropertyWrapper(const Vector4i &v) : TLVectorPropertyWrapper<pragma::util::Vector4iProperty, Vector4i>(v) {}
		LVector4iPropertyWrapper(int32_t x, int32_t y, int32_t z, int32_t w) : TLVectorPropertyWrapper<pragma::util::Vector4iProperty, Vector4i>(Vector4i {x, y, z, w}) {}
		LVector4iPropertyWrapper(const std::shared_ptr<pragma::util::Vector4iProperty> &v) : TLVectorPropertyWrapper(v) {}
	};

#define DEFINE_LUA_VECTOR_PROPERTY(TYPE, UNDERLYING_PROP_TYPE, UNDERLYING_TYPE)                                                                                                                                                                                                                  \
	using L##TYPE##Property = L##TYPE##PropertyWrapper;                                                                                                                                                                                                                                          \
	UNDERLYING_PROP_TYPE operator+(const UNDERLYING_PROP_TYPE &v, const L##TYPE##Property &prop) { return v + prop->GetValue(); }                                                                                                                                                                \
	UNDERLYING_PROP_TYPE operator-(const UNDERLYING_PROP_TYPE &v, const L##TYPE##Property &prop) { return v - prop->GetValue(); }                                                                                                                                                                \
	UNDERLYING_PROP_TYPE operator*(const UNDERLYING_PROP_TYPE &v, const L##TYPE##Property &prop) { return v * prop->GetValue(); }                                                                                                                                                                \
	UNDERLYING_PROP_TYPE operator/(const UNDERLYING_PROP_TYPE &v, const L##TYPE##Property &prop) { return v / prop->GetValue(); }                                                                                                                                                                \
	UNDERLYING_PROP_TYPE operator*(UNDERLYING_TYPE v, const L##TYPE##Property &prop) { return v * prop->GetValue(); }                                                                                                                                                                            \
	bool operator==(const UNDERLYING_PROP_TYPE &v, const L##TYPE##Property &prop) { return **prop == v; }                                                                                                                                                                                        \
	std::ostream &operator<<(std::ostream &str, const L##TYPE##Property &v) { return str << **v; }

	DEFINE_LUA_VECTOR_PROPERTY(Vector2, Vector2, float);
	DEFINE_LUA_VECTOR_PROPERTY(Vector2i, Vector2i, int32_t);
	DEFINE_LUA_VECTOR_PROPERTY(Vector3, Vector3, float);
	DEFINE_LUA_VECTOR_PROPERTY(Vector3i, Vector3i, int32_t);
	DEFINE_LUA_VECTOR_PROPERTY(Vector4, Vector4, float);
	DEFINE_LUA_VECTOR_PROPERTY(Vector4i, Vector4i, int32_t);

	// Quaternion
	class LQuatPropertyWrapper : public LSimplePropertyWrapper<pragma::util::QuatProperty, Quat> {
	  public:
		pragma::util::QuatProperty &operator*() { return GetProperty(); }
		const pragma::util::QuatProperty &operator*() const { return const_cast<LQuatPropertyWrapper *>(this)->operator*(); }
		pragma::util::QuatProperty *operator->() { return &GetProperty(); }
		const pragma::util::QuatProperty *operator->() const { return const_cast<LQuatPropertyWrapper *>(this)->operator->(); }

		virtual pragma::util::QuatProperty &GetProperty() const override { return *static_cast<pragma::util::QuatProperty *>(this->prop.get()); }

		LQuatPropertyWrapper() : LSimplePropertyWrapper<pragma::util::QuatProperty, Quat>(uquat::identity()) {}
		LQuatPropertyWrapper(const Quat &v) : LSimplePropertyWrapper<pragma::util::QuatProperty, Quat>(v) {}
		LQuatPropertyWrapper(float w, float x, float y, float z) : LSimplePropertyWrapper<pragma::util::QuatProperty, Quat>(Quat {w, x, y, z}) {}
		LQuatPropertyWrapper(const std::string &str) : LSimplePropertyWrapper<pragma::util::QuatProperty, Quat>(uquat::create(str)) {}
		LQuatPropertyWrapper(const std::shared_ptr<pragma::util::QuatProperty> &v) : LSimplePropertyWrapper(v) {}
		LQuatPropertyWrapper operator/(float f)
		{
			GetProperty() /= f;
			return *this;
		}
		LQuatPropertyWrapper operator*(float f)
		{
			GetProperty() *= f;
			return *this;
		}
		LQuatPropertyWrapper operator*(const Quat &other)
		{
			GetProperty() *= other;
			return *this;
		}
		LQuatPropertyWrapper operator*(const LQuatPropertyWrapper &propOther)
		{
			GetProperty() *= *propOther;
			return *this;
		}
	};
	using LQuatProperty = LQuatPropertyWrapper;

	// String
	class LStringPropertyWrapper : public LSimplePropertyWrapper<pragma::util::StringProperty, std::string> {
	  public:
		pragma::util::StringProperty &operator*() { return GetProperty(); }
		const pragma::util::StringProperty &operator*() const { return const_cast<LStringPropertyWrapper *>(this)->operator*(); }
		pragma::util::StringProperty *operator->() { return &GetProperty(); }
		const pragma::util::StringProperty *operator->() const { return const_cast<LStringPropertyWrapper *>(this)->operator->(); }

		virtual pragma::util::StringProperty &GetProperty() const override { return *static_cast<pragma::util::StringProperty *>(this->prop.get()); }

		LStringPropertyWrapper() : LSimplePropertyWrapper<pragma::util::StringProperty, std::string>() {}
		LStringPropertyWrapper(const std::string &v) : LSimplePropertyWrapper<pragma::util::StringProperty, std::string>(v) {}
		LStringPropertyWrapper(const std::shared_ptr<pragma::util::StringProperty> &v) : LSimplePropertyWrapper<pragma::util::StringProperty, std::string>(v) {}
	};
	using LStringProperty = LStringPropertyWrapper;

	// Matrix
	template<class TProperty, typename T>
	class TLMatrixPropertyWrapper : public LSimplePropertyWrapper<TProperty, T> {
	  public:
		TProperty &operator*() { return GetProperty(); }
		const TProperty &operator*() const { return const_cast<TLMatrixPropertyWrapper *>(this)->operator*(); }
		TProperty *operator->() { return &GetProperty(); }
		const TProperty *operator->() const { return const_cast<TLMatrixPropertyWrapper *>(this)->operator->(); }

		virtual TProperty &GetProperty() const override { return *static_cast<TProperty *>(this->prop.get()); }

		TLMatrixPropertyWrapper() : LSimplePropertyWrapper<TProperty, T>() {}
		TLMatrixPropertyWrapper(const T &v) : LSimplePropertyWrapper<TProperty, T>(v) {}
		TLMatrixPropertyWrapper(const std::shared_ptr<TProperty> &v) : LSimplePropertyWrapper<TProperty, T>(v) {}
	};
	using LMatrix2PropertyWrapper = TLMatrixPropertyWrapper<pragma::util::Matrix2Property, Mat2>;
	using LMatrix2x3PropertyWrapper = TLMatrixPropertyWrapper<pragma::util::Matrix2x3Property, Mat2x3>;
	using LMatrix3x2PropertyWrapper = TLMatrixPropertyWrapper<pragma::util::Matrix3x2Property, Mat3x2>;
	using LMatrix3PropertyWrapper = TLMatrixPropertyWrapper<pragma::util::Matrix3Property, Mat3>;
	using LMatrix3x4PropertyWrapper = TLMatrixPropertyWrapper<pragma::util::Matrix3x4Property, Mat3x4>;
	using LMatrix4x3PropertyWrapper = TLMatrixPropertyWrapper<pragma::util::Matrix4x3Property, Mat4x3>;
	using LMatrix4PropertyWrapper = TLMatrixPropertyWrapper<pragma::util::Matrix4Property, Mat4>;
	using LMatrix2Property = LMatrix2PropertyWrapper;
	using LMatrix2x3Property = LMatrix2x3PropertyWrapper;
	using LMatrix3x2Property = LMatrix3x2PropertyWrapper;
	using LMatrix3Property = LMatrix3PropertyWrapper;
	using LMatrix3x4Property = LMatrix3x4PropertyWrapper;
	using LMatrix4x3Property = LMatrix4x3PropertyWrapper;
	using LMatrix4Property = LMatrix4PropertyWrapper;

	namespace Lua {
		namespace Property {
			DLLNETWORK void register_classes(Interface &l);

			DLLNETWORK void push(lua::State *l, pragma::util::Int8Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::UInt8Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Int16Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::UInt16Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Int32Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::UInt32Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Int64Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::UInt64Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::FloatProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::DoubleProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::LongDoubleProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::BoolProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::ColorProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::EulerAnglesProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Vector2Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Vector2iProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Vector3Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Vector3iProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Vector4Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Vector4iProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::QuatProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::StringProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::EntityProperty &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Matrix2Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Matrix2x3Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Matrix3x2Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Matrix3Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Matrix3x4Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Matrix4x3Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::Matrix4Property &prop);
			DLLNETWORK void push(lua::State *l, pragma::util::BaseProperty &prop);

			template<typename TEnum>
			void push(lua::State *l, pragma::util::TEnumProperty<TEnum> &prop)
			{
				Lua::Push<LGenericIntPropertyWrapper>(l, LGenericIntPropertyWrapper(std::static_pointer_cast<pragma::util::TEnumProperty<TEnum>>(prop.shared_from_this())));
			}
			template<class TProperty, typename T>
			void add_callback(lua::State *l, TProperty &prop, const luabind::object &oCallback)
			{
				CheckFunction(l, 2);
				auto cb = prop->AddCallback([l, oCallback](std::reference_wrapper<const T> oldVal, std::reference_wrapper<const T> newVal) {
					auto c = Lua::CallFunction(
					  l,
					  [&oCallback, &oldVal, &newVal](lua::State *l) -> StatusCode {
						  oCallback.push(l);
						  Lua::Push<T>(l, oldVal.get());
						  Lua::Push<T>(l, newVal.get());
						  return StatusCode::Ok;
					  },
					  0);
				});
				Lua::Push<CallbackHandle>(l, cb);
			}

			template<class TProperty, typename T>
			void add_modifier(lua::State *l, TProperty &prop, const luabind::object &oCallback)
			{
				CheckFunction(l, 2);
				auto cb = prop->AddModifier([l, oCallback](T &val) {
					auto c = Lua::CallFunction(
					  l,
					  [&oCallback, &val](lua::State *l) -> StatusCode {
						  oCallback.push(l);
						  Lua::Push<T>(l, val);
						  return StatusCode::Ok;
					  },
					  1);
					if(c == StatusCode::Ok && IsSet(l, -1))
						val = Lua::Check<T>(l, -1);
				});
				Lua::Push<CallbackHandle>(l, cb);
			}

			template<class TProperty, typename T>
			void unlink(lua::State *l, TProperty &prop)
			{
				prop->Unlink();
			}

			template<class TProperty, typename T>
			void get(lua::State *l, TProperty &prop)
			{
				Lua::Push<T>(l, prop->GetValue());
			}

			template<class TProperty, typename T>
			void set(lua::State *l, TProperty &prop, const T &val)
			{
				prop->operator=(val);
			}

			template<class TProperty, typename T>
			void set_locked(lua::State *l, TProperty &prop, bool bLocked)
			{
				prop->SetLocked(bLocked);
			}

			template<class TProperty, typename T>
			void is_locked(lua::State *l, TProperty &prop)
			{
				Lua::PushBool(l, prop->IsLocked());
			}

			template<class TProperty, typename T>
			void invoke_callbacks(lua::State *l, TProperty &prop)
			{
				prop->InvokeCallbacks();
			}
			template<class TProperty, typename T, class TLuaClass>
			void add_generic_methods(TLuaClass &classDef)
			{
				classDef.def("AddCallback", static_cast<void (*)(lua::State *, TProperty &, const luabind::object &)>(add_callback<TProperty, T>));
				classDef.def("AddModifier", static_cast<void (*)(lua::State *, TProperty &, const luabind::object &)>(add_modifier<TProperty, T>));
				classDef.def("Unlink", static_cast<void (*)(lua::State *, TProperty &)>(unlink<TProperty, T>));
				classDef.def("Get", static_cast<void (*)(lua::State *, TProperty &)>(get<TProperty, T>));
				classDef.def("Set", static_cast<void (*)(lua::State *, TProperty &, const T &)>(set<TProperty, T>));
				classDef.def("SetLocked", static_cast<void (*)(lua::State *, TProperty &, bool)>(set_locked<TProperty, T>));
				classDef.def("IsLocked", static_cast<void (*)(lua::State *, TProperty &)>(is_locked<TProperty, T>));
				classDef.def("InvokeCallbacks", static_cast<void (*)(lua::State *, TProperty &)>(invoke_callbacks<TProperty, T>));
			}
			template<class TProperty, typename T, class TLinkProperty>
			void link_different(lua::State *l, TProperty &prop, TLinkProperty &propOther)
			{
				prop->Link(*propOther);
			}

			template<class TProperty, typename T>
			void link(lua::State *l, TProperty &prop, TProperty &propOther)
			{
				link_different<TProperty, T, TProperty>(l, prop, propOther);
			}
			template<class TWrapper, class TProperty>
			void push_property(lua::State *l, TProperty &prop)
			{
				Lua::Push<TWrapper>(l, TWrapper(std::static_pointer_cast<TProperty>(prop.shared_from_this())));
			}
		};
	};
};
