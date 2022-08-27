#pragma once

// Closed hash table with quadratic probing
template <typename T>
struct hash_table
{
    u32 Count;
    T *Values;
};

template <typename T>
inline b32
IsSlotEmpty(T *Value)
{
    b32 Result = StringEquals(Value->Name, "");
    return Result;
}

template <typename T>
internal T *
HashTableLookup(hash_table<T> *HashTable, char *Name)
{
    u64 HashValue = Hash(Name);
    u32 HashSlot = HashValue % HashTable->Count;

    T *Result = HashTable->Values + HashSlot;

    u32 IterationCount = 0;

    while (!(IsSlotEmpty(Result) || StringEquals(Result->Name, Name)))
    {
        // todo: round-robin?
        u32 ProbIndex = (HashSlot + Square(IterationCount + 1)) % HashTable->Count;
        Result = HashTable->Values + ProbIndex;

        if (IterationCount++ >= HashTable->Count)
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
