/**
\file	Source\Actor\PlayerCamera\PlayerCamera.h

This is a camera management class. It creates several cameras, one of each type, and then provides features
for switching between the cameras. While the name implies the cameras are just for the player, in general they will
be able to follow any entity in the game.

The host entity will be used as a default for the entity which the camera operates from.
*/
#pragma once

#include <IGameObject.h>
#include "ICameraManagerComponent.h"
#include "ICameraComponent.h"


class CCameraManagerComponent : public CGameObjectExtensionHelper <CCameraManagerComponent, ICameraManagerComponent>
{
public:
	// ***
	// *** ISimpleExtension
	// ***

	void PostInit(IGameObject * pGameObject) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;


	// ***
	// *** ICameraManagerComponent
	// ***

public:

	/**
	Player cameras generally need to follow an actor. This allows us to switch which entity represents the actor that
	the camera is following.

	\param	entityID	If non-null, the entity.
	*/
	void AttachToEntity(EntityId entityID) override;


	/**
	Gets current camera mode.

	\return	The current camera mode.
	*/
	ECameraMode GetCameraMode() override;


	/**
	Sets camera mode.

	\param	mode  	The mode.
	\param	reason	The reason.
	*/
	void SetCameraMode(ECameraMode mode, const char* reason) override;


	/**
	Gets the currently active camera.

	\return	null if it fails, else the camera.
	*/
	ICameraComponent* GetCamera() const override;


	/**
	Gets whether this instance is in third person or not.

	\return	Whether this instance is in third person or not.
	*/
	bool IsThirdPerson() const override;


	/**
	Toggle between the first and third person camera modes, if either is selected.
	*/
	void ToggleThirdPerson() override;


	// ***
	// *** CCameraManagerComponent
	// ***

	struct SExternalCVars
	{
		/** Is the camera in third person mode? */
		int m_isThirdPerson;

		/** Offset the view camera by this vector. */
		ICVar* m_debugViewOffset;
	};
	const SExternalCVars &GetCVars() const;
	SExternalCVars &SetCVars();


	/**
	Constructor.

	\param [in,out]	playerOwner	The player that owns this item.
	*/
	CCameraManagerComponent();

	/** Destructor. */
	virtual ~CCameraManagerComponent();


	/**
	Gets view offset.
	
	\return The view offset.
	**/
	Vec3 GetViewOffset() override;

	/**	Camera debug actions. **/
	bool OnActionCameraShiftUp(EntityId entityId, const ActionId& actionId, int activationMode, float value) override;
	bool OnActionCameraShiftDown(EntityId entityId, const ActionId& actionId, int activationMode, float value) override;
	bool OnActionCameraShiftLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value) override;
	bool OnActionCameraShiftRight(EntityId entityId, const ActionId& actionId, int activationMode, float value) override;
	bool OnActionCameraShiftForward(EntityId entityId, const ActionId& actionId, int activationMode, float value) override;
	bool OnActionCameraShiftBackward(EntityId entityId, const ActionId& actionId, int activationMode, float value) override;


private:
	/**	An array large enough to hold one of each defined camera mode. **/
	ICameraComponent* m_cameraModes [ECameraMode::eCameraMode_Last];

	/** The current camera mode. */
	ECameraMode m_cameraMode { ECameraMode::eCameraMode_NoCamera };

	/** The last camera mode. */
	ECameraMode m_lastCameraMode { ECameraMode::eCameraMode_NoCamera };

	/** The amount each offset tweak will adjust the camera by. **/
	const float adjustmentAmount { 0.025f };

	/** Provides a last moment offset for the view - useful for debugging and player controller extra camera movement. **/
	Vec3 m_interactiveViewOffset { ZERO };

	/** The player. */
	CPlayer* m_pPlayer { nullptr };
};
