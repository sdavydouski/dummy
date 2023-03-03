#pragma once

#define EMPTY_SLOT_KEY_STRING ""
#define EMPTY_SLOT_KEY_U32 0

struct memory_arena;

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

template <typename T>
inline void
InitHashTable(hash_table<T> *HashTable, u32 Count, memory_arena *Arena)
{
    Assert(IsPrime(Count));

    HashTable->Count = Count;
    HashTable->Values = PushArray(Arena, Count, T);
}

template <typename TValue, typename TKey>
dummy_internal TValue *
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

// Stack
template <typename T>
struct stack
{
    u32 MaxCount;
    u32 Head;
    T *Values;
};

template <typename T>
inline void
InitStack(stack<T> *Stack, u32 MaxCount, memory_arena *Arena)
{
    Stack->MaxCount = MaxCount;
    Stack->Head = 0;
    Stack->Values = PushArray(Arena, MaxCount, T);
}

template <typename T>
inline void
Push(stack<T> *Stack, T NewValue)
{
    T *Value = Stack->Values + Stack->Head++;

    Assert(Stack->Head < Stack->MaxCount);

    *Value = NewValue;
}

template <typename T>
inline T
Top(stack<T> *Stack)
{
    Assert(Stack->Head > 0);

    T Value = Stack->Values[Stack->Head - 1];
    return Value;
}

template <typename T>
inline T
Pop(stack<T> *Stack)
{
    T Value = Top(Stack);
    --Stack->Head;

    Assert(Stack->Head >= 0);

    return Value;
}

template <typename T>
inline u32
Size(stack<T> *Stack)
{
    u32 Result = Stack->Head;
    return Result;
}

template <typename T>
inline void
Clear(stack<T> *Stack)
{
    Stack->Head = 0;
}
//