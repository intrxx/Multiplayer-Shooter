// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BOverheadWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class BLASTER_API UBOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NetRoleText;
	
	void SetNetRoleText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

protected:
	virtual void NativeDestruct() override;
};
