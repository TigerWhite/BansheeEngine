//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAnimation.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptAnimationClip.h"

namespace BansheeEngine
{
	ScriptAnimation::ScriptAnimation(MonoObject* managedInstance)
		:ScriptObject(managedInstance), mAnimation(nullptr)
	{
		mAnimation = Animation::create();
	}

	ScriptAnimation::~ScriptAnimation()
	{ }

	void ScriptAnimation::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptAnimation::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptAnimation::internal_Destroy);

		metaData.scriptClass->addInternalCall("Internal_SetWrapMode", &ScriptAnimation::internal_SetWrapMode);
		metaData.scriptClass->addInternalCall("Internal_SetSpeed", &ScriptAnimation::internal_SetSpeed);

		metaData.scriptClass->addInternalCall("Internal_Play", &ScriptAnimation::internal_Play);
		metaData.scriptClass->addInternalCall("Internal_BlendAdditive", &ScriptAnimation::internal_BlendAdditive);
		metaData.scriptClass->addInternalCall("Internal_Blend1D", &ScriptAnimation::internal_Blend1D);
		metaData.scriptClass->addInternalCall("Internal_Blend2D", &ScriptAnimation::internal_Blend2D);
		metaData.scriptClass->addInternalCall("Internal_CrossFade", &ScriptAnimation::internal_CrossFade);

		metaData.scriptClass->addInternalCall("Internal_Stop", &ScriptAnimation::internal_Stop);
		metaData.scriptClass->addInternalCall("Internal_StopAll", &ScriptAnimation::internal_StopAll);
		metaData.scriptClass->addInternalCall("Internal_IsPlaying", &ScriptAnimation::internal_IsPlaying);

		metaData.scriptClass->addInternalCall("Internal_GetState", &ScriptAnimation::internal_GetState);
		metaData.scriptClass->addInternalCall("Internal_SetState", &ScriptAnimation::internal_SetState);
	}

	void ScriptAnimation::internal_Create(MonoObject* instance)
	{
		new (bs_alloc<ScriptAnimation>()) ScriptAnimation(instance);
	}

	void ScriptAnimation::internal_Destroy(ScriptAnimation* thisPtr)
	{
		thisPtr->mAnimation = nullptr;
	}

	void ScriptAnimation::internal_SetWrapMode(ScriptAnimation* thisPtr, AnimWrapMode wrapMode)
	{
		thisPtr->getInternal()->setWrapMode(wrapMode);
	}

	void ScriptAnimation::internal_SetSpeed(ScriptAnimation* thisPtr, float speed)
	{
		thisPtr->getInternal()->setSpeed(speed);
	}

	void ScriptAnimation::internal_Play(ScriptAnimation* thisPtr, ScriptAnimationClip* clip)
	{
		HAnimationClip nativeClip;
		if (clip != nullptr)
			nativeClip = clip->getHandle();

		thisPtr->getInternal()->play(nativeClip);
	}

	void ScriptAnimation::internal_BlendAdditive(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, float weight, 
		float fadeLength, UINT32 layer)
	{
		HAnimationClip nativeClip;
		if (clip != nullptr)
			nativeClip = clip->getHandle();

		thisPtr->getInternal()->blendAdditive(nativeClip, weight, fadeLength, layer);
	}

	void ScriptAnimation::internal_Blend1D(ScriptAnimation* thisPtr, MonoObject* info, float t)
	{
		Blend1DInfo nativeInfo = ScriptBlend1DInfo::fromManaged(info);
		thisPtr->getInternal()->blend1D(nativeInfo, t);
	}

	void ScriptAnimation::internal_Blend2D(ScriptAnimation* thisPtr, MonoObject* info, Vector2* t)
	{
		Blend2DInfo nativeInfo = ScriptBlend2DInfo::fromManaged(info);
		thisPtr->getInternal()->blend2D(nativeInfo, *t);
	}

	void ScriptAnimation::internal_CrossFade(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, float fadeLength)
	{
		HAnimationClip nativeClip;
		if (clip != nullptr)
			nativeClip = clip->getHandle();

		thisPtr->getInternal()->crossFade(nativeClip, fadeLength);
	}

	void ScriptAnimation::internal_Stop(ScriptAnimation* thisPtr, UINT32 layer)
	{
		thisPtr->getInternal()->stop(layer);
	}

	void ScriptAnimation::internal_StopAll(ScriptAnimation* thisPtr)
	{
		thisPtr->getInternal()->stopAll();
	}

	bool ScriptAnimation::internal_IsPlaying(ScriptAnimation* thisPtr)
	{
		return thisPtr->getInternal()->isPlaying();
	}

	bool ScriptAnimation::internal_GetState(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, AnimationClipState* state)
	{
		HAnimationClip nativeClip;
		if (clip != nullptr)
			nativeClip = clip->getHandle();

		return thisPtr->getInternal()->getState(nativeClip, *state);
	}

	void ScriptAnimation::internal_SetState(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, AnimationClipState* state)
	{
		HAnimationClip nativeClip;
		if (clip != nullptr)
			nativeClip = clip->getHandle();

		thisPtr->getInternal()->setState(nativeClip, *state);
	}

	MonoField* ScriptBlendClipInfo::clipField = nullptr;
	MonoField* ScriptBlendClipInfo::positionField = nullptr;

	ScriptBlendClipInfo::ScriptBlendClipInfo(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptBlendClipInfo::initRuntimeData()
	{
		clipField = metaData.scriptClass->getField("clip");
		positionField = metaData.scriptClass->getField("position");
	}

	BlendClipInfo ScriptBlendClipInfo::fromManaged(MonoObject* instance)
	{
		BlendClipInfo output;

		MonoObject* managedAnimClip = clipField->getValueBoxed(instance);
		if(managedAnimClip)
		{
			ScriptAnimationClip* clip = ScriptAnimationClip::toNative(managedAnimClip);
			output.clip = clip->getHandle();
		}
		
		positionField->getValue(instance, &output.position);

		return output;
	}

	MonoField* ScriptBlend1DInfo::clipsField = nullptr;

	ScriptBlend1DInfo::ScriptBlend1DInfo(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptBlend1DInfo::initRuntimeData()
	{
		clipsField = metaData.scriptClass->getField("clips");
	}

	Blend1DInfo ScriptBlend1DInfo::fromManaged(MonoObject* instance)
	{
		MonoArray* managedClipsArray = (MonoArray*)clipsField->getValueBoxed(instance);
		if (managedClipsArray == nullptr)
			return Blend1DInfo(0);

		ScriptArray clipsArray(managedClipsArray);

		UINT32 numClips = clipsArray.size();
		Blend1DInfo output(numClips);

		for (UINT32 i = 0; i < numClips; i++)
			output.clips[i] = ScriptBlendClipInfo::fromManaged(clipsArray.get<MonoObject*>(i));

		return output;
	}

	MonoField* ScriptBlend2DInfo::topLeftClipField = nullptr;
	MonoField* ScriptBlend2DInfo::topRightClipField = nullptr;
	MonoField* ScriptBlend2DInfo::botLeftClipField = nullptr;
	MonoField* ScriptBlend2DInfo::botRightClipField = nullptr;

	ScriptBlend2DInfo::ScriptBlend2DInfo(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptBlend2DInfo::initRuntimeData()
	{
		topLeftClipField = metaData.scriptClass->getField("topLeftClip");
		topRightClipField = metaData.scriptClass->getField("topRightClip");
		botLeftClipField = metaData.scriptClass->getField("botLeftClip");
		botRightClipField = metaData.scriptClass->getField("botRightClip");
	}

	Blend2DInfo ScriptBlend2DInfo::fromManaged(MonoObject* instance)
	{
		Blend2DInfo output;

		MonoObject* managedTopLeftClip = topLeftClipField->getValueBoxed(instance);
		if (managedTopLeftClip != nullptr)
		{
			ScriptAnimationClip* clip = ScriptAnimationClip::toNative(managedTopLeftClip);
			output.topLeftClip = clip->getHandle();
		}

		MonoObject* managedTopRightClip = topRightClipField->getValueBoxed(instance);
		if (managedTopRightClip != nullptr)
		{
			ScriptAnimationClip* clip = ScriptAnimationClip::toNative(managedTopRightClip);
			output.topRightClip = clip->getHandle();
		}

		MonoObject* managedBotLeftClip = botLeftClipField->getValueBoxed(instance);
		if (managedBotLeftClip != nullptr)
		{
			ScriptAnimationClip* clip = ScriptAnimationClip::toNative(managedBotLeftClip);
			output.botLeftClip = clip->getHandle();
		}

		MonoObject* managedBotRightClip = botRightClipField->getValueBoxed(instance);
		if (managedBotRightClip != nullptr)
		{
			ScriptAnimationClip* clip = ScriptAnimationClip::toNative(managedBotRightClip);
			output.botRightClip = clip->getHandle();
		}

		return output;
	}
}