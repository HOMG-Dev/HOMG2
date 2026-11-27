#pragma once

#include "GameFramework/HUD.h"
#include "FieldHud.generated.h"

class SWidget;
class UGameViewportClient;
class ULocalPlayer;
class UFieldMatchSession;

UCLASS()
class HOMG2_API AFieldHud : public AHUD
{
	GENERATED_BODY()

public:
	AFieldHud();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleMatchChanged();
	void RemoveLayout();

	TSharedPtr<SWidget> Layout;
	TWeakObjectPtr<UGameViewportClient> ViewportClient;
	TWeakObjectPtr<ULocalPlayer> LocalPlayer;
	TWeakObjectPtr<UFieldMatchSession> MatchSession;
	FDelegateHandle MatchChangedHandle;
};
