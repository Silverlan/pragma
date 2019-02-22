#ifndef __S_ENT_DEBUG_HPP__
#define __S_ENT_DEBUG_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"

class DLLSERVER EntDebugText
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

////////////////

class DLLSERVER EntDebugPoint
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

////////////////

class DLLSERVER EntDebugLine
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

////////////////

class DLLSERVER EntDebugBox
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

////////////////

class DLLSERVER EntDebugSphere
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

////////////////

class DLLSERVER EntDebugCone
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

////////////////

class DLLSERVER EntDebugCylinder
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

////////////////

class DLLSERVER EntDebugPlane
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
