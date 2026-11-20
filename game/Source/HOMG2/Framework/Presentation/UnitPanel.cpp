#include "Framework/Presentation/UnitPanel.h"
#include "Framework/Presentation/UnitPresenter.h"
void UUnitPanel::BindPresenter(UUnitPresenter* Source)
{
    if (Presenter.IsValid()) Presenter->OnChanged().RemoveAll(this);
    Presenter = Source;
    if (Source) Source->OnChanged().AddUObject(this, &ThisClass::Refresh);
    Refresh();
}

void UUnitPanel::Refresh() { RenderUnit(Presenter.IsValid() ? Presenter->GetSelected() : FUnitSnapshot{}); }

void UUnitPanel::NativeDestruct()
{
    if (Presenter.IsValid()) Presenter->OnChanged().RemoveAll(this);
    Presenter.Reset();
    Super::NativeDestruct();
}
