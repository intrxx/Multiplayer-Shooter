// Copyright 2026 out of sCope team - intrxx


#include "Input/BInputConfig.h"

const UInputAction* UBInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FBlasterInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	
	return nullptr;
}
