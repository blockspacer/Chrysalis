#include <StdAfx.h>

#include "Character.h"
#include <Actor/Movement/ActorMovementController.h>
#include <Actor/Character/Movement/StateMachine/CharacterStateEvents.h>
#include <Actor/Movement/StateMachine/ActorStateUtility.h>
#include <Game/GameFactory.h>


// Definition of the state machine that controls character movement.
DEFINE_STATE_MACHINE(CCharacter, Movement);


class CCharacterRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		CCharacter::Register();
	}
};

CCharacterRegistrator g_CharacterRegistrator;


void CCharacter::Register()
{
	auto properties = new SNativeEntityPropertyInfo [eNumProperties];
	memset(properties, 0, sizeof(SNativeEntityPropertyInfo) * eNumProperties);

	RegisterEntityPropertyObject(properties, eProperty_Model, "Model", "", "Actor model");
	RegisterEntityProperty<float>(properties, eProperty_Mass, "Mass", "82", "Actor mass", 0, 10000);
	RegisterEntityProperty<string>(properties, eProperty_Controller_Definition, "ControllerDefinition", "sdk_tutorial3controllerdefs.xml", "Controller Definition", 0, 0);
	RegisterEntityProperty<string>(properties, eProperty_Scope_Context, "ScopeContext", "MainCharacter", "Scope Context", 0, 0);
	RegisterEntityProperty<string>(properties, eProperty_Animation_Database, "AnimationDatabase", "sdk_tutorial3database.adb", "Animation Database", 0, 0);

	// Finally, register the entity class so that instances can be created later on either via Launcher or Editor
	CGameFactory::RegisterNativeEntity<CCharacter>("Character", "Actors", "Light.bmp", 0u, properties, eNumProperties);

	// Create flownode
	CGameEntityNodeFactory &nodeFactory = CGameFactory::RegisterEntityFlowNode("Character");

	// Define input ports, and the callback function for when they are triggered
	std::vector<SInputPortConfig> inputs;
	inputs.push_back(InputPortConfig_Void("Open", "Open the door"));
	inputs.push_back(InputPortConfig_Void("Close", "Close the door"));
	nodeFactory.AddInputs(inputs, OnFlowgraphActivation);

	// Mark the factory as complete, indicating that there will be no additional ports
	nodeFactory.Close();
}


void CCharacter::OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo *pActInfo, const class CFlowGameEntityNode *pNode)
{
	if (auto pExtension = static_cast<CCharacter *>(QueryExtension(entityId)))
	{
		if (IsPortActive(pActInfo, eInputPort_Open))
		{
			//			pExtension->SetPropertyBool(eProperty_IsOpen, true);
		}
		else if (IsPortActive(pActInfo, eInputPort_Close))
		{
			//			pExtension->SetPropertyBool(eProperty_IsOpen, false);
		}
	}
}


CCharacter::CCharacter()
{
}


CCharacter::~CCharacter()
{
}


// ***
// *** IGameObjectExtension
// ***


void CCharacter::PostInit(IGameObject * pGameObject)
{
	CActor::PostInit(pGameObject);

	// Register for game object events.
	RegisterEvents();

	// Get it into a known state.
	Reset();
}


bool CCharacter::ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params)
{
	CActor::ReloadExtension(pGameObject, params);

	// Register for game object events again.
	RegisterEvents();

	return true;
}


void CCharacter::PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params)
{
	CActor::PostReloadExtension(pGameObject, params);
}


void CCharacter::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	CActor::Update(ctx, updateSlot);
}


void CCharacter::HandleEvent(const SGameObjectEvent& event)
{
	CActor::HandleEvent(event);
}


void CCharacter::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		// Physicalize on level start for Launcher
		case ENTITY_EVENT_START_LEVEL:

			// Editor specific, physicalize on reset, property change or transform change
		case ENTITY_EVENT_RESET:
		case ENTITY_EVENT_EDITOR_PROPERTY_CHANGED:
		case ENTITY_EVENT_XFORM_FINISHED_EDITOR:
			Reset();
			break;

		case ENTITY_EVENT_PREPHYSICSUPDATE:
			PrePhysicsUpdate();
			break;
	}
}


IComponent::ComponentEventPriority CCharacter::GetEventPriority(const int eventID) const
{
	switch (eventID)
	{
		case ENTITY_EVENT_PREPHYSICSUPDATE:
			//			return(ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_Actor + (m_isClient ? EEntityEventPriority_Client : 0));
			return ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_Actor + EEntityEventPriority_Client; // HACK: only used for when we are the client, fix later.
	}

	return IGameObjectExtension::GetEventPriority(eventID);
}


// ***
// *** CCharacter
// ***


bool CCharacter::Physicalize()
{
	return CActor::Physicalize();
}


void CCharacter::PrePhysicsUpdate()
{
	// TODO: HACK: This section of code needs a mix of both IActor / CActor code and CCharacter code, which
	// makes it a bit messy to do with simple inheritance. It needs to be properly refactored at some point,
	// preferably before going on to work on the same function in CMount and CPet.
	// 
	// At some point it will need to call into CActor::PrePhysicsUpdate() and some other similar
	// granular routines.

	const float frameTime = gEnv->pTimer->GetFrameTime();

	if (m_pMovementController)
	{
		SActorMovementRequest movementRequest;

		// Update the movement controller.
		m_pMovementController->UpdateActorMovementRequest(movementRequest, frameTime);

		// They requested a stance change.
		//if (movementRequest.stance != STANCE_NULL)
		//{
		//	SetStance (movementRequest.stance);
		//}

		// Request the movement from the movement controller.
		// NOTE: this one here only cleared the look target previously...searching for other calls to movementrequest.
		//CMovementRequest movementRequest;
		//m_pMovementController->RequestMovement (movementRequest);

		// TODO: process aim?


		// TODO: there used to be code for the player to process turning here. I think it might be better handled more abstractly
		// along with other factors like movement. Sample is provided below.
		// SPlayerRotationParams::EAimType aimType = GetCurrentAimType();
		//m_pPlayerRotation->Process(pCurrentItem, movementRequest,
		//	m_playerRotationParams.m_verticalAims [IsThirdPerson() ? 1 : 0] [aimType],
		//	frameTime);


		// TODO: Not really required just yet.
		//m_characterRotation->Process(movementRequest, frameTime);



		// TODO: call to save data to recording system?



		// The routine will need to be rewritten to work with actors only, or we need a new one that does the actor, that
		// is called by a character version of this.
		CCharacterStateUtil::UpdatePhysicsState(*this, m_actorPhysics, frameTime);




		//#ifdef STATE_DEBUG
		//		if (g_pGameCVars->pl_watchPlayerState >= (bIsClient ? 1 : 2))
		//		{
		//			// NOTE: outputting this info here is 'was happened last frame' not 'what was decided this frame' as it occurs before the prePhysicsEvent is dispatched
		//			// also IsOnGround and IsInAir can possibly both be false e.g. - if you're swimming
		//			// May be able to remove this log now the new HSM debugging is in if it offers the same/improved functionality
		//			CryWatch("%s stance=%s flyMode=%d %s %s%s%s%s", GetEntity()->GetEntityTextDescription(), GetStanceName(GetStance()), m_actorState.flyMode, IsOnGround() ? "ON-GROUND" : "IN-AIR", IsThirdPerson() ? "THIRD-PERSON" : "FIRST-PERSON", IsDead() ? "DEAD" : "ALIVE", m_actorState.isScoped ? " SCOPED" : "", m_actorState.isInBlendRagdoll ? " FALLNPLAY" : "");
		//		}
		//#endif



		// Push the pre-physics event down to our state machine.
		// TODO: This should become an Actor prephysics event instead, so all derived classes can share it.
		const SActorPrePhysicsData prePhysicsData(frameTime, movementRequest);
		const SStateEventActorMovementPrePhysics prePhysicsEvent(&prePhysicsData);
		StateMachineHandleEventMovement(STATE_DEBUG_APPEND_EVENT(prePhysicsEvent));

		//CryWatch("%s : velocity = %f, %f, %f\r\n", __func__, movementRequest.desiredVelocity.x, movementRequest.desiredVelocity.y, movementRequest.desiredVelocity.z);


		// TODO: does stance change really belong here?
		//if (movementRequest.stance != STANCE_NULL)
		//{
		//	SetStance(movementRequest.stance);
		//}

		// Bring the animation state of the character into line with it's requested state.
		// TODO: Make sure this happens - it'c commented out for now!
		UpdateAnimationState(movementRequest);
	}
}


void CCharacter::RegisterEvents()
{
	// Lists the game object events we want to be notified about.
	const int EventsToRegister [] =
	{
		eGFE_OnCollision,			// Collision events.
		eGFE_BecomeLocalPlayer,		// Become client actor events.
		eGFE_OnPostStep,			// Not sure if it's needed for character animation here...but it is required for us to trap that event in this code.
	};

	// Register for the specified game object events.
	GetGameObject()->UnRegisterExtForEvents(this, nullptr, 0);
	GetGameObject()->RegisterExtForEvents(this, EventsToRegister, sizeof(EventsToRegister) / sizeof(int));
}


// ***
// *** Life-cycle
// ***


void CCharacter::Reset()
{
	CActor::Reset();
}


void CCharacter::Kill()
{
	CActor::Kill();
}


void CCharacter::Revive(EReasonForRevive reasonForRevive)
{
	CActor::Revive(reasonForRevive);
}


// ***
// *** Hierarchical State Machine Support
// ***


void CCharacter::SelectMovementHierarchy()
{
	// Force the state machine in the proper hierarchy
	//if (IsAIControlled ())
	//{
	//	CRY_ASSERT (!IsPlayer ());

	//	StateMachineHandleEventMovement (ACTOR_EVENT_ENTRY_AI);
	//}
	//else
	//{
	//	StateMachineHandleEventMovement (ACTOR_EVENT_ENTRY_PLAYER);
	//}

	// HACK: set it to always be player for now.
	// TODO: NEED THIS!!!
	StateMachineHandleEventMovement(ACTOR_EVENT_ENTRY);
}


void CCharacter::MovementHSMRelease()
{
	StateMachineReleaseMovement();
}


void CCharacter::MovementHSMInit()
{
	StateMachineInitMovement();
}


void CCharacter::MovementHSMSerialize(TSerialize ser)
{
	StateMachineSerializeMovement(SStateEventSerialize(ser));
}


void CCharacter::MovementHSMUpdate(SEntityUpdateContext& ctx, int updateSlot)
{
	//#ifdef STATE_DEBUG
	//	const bool shouldDebug = (s_StateMachineDebugEntityID == GetEntityId ());
	//#else
	//	const bool shouldDebug = false;
	//#endif
	const bool shouldDebug = false;
	//const bool shouldDebug = true;

	StateMachineUpdateMovement(ctx.fFrameTime, shouldDebug);

	// Pass the update into the movement state machine.
	// TODO: make this happen.
	StateMachineHandleEventMovement(SStateEventUpdate(ctx.fFrameTime));
}


void CCharacter::MovementHSMReset()
{
	StateMachineResetMovement();
}
