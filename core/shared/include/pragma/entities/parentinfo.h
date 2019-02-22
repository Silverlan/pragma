#ifndef __PARENTINFO_H__
#define __PARENTINFO_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <pragma/math/angle/wvquaternion.h>
#include "pragma/entities/parentmode.h"
#include <vector>
#include <optional>

class EntityHandle;
namespace pragma
{
	class BaseParentComponent;
};
struct DLLNETWORK AttachmentInfo
{
	std::optional<Vector3> offset = {};
	std::optional<Quat> rotation = {};
	FAttachmentMode flags = FAttachmentMode::None;
};
struct DLLNETWORK AttachmentData
{
	AttachmentData()=default;
	~AttachmentData()=default;
	Vector3 offset = {};
	Quat rotation = uquat::identity();
	FAttachmentMode flags=FAttachmentMode::None;
	int32_t bone = -1;
	int32_t attachment = -1;
	util::WeakHandle<pragma::BaseParentComponent> parent = {};
	std::vector<int32_t> boneMapping = {};
};

#endif