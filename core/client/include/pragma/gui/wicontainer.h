#ifndef __WICONTAINER_H__
#define __WICONTAINER_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <unordered_map>

class DLLCLIENT WIContainer
	: public WIBase
{
protected:
	std::array<int32_t,4> m_padding;
public:
	WIContainer();
	enum class DLLCLIENT Padding : uint32_t
	{
		Top = 0,
		Right,
		Bottom,
		Left
	};
	void SetPadding(int32_t padding);
	void SetPadding(int32_t top,int32_t right,int32_t bottom,int32_t left);
	void SetPadding(Padding paddingType,int32_t padding);
	void SetPaddingTop(int32_t top);
	void SetPaddingRight(int32_t right);
	void SetPaddingBottom(int32_t bottom);
	void SetPaddingLeft(int32_t left);
	const std::array<int32_t,4> &GetPadding() const;
	int32_t GetPaddingTop() const;
	int32_t GetPaddingRight() const;
	int32_t GetPaddingBottom() const;
	int32_t GetPaddingLeft() const;
	int32_t GetPadding(Padding paddingType) const;
	virtual void SizeToContents() override;
};

#endif
