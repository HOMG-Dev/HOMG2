#include "M1/FieldMatchSession.h"

void UFieldMatchSession::InitializeFixture()
{
	if (State.SessionId.IsValid()) return;
	State = FFieldMatchSnapshot{};
	State.SessionId = FGuid::NewGuid();
	FFieldBattalionSnapshot Battalion;
	Battalion.Id = FGuid::NewGuid();
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FFieldMemberSnapshot Member;
		Member.Id = FGuid::NewGuid();
		Battalion.Members.Add(Member);
	}
	State.Battalions.Add(Battalion);
	FFieldEquipmentSnapshot Equipment;
	Equipment.Id = FGuid::NewGuid();
	Equipment.Kind = TEXT("TestEquipment");
	State.Inventory.Add(Equipment);
}

bool UFieldMatchSession::SetGold(int32 Gold)
{
	if (Gold < 0 || Gold == State.Gold) return false;
	State.Gold = Gold;
	Changed.Broadcast();
	return true;
}

bool UFieldMatchSession::RemoveEquipment(const FGuid& EquipmentId)
{
	const int32 Index = State.Inventory.IndexOfByPredicate([&](const FFieldEquipmentSnapshot& Item)
	{
		return Item.Id == EquipmentId;
	});
	if (Index == INDEX_NONE) return false;
	State.Inventory.RemoveAt(Index);
	Changed.Broadcast();
	return true;
}
