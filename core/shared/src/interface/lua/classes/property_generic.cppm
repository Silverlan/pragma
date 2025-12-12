// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.classes.property_generic;

export import pragma.util;

export {
	enum class ArithmeticFloatPropertyType : uint32_t { Float = 0u, Double, LongDouble };

	enum class ArithmeticIntPropertyType : uint32_t { Int8 = 0u, UInt8, Int16, UInt16, Int32, UInt32, Int64, UInt64 };

	class LGenericIntPropertyWrapper;
	class LGenericFloatPropertyWrapper {
	  public:
		friend LGenericIntPropertyWrapper;
		LGenericFloatPropertyWrapper();
		LGenericFloatPropertyWrapper(double v);
		LGenericFloatPropertyWrapper(const pragma::util::PFloatProperty &prop);
		LGenericFloatPropertyWrapper(const pragma::util::PDoubleProperty &prop);
		LGenericFloatPropertyWrapper(const pragma::util::PLongDoubleProperty &prop);

		LGenericFloatPropertyWrapper *operator->();
		CallbackHandle AddCallback(const std::function<void(std::reference_wrapper<const double>, std::reference_wrapper<const double>)> &callback);
		CallbackHandle AddModifier(const std::function<void(std::reference_wrapper<double>)> &fModifier);
		void InvokeCallbacks();
		template<class TProperty>
		void Link(TProperty &prop)
		{
			switch(m_propertyType) {
			case ArithmeticFloatPropertyType::Float:
				static_cast<pragma::util::FloatProperty &>(*m_property).Link(prop);
				break;
			case ArithmeticFloatPropertyType::Double:
				static_cast<pragma::util::DoubleProperty &>(*m_property).Link(prop);
				break;
			case ArithmeticFloatPropertyType::LongDouble:
				static_cast<pragma::util::LongDoubleProperty &>(*m_property).Link(prop);
				break;
			}
		}
		template<class TProperty>
		void LinkOther(TProperty &prop)
		{
			switch(m_propertyType) {
			case ArithmeticFloatPropertyType::Float:
				prop.Link(static_cast<pragma::util::FloatProperty &>(*m_property));
				break;
			case ArithmeticFloatPropertyType::Double:
				prop.Link(static_cast<pragma::util::DoubleProperty &>(*m_property));
				break;
			case ArithmeticFloatPropertyType::LongDouble:
				prop.Link(static_cast<pragma::util::LongDoubleProperty &>(*m_property));
				break;
			}
		}
		void LinkOther(pragma::util::StringProperty &prop);
		void Link(pragma::util::StringProperty &prop);
		void Link(LGenericFloatPropertyWrapper &prop);
		void Link(LGenericIntPropertyWrapper &prop);
		void Unlink();
		float GetValue() const;
		void SetValue(float f);
		LGenericFloatPropertyWrapper &operator=(const double &other);
		void SetLocked(bool bLocked, bool bLinkLocked = true);
		bool IsLocked() const;

		// Arithmetic
		LGenericFloatPropertyWrapper operator+(const double &val) const;
		LGenericFloatPropertyWrapper operator+(const LGenericFloatPropertyWrapper &propOther) const;

		LGenericFloatPropertyWrapper operator-(const double &val) const;
		LGenericFloatPropertyWrapper operator-(const LGenericFloatPropertyWrapper &propOther) const;

		LGenericFloatPropertyWrapper operator*(const double &val) const;
		LGenericFloatPropertyWrapper operator*(const LGenericFloatPropertyWrapper &propOther) const;

		LGenericFloatPropertyWrapper operator/(const double &val) const;
		LGenericFloatPropertyWrapper operator/(const LGenericFloatPropertyWrapper &propOther) const;

		bool operator==(const double &val) const;
		bool operator==(const LGenericFloatPropertyWrapper &propOther) const;

		// Comparison
		bool operator<(const double &val) const;
		bool operator<(const LGenericFloatPropertyWrapper &propOther) const;
		bool operator<=(const double &val) const;
		bool operator<=(const LGenericFloatPropertyWrapper &propOther) const;
	  private:
		std::shared_ptr<pragma::util::BaseProperty> m_property = nullptr;
		ArithmeticFloatPropertyType m_propertyType;
	};

	class LGenericIntPropertyWrapper {
	  public:
		friend LGenericFloatPropertyWrapper;
		LGenericIntPropertyWrapper();
		LGenericIntPropertyWrapper(int32_t v);
		LGenericIntPropertyWrapper(const pragma::util::PInt8Property &prop);
		LGenericIntPropertyWrapper(const pragma::util::PUInt8Property &prop);
		LGenericIntPropertyWrapper(const pragma::util::PInt16Property &prop);
		LGenericIntPropertyWrapper(const pragma::util::PUInt16Property &prop);
		LGenericIntPropertyWrapper(const pragma::util::PInt32Property &prop);
		LGenericIntPropertyWrapper(const pragma::util::PUInt32Property &prop);
		LGenericIntPropertyWrapper(const pragma::util::PInt64Property &prop);
		LGenericIntPropertyWrapper(const pragma::util::PUInt64Property &prop);
		template<typename TEnum>
		LGenericIntPropertyWrapper(const pragma::util::PEnumProperty<TEnum> &prop) : m_property(prop)
		{
			// Note: The implementation of pragma::util::TEnumProperty<TEnum> has to be exactly
			// the same as the implementation of the property class for the underlying integral
			// type, because it will be cast into that type.
			// TODO: HACK: Find a better way to do this!
			if constexpr(std::is_same_v<std::underlying_type_t<TEnum>, int8_t>) {
				m_propertyType = ArithmeticIntPropertyType::Int8;
				static_assert(sizeof(pragma::util::TEnumProperty<TEnum>) == sizeof(pragma::util::Int8Property), "Implementation of pragma::util::TEnumProperty<TEnum> and pragma::util::Int8Property have to be the same!");
			}
			else if constexpr(std::is_same_v<std::underlying_type_t<TEnum>, uint8_t>) {
				m_propertyType = ArithmeticIntPropertyType::UInt8;
				static_assert(sizeof(pragma::util::TEnumProperty<TEnum>) == sizeof(pragma::util::UInt8Property), "Implementation of pragma::util::TEnumProperty<TEnum> and pragma::util::UInt8Property have to be the same!");
			}
			else if constexpr(std::is_same_v<std::underlying_type_t<TEnum>, int16_t>) {
				m_propertyType = ArithmeticIntPropertyType::Int16;
				static_assert(sizeof(pragma::util::TEnumProperty<TEnum>) == sizeof(pragma::util::Int16Property), "Implementation of pragma::util::TEnumProperty<TEnum> and pragma::util::Int16Property have to be the same!");
			}
			else if constexpr(std::is_same_v<std::underlying_type_t<TEnum>, uint16_t>) {
				m_propertyType = ArithmeticIntPropertyType::UInt16;
				static_assert(sizeof(pragma::util::TEnumProperty<TEnum>) == sizeof(pragma::util::UInt16Property), "Implementation of pragma::util::TEnumProperty<TEnum> and pragma::util::UInt16Property have to be the same!");
			}
			else if constexpr(std::is_same_v<std::underlying_type_t<TEnum>, int32_t>) {
				m_propertyType = ArithmeticIntPropertyType::Int32;
				static_assert(sizeof(pragma::util::TEnumProperty<TEnum>) == sizeof(pragma::util::Int32Property), "Implementation of pragma::util::TEnumProperty<TEnum> and pragma::util::Int32Property have to be the same!");
			}
			else if constexpr(std::is_same_v<std::underlying_type_t<TEnum>, uint32_t>) {
				m_propertyType = ArithmeticIntPropertyType::UInt32;
				static_assert(sizeof(pragma::util::TEnumProperty<TEnum>) == sizeof(pragma::util::UInt32Property), "Implementation of pragma::util::TEnumProperty<TEnum> and pragma::util::UInt32Property have to be the same!");
			}
			else if constexpr(std::is_same_v<std::underlying_type_t<TEnum>, int64_t>) {
				m_propertyType = ArithmeticIntPropertyType::Int64;
				static_assert(sizeof(pragma::util::TEnumProperty<TEnum>) == sizeof(pragma::util::Int64Property), "Implementation of pragma::util::TEnumProperty<TEnum> and pragma::util::Int64Property have to be the same!");
			}
			else if constexpr(std::is_same_v<std::underlying_type_t<TEnum>, uint64_t>) {
				m_propertyType = ArithmeticIntPropertyType::UInt64;
				static_assert(sizeof(pragma::util::TEnumProperty<TEnum>) == sizeof(pragma::util::UInt64Property), "Implementation of pragma::util::TEnumProperty<TEnum> and pragma::util::UInt64Property have to be the same!");
			}
			else
				static_assert("Unsupported enum type!");
		}

		LGenericIntPropertyWrapper *operator->();
		CallbackHandle AddCallback(const std::function<void(std::reference_wrapper<const int32_t>, std::reference_wrapper<const int32_t>)> &callback);
		CallbackHandle AddModifier(const std::function<void(std::reference_wrapper<int32_t>)> &fModifier);
		void InvokeCallbacks();
		template<class TProperty>
		void Link(TProperty &prop)
		{
			switch(m_propertyType) {
			case ArithmeticIntPropertyType::Int8:
				static_cast<pragma::util::Int8Property &>(*m_property).Link(prop);
				break;
			case ArithmeticIntPropertyType::UInt8:
				static_cast<pragma::util::UInt8Property &>(*m_property).Link(prop);
				break;
			case ArithmeticIntPropertyType::Int16:
				static_cast<pragma::util::Int16Property &>(*m_property).Link(prop);
				break;
			case ArithmeticIntPropertyType::UInt16:
				static_cast<pragma::util::UInt16Property &>(*m_property).Link(prop);
				break;
			case ArithmeticIntPropertyType::Int32:
				static_cast<pragma::util::Int32Property &>(*m_property).Link(prop);
				break;
			case ArithmeticIntPropertyType::UInt32:
				static_cast<pragma::util::UInt32Property &>(*m_property).Link(prop);
				break;
			case ArithmeticIntPropertyType::Int64:
				static_cast<pragma::util::Int64Property &>(*m_property).Link(prop);
				break;
			case ArithmeticIntPropertyType::UInt64:
				static_cast<pragma::util::UInt64Property &>(*m_property).Link(prop);
				break;
			}
		}
		template<class TProperty>
		void LinkOther(TProperty &prop)
		{
			switch(m_propertyType) {
			case ArithmeticIntPropertyType::Int8:
				prop.Link(static_cast<pragma::util::Int8Property &>(*m_property));
				break;
			case ArithmeticIntPropertyType::UInt8:
				prop.Link(static_cast<pragma::util::UInt8Property &>(*m_property));
				break;
			case ArithmeticIntPropertyType::Int16:
				prop.Link(static_cast<pragma::util::Int16Property &>(*m_property));
				break;
			case ArithmeticIntPropertyType::UInt16:
				prop.Link(static_cast<pragma::util::UInt16Property &>(*m_property));
				break;
			case ArithmeticIntPropertyType::Int32:
				prop.Link(static_cast<pragma::util::Int32Property &>(*m_property));
				break;
			case ArithmeticIntPropertyType::UInt32:
				prop.Link(static_cast<pragma::util::UInt32Property &>(*m_property));
				break;
			case ArithmeticIntPropertyType::Int64:
				prop.Link(static_cast<pragma::util::Int64Property &>(*m_property));
				break;
			case ArithmeticIntPropertyType::UInt64:
				prop.Link(static_cast<pragma::util::UInt64Property &>(*m_property));
				break;
			}
		}
		void LinkOther(pragma::util::StringProperty &prop);
		void Link(pragma::util::StringProperty &prop);
		void Link(LGenericFloatPropertyWrapper &prop);
		void Link(LGenericIntPropertyWrapper &prop);
		void Unlink();
		int32_t GetValue() const;
		void SetValue(int32_t i);
		LGenericIntPropertyWrapper &operator=(const int32_t &other);
		void SetLocked(bool bLocked, bool bLinkLocked = true);
		bool IsLocked() const;

		// Arithmetic
		LGenericIntPropertyWrapper operator+(const int32_t &val) const;
		LGenericIntPropertyWrapper operator+(const LGenericIntPropertyWrapper &propOther) const;

		LGenericIntPropertyWrapper operator-(const int32_t &val) const;
		LGenericIntPropertyWrapper operator-(const LGenericIntPropertyWrapper &propOther) const;

		LGenericIntPropertyWrapper operator*(const int32_t &val) const;
		LGenericIntPropertyWrapper operator*(const LGenericIntPropertyWrapper &propOther) const;

		LGenericIntPropertyWrapper operator/(const int32_t &val) const;
		LGenericIntPropertyWrapper operator/(const LGenericIntPropertyWrapper &propOther) const;

		bool operator==(const int32_t &val) const;
		bool operator==(const LGenericIntPropertyWrapper &propOther) const;

		// Comparison
		bool operator<(const int32_t &val) const;
		bool operator<(const LGenericIntPropertyWrapper &propOther) const;
		bool operator<=(const int32_t &val) const;
		bool operator<=(const LGenericIntPropertyWrapper &propOther) const;
	  private:
		std::shared_ptr<pragma::util::BaseProperty> m_property = nullptr;
		ArithmeticIntPropertyType m_propertyType;
	};

	class LBasePropertyWrapper {
	  public:
		template<class TProperty, typename T>
		void Initialize(const T &v)
		{
			prop = TProperty::Create(v);
		}
		mutable std::shared_ptr<pragma::util::BaseProperty> prop = nullptr;
		virtual pragma::util::BaseProperty &GetProperty() const { return *prop; }
	  protected:
		LBasePropertyWrapper() = default;
	};

	template<class TProperty, typename T>
	class LSimplePropertyWrapper : public LBasePropertyWrapper {
	  public:
		LSimplePropertyWrapper(const T &v) : LBasePropertyWrapper() { Initialize<TProperty, T>(v); }
		LSimplePropertyWrapper(const std::shared_ptr<TProperty> &v) : LBasePropertyWrapper() { this->prop = v; }
		LSimplePropertyWrapper() : LSimplePropertyWrapper(T {}) {}
		TProperty &operator=(const T &val) { GetProperty() = val; }
		TProperty &operator=(const LSimplePropertyWrapper<TProperty, T> &other) { GetProperty() = other; }
		bool operator==(const T &val) const { return GetProperty() == val; }
		bool operator==(const LSimplePropertyWrapper<TProperty, T> &prop) const { return GetProperty() == *prop; }

		const T &GetValue() const { return const_cast<LSimplePropertyWrapper<TProperty, T> *>(this)->GetValue(); }
		T &GetValue() { return **static_cast<TProperty *>(this->prop.get()); }

		void SetValue(const T &v) { *static_cast<TProperty *>(this->prop.get()) = v; }

		TProperty &operator*() { return GetProperty(); }
		const TProperty &operator*() const { return const_cast<LSimplePropertyWrapper<TProperty, T> *>(this)->operator*(); }
		TProperty *operator->() { return &GetProperty(); }
		const TProperty *operator->() const { return const_cast<LSimplePropertyWrapper<TProperty, T> *>(this)->operator->(); }

		virtual TProperty &GetProperty() const override { return *static_cast<TProperty *>(this->prop.get()); }
	};

	template<class TProperty, typename T>
	class TLNumberPropertyWrapper : public LSimplePropertyWrapper<TProperty, T> {
	  public:
		TProperty &operator*() { return GetProperty(); }
		const TProperty &operator*() const { return GetProperty(); }
		TProperty *operator->() { return &GetProperty(); }
		const TProperty *operator->() const { return &GetProperty(); }

		virtual TProperty &GetProperty() const override { return *static_cast<TProperty *>(this->prop.get()); }

		// Arithmetic
		TLNumberPropertyWrapper<TProperty, T> operator+(const T &val) const
		{
			GetProperty() += val;
			return *this;
		}
		TLNumberPropertyWrapper<TProperty, T> operator+(const TLNumberPropertyWrapper<TProperty, T> &propOther) const
		{
			GetProperty() += *propOther;
			return *this;
		}
		TLNumberPropertyWrapper<TProperty, T> operator-(const T &val) const
		{
			GetProperty() -= val;
			return *this;
		}
		TLNumberPropertyWrapper<TProperty, T> operator-(const TLNumberPropertyWrapper<TProperty, T> &propOther) const
		{
			GetProperty() -= *propOther;
			return *this;
		}

		TLNumberPropertyWrapper<TProperty, T> operator*(const T &val) const
		{
			GetProperty() *= val;
			return *this;
		}
		TLNumberPropertyWrapper<TProperty, T> operator*(const TLNumberPropertyWrapper<TProperty, T> &propOther) const
		{
			GetProperty() *= *propOther;
			return *this;
		}

		TLNumberPropertyWrapper<TProperty, T> operator/(const T &val) const
		{
			GetProperty() /= val;
			return *this;
		}
		TLNumberPropertyWrapper<TProperty, T> operator/(const TLNumberPropertyWrapper<TProperty, T> &propOther) const
		{
			GetProperty() /= *propOther;
			return *this;
		}

		bool operator==(const T &val) const { return GetProperty() == val; }
		bool operator==(const TLNumberPropertyWrapper<TProperty, T> &propOther) const { return GetProperty() == *propOther; }

		// Comparison
		bool operator<(const T &val) const { return GetProperty() < val; }
		bool operator<(const TLNumberPropertyWrapper<TProperty, T> &propOther) const { return GetProperty() < *propOther; }
		bool operator<=(const T &val) const { return GetProperty() < val; }
		bool operator<=(const TLNumberPropertyWrapper<TProperty, T> &propOther) const { return GetProperty() < *propOther; }
	};
};
