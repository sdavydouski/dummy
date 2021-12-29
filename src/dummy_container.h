// Closed hash table with linear (for now) probing
template <typename T>
inline b32
IsSlotEmpty(T *Value)
{
    b32 Result = StringEquals(Value->Name, "");
    return Result;
}

template <typename T>
internal T *
HashTableLookup(u32 Count, T *Values, char *Name)
{
    u64 HashValue = Hash(Name);
    u32 HashSlot = HashValue % Count;

    T *Result = Values + HashSlot;

    // Linear probing
    // todo: eventually do round-robin or quadratic
    u32 ProbIndex = (HashSlot + 1) % Count;
    u32 IterationCount = 0;

    // todo: better conditions
    while (!(IsSlotEmpty(Result) || StringEquals(Result->Name, Name)))
    {
        Result = Values + ProbIndex++;

        if (ProbIndex >= Count)
        {
            ProbIndex = 0;
        }

        if (IterationCount++ >= Count)
        {
            Assert(!"HashTable is full!");
        }
    }

    return Result;
}
//

// Doubly linked list with sentinel node
template <typename T>
inline void
AddToLinkedList(T *Sentinel, T *Item)
{
    // Adding to the end of the list
    Item->Prev = Sentinel->Prev;
    Item->Next = Sentinel;

    Item->Prev->Next = Item;
    Item->Next->Prev = Item;
}

template <typename T>
inline void
RemoveFromLinkedList(T *Item)
{
    Item->Prev->Next = Item->Next;
    Item->Next->Prev = Item->Prev;
}
//
