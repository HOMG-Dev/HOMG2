#include "Framework/Presentation/UnitPresenter.h"
#include "Framework/Network/UnitFeed.h"

void UUnitPresenter::Attach(UUnitFeed* InFeed)
{
    if (Feed.IsValid()) Feed->OnUpdated().RemoveAll(this);
    Feed = InFeed;
    Selected.Invalidate();
    if (InFeed) InFeed->OnUpdated().AddUObject(this, &ThisClass::Refresh);
    Refresh();
}

FUnitSnapshot UUnitPresenter::GetUnit(FGuid Id) const
{
    if (Feed.IsValid())
        for (const auto& S : Feed->GetVisible()) if (S.Id == Id) return S;
    return {};
}

FUnitSnapshot UUnitPresenter::GetSelected() const { return GetUnit(Selected); }

void UUnitPresenter::SelectUnit(FGuid Id)
{
    const FGuid VisibleId = GetUnit(Id).Id;
    if (VisibleId != Selected) { Selected = VisibleId; Changed.Broadcast(); }
}

void UUnitPresenter::Refresh()
{
    if (!GetSelected().Id.IsValid()) Selected.Invalidate();
    Changed.Broadcast();
}
