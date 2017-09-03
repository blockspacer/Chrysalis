#pragma once

#include "ICameraComponent.h"
#include <CrySystem/VR/IHMDDevice.h>
#include <CrySystem/VR/IHMDManager.h>
#include "../Camera/CameraManagerComponent.h"


namespace Chrysalis
{
/**
An camera used to get up close to entities allowing the player to examine them more closely than might be
possible with the default view.
**/
class CExamineCameraComponent
	: public ICameraComponent
{
protected:
	friend CChrysalisCorePlugin;
	static void Register(Schematyc::CEnvRegistrationScope& componentScope);

	// IEntityComponent
	void Initialize() override;
	void ProcessEvent(SEntityEvent& event) override;
	uint64 GetEventMask() const { return m_EventMask; }
	void OnShutDown() override;
	// ~IEntityComponent

public:
	CExamineCameraComponent() {}
	virtual ~CExamineCameraComponent() {}

	static void ReflectType(Schematyc::CTypeDesc<CExamineCameraComponent>& desc);

	static CryGUID& IID()
	{
		static CryGUID id = "{D2401F40-9A16-40F3-8414-794B5E2B79AB}"_cry_guid;
		return id;
	}

	void Update();


	// ***
	// *** ICameraComponent
	// ***


	/**
	Switch which entity the camera will treat as it's attachment.

	\param	entityID	If non-null, the entity.
	*/
	void AttachToEntity(EntityId entityId) override;

	/** Executes the activate action. */
	void OnActivate() override;

	/** Executes the deactivate action. */
	void OnDeactivate() override;


	// ***
	// *** CExamineRPGCamera
	// ***

public:
	virtual void ResetCamera();


private:
	/** If our entity has a camera manager, we store a pointer to it here. **/
	CCameraManagerComponent* m_pCameraManager { nullptr };

	/** Identifier for the entity which this camera is targeted towards. */
	EntityId m_targetEntityID { INVALID_ENTITYID };

	/** A delta value (degrees) to apply to the camera's initial calculated pitch. */
	float m_viewPitch;

	/** A delta value (degrees) to apply to the camera's initial calculated yaw. */
	float m_viewYaw;

	/** Provides a way to avoid updates when they are not required. **/
	uint64 m_EventMask { 0L };
};
}