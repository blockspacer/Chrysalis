#include <StdAfx.h>

#include "FirstPersonCameraComponent.h"
#include <IActorSystem.h>
#include <Player/Player.h>
#include <Player/Input/IPlayerInputComponent.h>


class CFirstPersonCameraRegistrator
	: public IEntityRegistrator
	, public CFirstPersonCameraComponent::SExternalCVars
{
	virtual void Register() override
	{
		CGameFactory::RegisterGameObjectExtension<CFirstPersonCameraComponent>("FirstPersonCamera");

		RegisterCVars();
	}

	void RegisterCVars()
	{
		REGISTER_CVAR2("camera_firstperson_Debug", &m_debug, 0, VF_CHEAT, "Allow debug display.");
		REGISTER_CVAR2("camera_firstperson_PitchMin", &m_pitchMin, -85.0f, VF_CHEAT, "Minimum pitch angle of the camera (degrees)");
		REGISTER_CVAR2("camera_firstperson_PitchMax", &m_pitchMax, 85.0f, VF_CHEAT, "Maximum pitch angle of the camera (degrees)");
	}
};

CFirstPersonCameraRegistrator g_firstPersonCameraRegistrator;

// ***
// *** IGameObjectExtension
// ***


void CFirstPersonCameraComponent::PostInit(IGameObject * pGameObject)
{
	// It's a good idea to use the entity as a default for our target entity.
	m_targetEntityID = GetEntityId();

	// Create a new view and link it to this entity.
	auto pViewSystem = gEnv->pGame->GetIGameFramework()->GetIViewSystem();
	m_pView = pViewSystem->CreateView();
	m_pView->LinkTo(GetGameObject());

	// We are usually hosted in the same entity as a camera manager. Use it if you can find one.
	m_pCameraManager = static_cast<ICameraManagerComponent*> (pGameObject->QueryExtension("CameraManager"));
}


bool CFirstPersonCameraComponent::ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params)
{
	ResetGameObject();

	// If we're meant to be active, capture the view.
	OnActivate();

	return true;
}


void CFirstPersonCameraComponent::Release()
{
	// Release the view.
	GetGameObject()->ReleaseView(this);
	gEnv->pGame->GetIGameFramework()->GetIViewSystem()->RemoveView(m_pView);
	m_pView = nullptr;

	delete this;
}


void CFirstPersonCameraComponent::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	auto pPlayerInput = CPlayer::GetLocalPlayer()->GetPlayerInput();

	// Default on failure is to return a cleanly constructed blank camera pose.
	CCameraPose newCameraPose { CCameraPose() };

	// If the player changes the camera zoom, we will toggle to the third person view.
	if (m_pCameraManager)
	{
		if ((pPlayerInput->GetZoomDelta() > FLT_EPSILON) && (!m_pCameraManager->IsThirdPerson()))
			m_pCameraManager->ToggleThirdPerson();
	}

	// Resolve the entity.
	auto pEntity = gEnv->pEntitySystem->GetEntity(m_targetEntityID);
	if (pEntity)
	{
		// It's possible there is no actor to query for eye position, in that case, return a safe default
		// value for an average height person.
		Vec3 localEyePosition { AverageEyePosition };

		// If we are attached to an entity that is an actor we can use their eye position.
		auto pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_targetEntityID);
		if (pActor)
			localEyePosition = pActor->GetLocalEyePos();

		// Apply the player input rotation for this frame, and limit the pitch / yaw movement according to the set max and min values.
		m_viewPitch -= pPlayerInput->GetMousePitchDelta() - pPlayerInput->GetXiPitchDelta();
		m_viewPitch = clamp_tpl(m_viewPitch, DEG2RAD(GetCVars().m_pitchMin), DEG2RAD(GetCVars().m_pitchMax));

		// Pose is based on entity position and the eye position.
		// We will use the rotation of the entity as a base, and apply pitch based on our own reckoning.
		const Vec3 position = pEntity->GetPos() + localEyePosition;
		const Quat rotation = pEntity->GetRotation() * Quat(Ang3(m_viewPitch, 0.0f, 0.0f));
		newCameraPose = CCameraPose(position, rotation);

#if defined(_DEBUG)
		if (GetCVars().m_debug)
		{
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(position, 0.04f, ColorB(0, 0, 255, 255));
		}
#endif
	}

	// We set the pose, regardless of the result.
	SetCameraPose(newCameraPose);
}


// ***
// *** ICameraComponent
// ***


void CFirstPersonCameraComponent::OnActivate()
{
	gEnv->pGame->GetIGameFramework()->GetIViewSystem()->SetActiveView(m_pView);
	GetGameObject()->CaptureView(this);
	GetGameObject()->EnableUpdateSlot(this, CPlayer::EPlayerUpdateSlot::ePlayerUpdateSlot_CameraFirstPerson);
}


void CFirstPersonCameraComponent::OnDeactivate()
{
	GetGameObject()->ReleaseView(this);
	GetGameObject()->DisableUpdateSlot(this, CPlayer::EPlayerUpdateSlot::ePlayerUpdateSlot_CameraFirstPerson);
}


// ***
// *** IGameObjectView
// ***


void CFirstPersonCameraComponent::UpdateView(SViewParams& params)
{
	// The last update call should have given us a new updated position and rotation.
	// We now pass those off to the view system. 
	params.SaveLast();
	params.position = GetCameraPose().GetPosition();
	params.rotation = GetCameraPose().GetRotation();

	// Apply a last minute offset if available for debugging purposes.
	if (m_pCameraManager)
		params.position += GetCameraPose().GetRotation() * m_pCameraManager->GetViewOffset();

	// For simplicity at present we are just hard coding the FoV.
	params.fov = DEG2RAD(60.0f);
}


// ***
// *** CFirstPersonCameraComponent
// ***


const CFirstPersonCameraComponent::SExternalCVars& CFirstPersonCameraComponent::GetCVars() const
{
	return g_firstPersonCameraRegistrator;
}
