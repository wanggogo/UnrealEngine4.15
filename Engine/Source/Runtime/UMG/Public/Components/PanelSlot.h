// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/Visual.h"
#include "PanelSlot.generated.h"

/** The base class for all Slots in UMG. */
UCLASS(BlueprintType)
class UMG_API UPanelSlot : public UVisual
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(Instanced)
	class UPanelWidget* Parent;

	UPROPERTY(Instanced)
	class UWidget* Content;
	
	bool IsDesignTime() const;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/** Applies all properties to the live slot if possible. */
	virtual void SynchronizeProperties()
	{
	}

#if WITH_EDITOR

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		SynchronizeProperties();
	}

#endif
};
