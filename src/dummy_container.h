#pragma once

// Closed hash table with quadratic probing
template <typename T>
struct hash_table
{
    u32 Count;
    T *Values;
};

inline b32
IsSlotEmpty(char *Key)
{
    b32 Result = StringEquals(Key, "");
    return Result;
}

inline b32
IsSlotEmpty(u32 Key)
{
    b32 Result = (Key == 0);
    return Result;
}

template <typename T>
internal T *
HashTableLookup(hash_table<T> *HashTable, char *Key)
{
    u64 HashValue = Hash(Key);
    u32 HashSlot = HashValue % HashTable->Count;

    T *Result = HashTable->Values + HashSlot;

    u32 IterationCount = 0;

    while (!(IsSlotEmpty(Result->Key) || StringEquals(Result->Key, Key)))
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

template <typename T>
internal T *
HashTableLookup(hash_table<T> *HashTable, u32 Key)
{
    u64 HashValue = Hash(Key);
    u32 HashSlot = HashValue % HashTable->Count;

    T *Result = HashTable->Values + HashSlot;

    u32 IterationCount = 0;

    while (!(IsSlotEmpty(Result->Key) || Result->Key == Key))
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
