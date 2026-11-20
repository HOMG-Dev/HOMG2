#include "Framework/Presentation/UnitMarker.h"
#include "Framework/Presentation/UnitPresenter.h"
UUnitMarker::UUnitMarker() { SetVisibility(false); }
void UUnitMarker::BindUnit(UUnitPresenter* Source, FGuid Id)
{
    if (Presenter.IsValid()) Presenter->OnChanged().RemoveAll(this);
    Presenter = Source;
    Unit = Id;
    if (Source) Source->OnChanged().AddUObject(this, &ThisClass::Refresh);
    Refresh();
}

void UUnitMarker::Refresh()
{
    const auto Snapshot = Presenter.IsValid() ? Presenter->GetUnit(Unit) : FUnitSnapshot{};
    SetVisibility(Snapshot.Id.IsValid());
    RenderUnit(Snapshot);
}

void UUnitMarker::RenderUnit_Implementation(const FUnitSnapshot& Snapshot)
{
    SetText(Snapshot.Id.IsValid() ? UEnum::GetDisplayValueAsText(Snapshot.Condition) : FText::GetEmpty());
}

void UUnitMarker::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Presenter.IsValid()) Presenter->OnChanged().RemoveAll(this);
    Presenter.Reset();
    Super::EndPlay(Reason);
}
