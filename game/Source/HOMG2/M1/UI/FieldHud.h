#pragma once

#include "GameFramework/HUD.h"
#include "FieldHud.generated.h"

class SWidget;
class UGameViewportClient;
class ULocalPlayer;

UCLASS()
class HOMG2_API AFieldHud : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	TSharedPtr<SWidget> Layout;
	TWeakObjectPtr<UGameViewportClient> ViewportClient;
	TWeakObjectPtr<ULocalPlayer> LocalPlayer;
};
