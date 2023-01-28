#pragma once

#define EMPTY_SLOT_KEY_STRING ""
#define EMPTY_SLOT_KEY_U32 0

// Closed hash table with quadratic probing
template <typename T>
struct hash_table
{
    u32 Count;
    T *Values;
};

inline bool32
IsSlotEmpty(char *Key)
{
    bool32 Result = StringEquals(Key, EMPTY_SLOT_KEY_STRING);
    return Result;
}

inline bool32
IsSlotEmpty(u32 Key)
{
    bool32 Result = (Key == EMPTY_SLOT_KEY_U32);
    return Result;
}

inline bool32
IsEquals(char *Key1, char *Key2)
{
    bool32 Result = StringEquals(Key1, Key2);
    return Result;
}

inline bool32
IsEquals(u32 Key1, u32 Key2)
{
    bool32 Result = Key1 == Key2;
    return Result;
}

inline void
RemoveFromHashTable(char *Key)
{
    CopyString(EMPTY_SLOT_KEY_STRING, Key);
}

inline void
RemoveFromHashTable(u32 *Key)
{
    *Key = EMPTY_SLOT_KEY_U32;
}

template <typename TValue, typename TKey>
internal TValue *
HashTableLookup(hash_table<TValue> *HashTable, TKey Key)
{
    u64 HashValue = Hash(Key);
    u32 HashSlot = HashValue % HashTable->Count;

    TValue *Result = HashTable->Values + HashSlot;

    u32 IterationCount = 0;

    while (!(IsSlotEmpty(Result->Key) || IsEquals(Result->Key, Key)))
    {
        // todo: round-robin?
        u32 ProbIndex = (HashSlot + Square(IterationCount + 1)) % HashTable->Count;
        Result = HashTable->Values + ProbIndex;

        if (IterationCount++ >= HashTable->Count)
        {
            Assert(!"HashTable is full!");
        }

        if (IterationCount >= 3)
        {
            Assert(!"Better hash function?");
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
