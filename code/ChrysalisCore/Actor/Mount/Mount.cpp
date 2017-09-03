#include <StdAfx.h>

#include "Mount.h"

namespace Chrysalis
{
// Definition of the state machine that controls character movement.
//DEFINE_STATE_MACHINE(CMountComponent, Movement);


void CMountComponent::Register(Schematyc::CEnvRegistrationScope& componentScope)
{
}


void CMountComponent::ReflectType(Schematyc::CTypeDesc<CMountComponent>& desc)
{
	desc.SetGUID(CMountComponent::IID());
	desc.SetEditorCategory("Actors");
	desc.SetLabel("Mount");
	desc.SetDescription("No description.");
	desc.SetIcon("icons:ObjectTypes/light.ico");
	desc.SetComponentFlags({ IEntityComponent::EFlags::Transform });
}


void CMountComponent::Initialize()
{
	const auto pEntity = GetEntity();

	// Get it into a known state.
	OnResetState();
}


void CMountComponent::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		// Physicalize on level start for Launcher
		case ENTITY_EVENT_START_LEVEL:

			// Editor specific, physicalize on reset, property change or transform change
		case ENTITY_EVENT_RESET:
		case ENTITY_EVENT_EDITOR_PROPERTY_CHANGED:
		case ENTITY_EVENT_XFORM_FINISHED_EDITOR:
			OnResetState();
			break;

		case ENTITY_EVENT_UPDATE:
			break;

		case ENTITY_EVENT_PREPHYSICSUPDATE:
			PrePhysicsUpdate();
			break;
	}
}


void CMountComponent::PrePhysicsUpdate()
{
	// TODO: HACK: Copy and update the function from CMountComponent when time allows. This is strictly for test purposes
	// for now. 
}


void CMountComponent::OnResetState()
{
}


//// ***
//// *** Hierarchical State Machine Support
//// ***
//
//
//void CMountComponent::SelectMovementHierarchy()
//{
//	//StateMachineHandleEventMovement(ACTOR_EVENT_ENTRY);
//}
//
//
//void CMountComponent::MovementHSMRelease()
//{
//	//StateMachineReleaseMovement();
//}
//
//
//void CMountComponent::MovementHSMInit()
//{
//	//StateMachineInitMovement();
//}
//
//
//void CMountComponent::MovementHSMSerialize(TSerialize ser)
//{
//	//StateMachineSerializeMovement(SStateEventSerialize(ser));
//}
//
//
//void CMountComponent::MovementHSMUpdate(SEntityUpdateContext& ctx, int updateSlot)
//{
//	//StateMachineUpdateMovement(ctx.fFrameTime, false);
//
//	// Pass the update into the movement state machine.
//	// TODO: make this happen.
//	//StateMachineHandleEventMovement(SStateEventUpdate(ctx.fFrameTime));
//}
//
//
//void CMountComponent::MovementHSMReset()
//{
//	//StateMachineResetMovement();
//}
}