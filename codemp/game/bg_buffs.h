#pragma once

/////////////////////////////////////////////////////////
//
//	Jedi Knight Galaxies - Buff System
//	Buffs have an intensity and duration attached to them.

#include "qcommon/q_shared.h"

#define MAX_BUFF_COUNT	255

/*
=================================
Damage Buff Structure

Intensity modifies the damage amount.
Only needed on the server.
=================================
*/
struct jkgBuffDamageData_t
{
	int damage;					// amount of damage to do each tick (modified by intensity)
	int means;					// means of damage (damage type)
	int tick;					// how many milliseconds between each damage tick
};

/*
=================================
Visual Buff Structure

Only present on the client.
=================================
*/
struct jkgBuffEffectData_t
{
	char effectName[MAX_QPATH];	// the effect to bolt on
	char effectBolt[MAX_QPATH];	// which bone on the skeleton to bolt onto
	int tick;					// how many MS between playing the effect
};

struct jkgBuffVisualData_t
{
	char screenShader[MAX_QPATH];				// Draw a shader over the screen while this buff is active
	char applySound[MAX_QPATH];					// Play a sound when this buff is applied
	char cancelSound[MAX_QPATH];				// Play a sound when this buff is canceled by something else
	char fadeSound[MAX_QPATH];					// Play a sound when this buff's duration runs out
	char bodyShader[MAX_QPATH];					// Apply a fullbody shader to the player while this buff is active
	std::vector<jkgBuffEffectData_t> boltedOnEffects;	// EFX to play while the buff is active
};

/*
=================================
Canceling Buff Structures

Explains how buffs cancel each other out.
Buffs with a higher priority will cancel out buffs in the same category.
Buffs can also cancel out buffs in *other categories*
=================================
*/
struct jkgBuffCancelCategory_t
{
	char category[32]; // tries to cancel out all active buffs of this category
	int priority; // won't cancel out the buff unless the cancel priority is higher (no priority listed = infinite priority)
};

struct jkgBuffCancelingData_t
{
	qboolean noBuffStack;	// if true, this buff will try to cancel out other buffs of the same category
	int priority;			// buffs with higher priority cancel out lower buffs in the same category

	std::vector<jkgBuffCancelCategory_t> extraCancel; // try to cancel out other categories (example, fire cancels out ice)
};

/*
=================================
Passive Buff Structure

These are basically stats, not damage.
=================================
*/
struct jkgBuffMeanModifier_t
{
	int means;	// the means of damage to modify
	float modifier;	// how much to modify the damage by
};

struct jkgBuffPassiveData_t
{
	float movementSpeedModifier;	// defaults to 1.0 (not relevant with some kinds of pmoveTypes)
	int pmoveType;					// defaults to PM_WALKING

	std::vector<jkgBuffMeanModifier_t> meansModifiers;	// modifies the amount of damage done by certain damage types
};

/*
=================================
Overall Buff Structure
=================================
*/

struct jkgBuff_t
{
	char internalName[64];	// used for linkage
	char category[32]; // which buff category this belongs to

	jkgBuffDamageData_t damage;
	jkgBuffVisualData_t visuals;
	jkgBuffCancelingData_t canceling;
	jkgBuffPassiveData_t passive;
};