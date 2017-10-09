#pragma once

#include <DefaultComponents/Geometry/BaseMeshComponent.h>


class CPlugin_CryDefaultEntities;


namespace Chrysalis
{
class CControlledAnimationComponent
	: public Cry::DefaultComponents::CBaseMeshComponent
{
protected:
	friend CChrysalisCorePlugin;
	static void Register(Schematyc::CEnvRegistrationScope& componentScope);

	// IEntityComponent
	virtual void Initialize() final;
	virtual void ProcessEvent(SEntityEvent& event) final;
	uint64 GetEventMask() const { return Cry::DefaultComponents::CBaseMeshComponent::GetEventMask() | BIT64(ENTITY_EVENT_UPDATE); }
	// ~IEntityComponent

public:
	CControlledAnimationComponent() {}
	virtual ~CControlledAnimationComponent() {}

	static void ReflectType(Schematyc::CTypeDesc<CControlledAnimationComponent>& desc);

	static CryGUID& IID()
	{
		static CryGUID id = "{4902E792-BF53-40F3-B4C3-2FFFCDADD71A}"_cry_guid;
		return id;
	}

	virtual void PlayAnimation(Schematyc::LowLevelAnimationName name);

	virtual void SetPlaybackSpeed(float multiplier) { m_animationParams.m_fPlaybackSpeed = multiplier; }
	virtual void SetPlaybackWeight(float weight) { m_animationParams.m_fPlaybackWeight = weight; }
	virtual void SetLayer(int layer) { m_animationParams.m_nLayerID = layer; }
	virtual void SeekFrame(float frameTime);

	virtual void SetCharacterFile(const char* szPath);
	const char* GetCharacterFile() const { return m_filePath.value.c_str(); }

	virtual void SetDefaultAnimationName(const char* szPath);
	const char* GetDefaultAnimationName() const { return m_defaultAnimation.value.c_str(); }
	
	// Loads character and mannequin data from disk.
	virtual void LoadFromDisk();

	// Applies the character to the entity.
	virtual void ResetObject();

	// Helper to allow exposing derived function to Schematyc.
	virtual void SetMeshType(Cry::DefaultComponents::EMeshType type) { SetType(type); }

protected:
	virtual void Update(SEntityUpdateContext* pCtx);

	CryCharAnimationParams m_animationParams;
	Schematyc::CharacterFileName m_filePath;
	Schematyc::LowLevelAnimationName m_defaultAnimation;
	_smart_ptr<ICharacterInstance> m_pCachedCharacter = nullptr;
	float m_frameTime;
};
}